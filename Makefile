src = $(wildcard src/*.c) $(wildcard src/nondos/*.c) $(wildcard src/3dgfx/*.c)
ssrc = src/data.asm
obj = $(src:.c=.o) $(ssrc:.asm=.o)
dep = $(src:.c=.d)
bin = game

inc = -Isrc -Isrc/3dgfx -Isrc/kern

CFLAGS = -pedantic -Wall -O2 -ffast-math -fno-strict-aliasing -g $(inc) -MMD
LDFLAGS = -lGL -lX11 -lXext -lm

PNGDUMP = tools/pngdump/pngdump


$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

src/data.o: src/data.asm data/tex.img
	nasm -f elf64 -o $@ $<

tools/pngdump/pngdump:
	$(MAKE) -C tools/pngdump

data/tex.img: data/tex.png $(PNGDUMP)
	$(PNGDUMP) -o $@ -oc $(subst .img,.pal,$@) -os $(subst .img,.slut,$@) -s 8 $<

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
