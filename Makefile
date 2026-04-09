# --- AliOS 4 Master Build System ---

# 1. Project Structure
SRCDIR = src
OBJDIR = obj
BIN = alios4.bin
ISO = alios4.iso

# 2. Automatically find ALL .c and .asm files in all subfolders
C_SOURCES = $(shell find $(SRCDIR) -name '*.c')
ASM_SOURCES = $(shell find $(SRCDIR) -name '*.asm')

# 3. Transform source paths into object paths in the 'obj' folder
OBJ = $(C_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
OBJ += $(ASM_SOURCES:$(SRCDIR)/%.asm=$(OBJDIR)/%.o)

# 4. Flags
CFLAGS = -m64 -c -ffreestanding -fno-stack-protector -Iinclude -Wall -Wextra
LDFLAGS = -n -T linker.ld

# --- Build Rules ---

all: $(ISO)

# Link everything together
$(BIN): $(OBJ)
	@echo "Linking AliOS 4..."
	ld $(LDFLAGS) -o $(BIN) $(OBJ)
	@echo -n "Made_by_justlinuxyourself" >> $(BIN)

# Rule for C files (Handles subdirectories automatically)
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	gcc $(CFLAGS) $< -o $@

# Rule for Assembly files
$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	@mkdir -p $(@D)
	nasm -f elf64 $< -o $@

# ISO Generation
$(ISO): $(BIN)
	@mkdir -p isodir/boot/grub
	cp $(BIN) isodir/boot/
	cp grub.cfg isodir/boot/grub/
	grub-mkrescue -o $(ISO) isodir

clean:
	rm -rf $(OBJDIR) $(BIN) $(ISO) isodir

.PHONY: all clean
