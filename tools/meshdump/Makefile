obj = meshdump.o cmesh.o meshload.o dynarr.o rbtree.o
bin = meshdump

LDFLAGS = -lm

$(bin): $(obj)
	$(CC) -o $@ $(obj) $(LDFLAGS)

clean:
	rm -f $(bin) $(obj)
