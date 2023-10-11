CFLAGS = -pedantic -Wall -g

mctab: mctab.o
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f *.o
