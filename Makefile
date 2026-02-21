# AliOS 4 Makefile
# LINKED TO NOTEBOOK: Build System Configuration

# 1. Add speaker.o to the OBJ list
OBJ = boot.o kernel.o vga.o kbd.o shell.o heap.o cmos.o timer.o speaker.o

# Output binary and ISO names
BIN = alios4.bin
ISO = alios4.iso

all: $(ISO)

# Link the kernel
$(BIN): $(OBJ)
	ld -n -o $(BIN) -T linker.ld $(OBJ)

# Compile Assembly
boot.o: src/section1_cpu/boot.asm
	nasm -f elf64 src/section1_cpu/boot.asm -o boot.o

# --- C Compilation Rules ---

# 2. Add the specific rule for speaker.o
speaker.o: src/section1_cpu/speaker.c
	gcc -m64 -c src/section1_cpu/speaker.c -o speaker.o -ffreestanding -fno-stack-protector

timer.o: src/section1_cpu/timer.c
	gcc -m64 -c src/section1_cpu/timer.c -o timer.o -ffreestanding -fno-stack-protector

kernel.o: src/kernel.c
	gcc -m64 -c src/kernel.c -o kernel.o -ffreestanding -fno-stack-protector

vga.o: src/section2_video/vga.c
	gcc -m64 -c src/section2_video/vga.c -o vga.o -ffreestanding -fno-stack-protector

kbd.o: src/section3_io/kbd.c
	gcc -m64 -c src/section3_io/kbd.c -o kbd.o -ffreestanding -fno-stack-protector

shell.o: src/section4_shell/shell.c
	gcc -m64 -c src/section4_shell/shell.c -o shell.o -ffreestanding -fno-stack-protector

heap.o: src/section1_cpu/heap.c
	gcc -m64 -c src/section1_cpu/heap.c -o heap.o -ffreestanding -fno-stack-protector

cmos.o: src/section3_io/cmos.c
	gcc -m64 -c src/section3_io/cmos.c -o cmos.o -ffreestanding -fno-stack-protector

# Create ISO
$(ISO): $(BIN)
	mkdir -p isodir/boot/grub
	cp $(BIN) isodir/boot/
	cp grub.cfg isodir/boot/grub/
	grub-mkrescue -o $(ISO) isodir

clean:
	rm -rf *.o $(BIN) $(ISO) isodir