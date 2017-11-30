OBJECTS=main.o
HEADERS=
LIBS=
CFLAGS=-std=c99

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

riscv-sim: $(OBJECTS)
	$(CC) -o riscv-sim $(OBJECTS) $(CFLAGS)

.PHONY: clean

clean:
	rm *.o
	rm riscv-sim
