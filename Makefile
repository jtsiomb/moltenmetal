src = $(wildcard src/*.c) $(wildcard src/nondos/*.c) $(wildcard src/3dgfx/*.c)
ssrc = src/data.asm
obj = $(src:.c=.o) $(ssrc:.asm=.o)
dep = $(src:.c=.d)
bin = game

inc = -Isrc -Isrc/3dgfx -Isrc/kern

CFLAGS = -pedantic -Wall -O3 -ffast-math -fno-strict-aliasing -g $(inc) -MMD
LDFLAGS = $(ldflags_sys)

PNGDUMP = tools/pngdump/pngdump
MESHDUMP = tools/meshdump/meshdump

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/)
ifeq ($(sys), mingw)
	obj = $(src:.c=.w32.o) $(ssrc:.asm=.w32.o)
	bin = game.exe

	ldflags_sys = -static-libgcc -lmingw32 -mwindows -lgdi32 -lwinmm -lopengl32
else
	ldflags_sys = -lGL -lX11 -lXext -lm
endif

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

%.w32.o: %.asm
	nasm -f coff -o $@ $<

%.w32.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

src/data.o: src/data.asm data/tex.img data/room.mesh
	nasm -f elf64 -o $@ $<

src/data.w32.o: src/data.asm data/tex.img data/room.mesh
	nasm -f coff -o $@ $<

tools/pngdump/pngdump:
	$(MAKE) -C tools/pngdump

tools/meshdump/meshdump:
	$(MAKE) -C tools/meshdump

%.img: %.png $(PNGDUMP)
	$(PNGDUMP) -o $@ -oc $(subst .img,.pal,$@) -os $(subst .img,.slut,$@) -s 8 $<

%.mesh: %.obj $(MESHDUMP)
	$(MESHDUMP) $< $@

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)

.PHONY: crosswin
crosswin:
	$(MAKE) CC=i686-w64-mingw32-gcc sys=mingw
