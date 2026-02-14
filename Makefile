# Use your cross-compiler prefix
CC = i686-elf-gcc
AS = i686-elf-as

# Compiler flags
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -ffreestanding -O2 -nostdlib -lgcc

# For Intel High Definition Audio (HDA) - Best for x86_64
QEMU_FLAGS += -device intel-hda -device hda-duplex,audiodev=snd0
QEMU_FLAGS += -audiodev pa,id=snd0  # Use 'pa' for PulseAudio, 'alsa' for ALSA, or 'dsound' for Windows
# The final kernel name
TARGET = myos.bin

# List of object files to build
OBJS = boot.o kernel.o idt.o gdt.o isr.o pic.o string.o cpu.o ata.o pci.o rtc.o

all: $(TARGET)

# Link the kernel
$(TARGET): $(OBJS)
	$(CC) -T linker.ld -o $(TARGET) $(LDFLAGS) $(OBJS)

# Assemble boot.s
boot.o: boot.s
	$(AS) boot.s -o boot.o

# Compile kernel.c
kernel.o: kernel.c
	$(CC) -c kernel.c -o kernel.o $(CFLAGS)

# Clean up build files
clean:
	rm -f *.o $(TARGET)

# Run in QEMU
run: $(TARGET)
	qemu-system-i386 $(QEMU_FLAGS) -kernel $(TARGET)

# This maps the PC speaker to your host's audio system
run-pcspk:
	qemu-system-x86_64 -machine pcspk-audiodev=snd0 -audiodev pa,id=snd0 -kernel $(TARGET)
