src = $(wildcard src/*.c) $(wildcard src/nondos/*.c) $(wildcard src/3dgfx/*.c)
obj = $(src:.c=.o)
dep = $(src:.c=.d)
bin = game

inc = -Isrc -Isrc/3dgfx -Isrc/kern

CFLAGS = -pedantic -Wall -g $(inc) -MMD
LDFLAGS = -lGL -lX11 -lXext -lm

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
