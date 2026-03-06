# AliOS 4 Makefile
# LINKED TO NOTEBOOK: Build System Configuration

# --- Configuration ---
OBJ = boot.o kernel.o vga.o kbd.o shell.o heap.o cmos.o timer.o speaker.o
BIN = alios4.bin
ISO = alios4.iso

# Compiler Flags (DRY - Don't Repeat Yourself)
CFLAGS = -m64 -c -ffreestanding -fno-stack-protector -Iinclude

# --- Primary Targets ---

all: $(ISO)

# Link the kernel and then embed the signature
$(BIN): $(OBJ)
	ld -n -o $(BIN) -T linker.ld $(OBJ)
	# Embedding signature safely at the end of the binary
	echo -n "Made_by_justlinuxyourself" >> $(BIN)

# --- Assembly Rules ---

boot.o: src/section1_cpu/boot.asm
	nasm -f elf64 src/section1_cpu/boot.asm -o boot.o

# --- C Compilation Rules ---

speaker.o: src/section1_cpu/speaker.c
	gcc $(CFLAGS) src/section1_cpu/speaker.c -o speaker.o

timer.o: src/section1_cpu/timer.c
	gcc $(CFLAGS) src/section1_cpu/timer.c -o timer.o

kernel.o: src/kernel.c
	gcc $(CFLAGS) src/kernel.c -o kernel.o

vga.o: src/section2_video/vga.c
	gcc $(CFLAGS) src/section2_video/vga.c -o vga.o

kbd.o: src/section3_io/kbd.c
	gcc $(CFLAGS) src/section3_io/kbd.c -o kbd.o

shell.o: src/section4_shell/shell.c
	gcc $(CFLAGS) src/section4_shell/shell.c -o shell.o

heap.o: src/section1_cpu/heap.c
	gcc $(CFLAGS) src/section1_cpu/heap.c -o heap.o

cmos.o: src/section3_io/cmos.c
	gcc $(CFLAGS) src/section3_io/cmos.c -o cmos.o

# --- ISO Creation ---

$(ISO): $(BIN)
	mkdir -p isodir/boot/grub
	cp $(BIN) isodir/boot/
	cp grub.cfg isodir/boot/grub/
	grub-mkrescue -o $(ISO) isodir

clean:
	rm -rf *.o $(BIN) $(ISO) isodir

.PHONY: all clean