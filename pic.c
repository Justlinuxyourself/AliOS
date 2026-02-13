#include <stdint.h>


static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

void pic_remap() {
    // ICW1: Start initialization in cascade mode
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW2: Vector offsets (Map IRQ 0-7 to 32-39, and IRQ 8-15 to 40-47)
    outb(0x21, 0x20); 
    outb(0xA1, 0x28);

    // ICW3: Tell Master PIC there is a slave PIC at IRQ2
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4: Set 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Mask all interrupts except the keyboard (IRQ 1) for now
    // 0xFD = 11111101 (Only bit 1 is 0/enabled)
    outb(0x21, 0xFD); 
    outb(0xA1, 0xFF);
}
