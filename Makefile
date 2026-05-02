# --- AliOS 4 Master Build System (VERBOSE VERSION) ---
SRCDIR = src
OBJDIR = obj
BIN = alios4.bin
ISO = alios4.iso

C_SOURCES = $(shell find $(SRCDIR) -name '*.c')
ASM_SOURCES = $(shell find $(SRCDIR) -name '*.asm' ! -name 'mbr.asm')

OBJ = $(C_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
OBJ += $(ASM_SOURCES:$(SRCDIR)/%.asm=$(OBJDIR)/%.o)

CFLAGS = -m64 -c -ffreestanding -fno-stack-protector -Iinclude -Wall -Wextra
# ADDED -Map=link.map to see the section layout
LDFLAGS = -n -T linker.ld --build-id=none -z max-page-size=0x1000 --no-warn-rwx-segments -Map=link.map

all: $(ISO)

mbr_payload: $(SRCDIR)/section1_cpu/mbr.asm
	mkdir -p include
	nasm -f bin $< -o mbr.bin
	xxd -i mbr.bin > include/mbr_bin.h
	rm mbr.bin

$(BIN): mbr_payload $(OBJ)
	@echo "--- LINKING STEP ---"
	ld $(LDFLAGS) -o $(BIN) $(OBJDIR)/section1_cpu/boot.o $(filter-out $(OBJDIR)/section1_cpu/boot.o, $(OBJ))

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(@D)
	gcc $(CFLAGS) $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	mkdir -p $(@D)
	nasm -f elf64 $< -o $@

$(ISO): $(BIN)
	@mkdir -p isodir/boot/grub
	cp $(BIN) isodir/boot/
	cp grub.cfg isodir/boot/grub/
	grub-mkrescue -o $(ISO) isodir

clean:
	rm -rf $(OBJDIR) $(BIN) $(ISO) isodir include/mbr_bin.h

.PHONY: all clean mbr_payload
