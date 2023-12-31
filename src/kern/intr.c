/*
pcboot - bootable PC demo/game kernel
Copyright (C) 2018  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY, without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include "config.h"
#include "intr.h"
#include "desc.h"
#include "segm.h"
#include "asmops.h"
#include "panic.h"

#define SYSCALL_INT		0x80

/* IDT gate descriptor bits */
#define GATE_TASK		(5 << 8)
#define GATE_INTR		(6 << 8)
#define GATE_TRAP		(7 << 8)
#define GATE_DEFAULT	(1 << 11)
#define GATE_PRESENT	(1 << 15)

/* PIC command and data ports */
#define PIC1_CMD	0x20
#define PIC1_DATA	0x21
#define PIC2_CMD	0xa0
#define PIC2_DATA	0xa1

/* PIC initialization command word 1 bits */
#define ICW1_ICW4_NEEDED	(1 << 0)
#define ICW1_SINGLE			(1 << 1)
#define ICW1_INTERVAL4		(1 << 2)
#define ICW1_LEVEL			(1 << 3)
#define ICW1_INIT			(1 << 4)
/* PIC initialization command word 4 bits */
#define ICW4_8086			(1 << 0)
#define ICW4_AUTO_EOI		(1 << 1)
#define ICW4_BUF_SLAVE		(1 << 3) /* 1000 */
#define ICW4_BUF_MASTER		(3 << 2) /* 1100 */
#define ICW4_SPECIAL		(1 << 4)

/* PIC operation command word 2 bits */
#define OCW2_EOI	(1 << 5)


static void gate_desc(desc_t *desc, uint16_t sel, uint32_t addr, int dpl, int type);

/* defined in intr_asm.S */
void set_idt(uint32_t addr, uint16_t limit);
void intr_entry_default(void);
void intr_entry_fast_timer(void);
void irq7_entry_check_spurious(void);
void irq15_entry_check_spurious(void);

/* the IDT (interrupt descriptor table) */
static desc_t idt[256] __attribute__((aligned(8)));

/* table of handler functions for all interrupts */
static intr_func_t intr_func[256];

static struct intr_frame *cur_intr_frame;
static int eoi_pending;

#define INTR_ENTRY_EC(n, name)		\
	void intr_entry_##name(void);	\
	set_intr_entry(n, intr_entry_##name);
#define INTR_ENTRY_NOEC(n, name)	INTR_ENTRY_EC(n, name)

void init_intr(void)
{
	int i;

	set_idt((uint32_t)idt, sizeof idt - 1);

	/* initialize all entry points and interrupt handlers */
	for(i=0; i<256; i++) {
		set_intr_entry(i, intr_entry_default);
		interrupt(i, 0);
	}

	/* by including intrtab.h here the series of INTR_ENTRY_* macros will be
	 * expanded to a series of function prototypes for all interrupt entry
	 * points and the corresponding calls to set_intr_entry to set up the IDT
	 * slots
	 */
#include "intrtab.h"

	/* change IRQ0 to the fast timer interrupt entry */
	set_intr_entry(IRQ_TO_INTR(0), intr_entry_fast_timer);

	/* change irq7 and irq15 to special entry points which first
	 * make sure we didn't get a spurious interrupt before proceeding
	 */
	set_intr_entry(IRQ_TO_INTR(7), irq7_entry_check_spurious);
	set_intr_entry(IRQ_TO_INTR(15), irq15_entry_check_spurious);

	/* initialize the programmable interrupt controller
	 * setting up the maping of IRQs [0, 15] to interrupts [32, 47]
	 */
	prog_pic(IRQ_OFFSET);
	eoi_pending = 0;
}

void cleanup_intr(void)
{
	disable_intr();
	/* reprogram the PIC to the default BIOS offset (8) */
	prog_pic(8);
}

/* retrieve the current interrupt frame.
 * returns 0 when called during init.
 */
struct intr_frame *get_intr_frame(void)
{
	return cur_intr_frame;
}

/* set an interrupt handler function for a particular interrupt */
void interrupt(int intr_num, intr_func_t func)
{
	int iflag = get_intr_flag();
	disable_intr();
	intr_func[intr_num] = func;
	set_intr_flag(iflag);
}

/* this function is called from all interrupt entry points
 * it calls the appropriate interrupt handlers if available and handles
 * sending an end-of-interrupt command to the PICs when finished.
 */
void dispatch_intr(struct intr_frame frm)
{
	cur_intr_frame = &frm;

	if(IS_IRQ(frm.inum)) {
		eoi_pending = frm.inum;
	}

	if(intr_func[frm.inum]) {
		intr_func[frm.inum](frm.inum);
	} else {
		if(frm.inum < 32) {
			panic("unhandled exception %u, error code: %u, at cs:eip=%x:%x\n",
					frm.inum, frm.err, frm.cs, frm.eip);
		}
		/*printf("unhandled interrupt %d\n", frm.inum);*/
	}

	disable_intr();
	if(eoi_pending) {
		end_of_irq(INTR_TO_IRQ(eoi_pending));
	}
}

void prog_pic(int offs)
{
	/* send ICW1 saying we'll follow with ICW4 later on */
	outp(PIC1_CMD, ICW1_INIT | ICW1_ICW4_NEEDED);
	outp(PIC2_CMD, ICW1_INIT | ICW1_ICW4_NEEDED);
	/* send ICW2 with IRQ remapping */
	outp(PIC1_DATA, offs);
	outp(PIC2_DATA, offs + 8);
	/* send ICW3 to setup the master/slave relationship */
	/* ... set bit3 = 3rd interrupt input has a slave */
	outp(PIC1_DATA, 4);
	/* ... set slave ID to 2 */
	outp(PIC2_DATA, 2);
	/* send ICW4 to set 8086 mode (no calls generated) */
	outp(PIC1_DATA, ICW4_8086);
	outp(PIC2_DATA, ICW4_8086);
	/* done, just reset the data port to 0 */
	outp(PIC1_DATA, 0);
	outp(PIC2_DATA, 0);
}

static void gate_desc(desc_t *desc, uint16_t sel, uint32_t addr, int dpl, int type)
{
	/* first 16bit part is the low 16bits of the entry address */
	desc->d[0] = addr & 0xffff;
	/* second 16bit part is the segment selector for the entry code */
	desc->d[1] = sel;
	/* third 16bit part has the privilege level, type, and present bit */
	desc->d[2] = ((dpl & 3) << 13) | type | GATE_DEFAULT | GATE_PRESENT;
	/* last 16bit part is the high 16bits of the entry address */
	desc->d[3] = (addr & 0xffff0000) >> 16;
}

#define IS_TRAP(n)	((n) >= 32 && !IS_IRQ(n))
void set_intr_entry(int num, void (*handler)(void))
{
	int type = IS_TRAP(num) ? GATE_TRAP : GATE_INTR;

	/* the syscall interrupt has to have a dpl of 3 otherwise calling it from
	 * user space will raise a general protection exception. All the rest should
	 * have a dpl of 0 to disallow user programs to execute critical interrupt
	 * handlers and possibly crashing the system.
	 */
	int dpl = (num == SYSCALL_INT) ? 3 : 0;

	gate_desc(idt + num, selector(SEGM_CODE, 0), (uint32_t)handler, dpl, type);
}

void set_pic_mask(int pic, unsigned char mask)
{
	outp(pic > 0 ? PIC2_DATA : PIC1_DATA, mask);
}

unsigned char get_pic_mask(int pic)
{
	return inp(pic > 0 ? PIC2_DATA : PIC1_DATA);
}

void mask_irq(int irq)
{
	int port;
	unsigned char mask;

	if(irq < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		irq -= 8;
	}

	mask = inp(port) | (1 << irq);
	outp(port, mask);
}

void unmask_irq(int irq)
{
	int port;
	unsigned char mask;

	if(irq < 8) {
		port = PIC1_DATA;
	} else {
		port = PIC2_DATA;
		irq -= 8;
	}

	mask = inp(port) & ~(1 << irq);
	outp(port, mask);
}


void end_of_irq(int irq)
{
	int intr_state = get_intr_flag();
	disable_intr();

	if(!eoi_pending) {
		set_intr_flag(intr_state);
		return;
	}
	eoi_pending = 0;

	if(irq > 7) {
		outp(PIC2_CMD, OCW2_EOI);
	}
	outp(PIC1_CMD, OCW2_EOI);

	set_intr_flag(intr_state);
}

#ifdef ENABLE_GDB_STUB
void exceptionHandler(int id, void (*func)())
{
	set_intr_entry(id, func);
}
#endif
