csrc = $(wildcard src/*.c) $(wildcard src/kern/*.c) $(wildcard src/libc/*.c) \
	   $(wildcard src/3dgfx/*.c)
ssrc = $(wildcard src/*.asm) $(wildcard src/kern/*.asm) $(wildcard src/libc/*.asm)
obj = $(csrc:.c=.o) $(ssrc:.asm=.o)
dep = $(csrc:.c=.d)
bin = game.com

warn = -pedantic -Wall -Wno-unused-function
opt = -O2
inc = -Isrc -Isrc/3dgfx -Isrc/kern -Isrc/libc

AS = nasm

ASFLAGS = -Isrc/ -Isrc/kern/
CFLAGS = -m32 -march=i386 $(warn) $(opt) $(dbg) -fno-pic -ffreestanding \
		 -fno-stack-protector -mpreferred-stack-boundary=2 -nostdinc -ffast-math \
		 -fno-asynchronous-unwind-tables -fno-strict-aliasing $(inc) $(def) -MMD
LDFLAGS = -m elf_i386 -nostdlib -T com32.ld -Map game.map

$(bin): $(obj)
	$(LD) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.o: %.asm
	$(AS) -o $@ -f elf $(ASFLAGS) $<

%.s: %.c
	$(CC) $(CFLAGS) -masm=intel -S $< -o $@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

disasm: $(bin)
	ndisasm -o 0x100 -b 16 $< >$@
