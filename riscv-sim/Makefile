### CONFIGURATION ###
RISCV_PATH=/opt/riscv64-unknown-elf/bin
RISCV_CC=$(RISCV_PATH)/riscv64-unknown-elf-gcc
RISCV_AS=$(RISCV_PATH)/riscv64-unknown-elf-as
RISCV_LD=$(RISCV_PATH)/riscv64-unknown-elf-ld
RISCV_OBJCOPY=$(RISCV_PATH)/riscv64-unknown-elf-objcopy

### END CONFIGURATION ###

PROGRAMS=riscv-sim
OBJECTS=main.o uart.o mem.o
HEADERS=uart.h mem.h
LIBS=
CFLAGS=-std=c99 -Wall
RISCV_PROGRAMS=test/riscv-test.elf test/riscv-test.bin
RISCV_OBJECTS=test/head.o test/copy_data.o test/main.o test/uart.o
RISCV_CFLAGS=-march=rv32i -mabi=ilp32 -ffreestanding -nostdlib
RISCV_AFLAGS=-march=rv32i -mabi=ilp32

.PHONY: all

all: $(PROGRAMS) $(RISCV_PROGRAMS)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

test/%.o: test/%.c
	$(RISCV_CC) -c -o $@ $< $(RISCV_CFLAGS)

test/%.o: test/%.S
	$(RISCV_AS) -o $@ $< $(RISCV_AFLAGS)

riscv-sim: $(OBJECTS)
	$(CC) -o riscv-sim $(OBJECTS) $(CFLAGS)

test/riscv-test.elf: $(RISCV_OBJECTS) test/riscv-test.ld
	$(RISCV_LD) -T test/riscv-test.ld -o $@ $(RISCV_OBJECTS)

#test/riscv-test.bin: $(RISCV_OBJECTS) test/riscv-test.ld
#	$(RISCV_LD) -T test/riscv-test.ld --oformat=binary -o $@ $(RISCV_OBJECTS)

test/riscv-test.bin: test/riscv-test.elf
	$(RISCV_OBJCOPY) -O binary $< $@

.PHONY += clean

clean:
	rm -f $(OBJECTS)
	rm -f $(RISCV_OBJECTS)
	rm -f $(PROGRAMS)
	rm -f $(RISCV_PROGRAMS)
