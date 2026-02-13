#include <stdint.h>
extern void idt_flush(uint32_t);
// 1. Fixed the struct name to be consistent
struct idt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry_struct {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_hi;
} __attribute__((packed));

// 1. Define the template first
struct idt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_entry_struct idt_entries[256];
struct idt_ptr_struct   idt_ptr; // Now this matches the struct above!

// Function to set an individual gate
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = (base & 0xFFFF);
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags;
}

// External assembly links
extern void idt_flush(uint32_t);
extern void irq1(); // ASM Keyboard Stub
extern void irq8(); // ASM RTC Stub
extern void isr0(); // ASM Divide-by-zero Stub

void init_idt() {
    // Set the pointer limit and base
    idt_ptr.limit = (sizeof(struct idt_entry_struct) * 256) - 1;
    idt_ptr.base  = (uint32_t)&idt_entries;

    // 1. Zero out the IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // 2. Map handlers to ASM stubs
    idt_set_gate(0,  (uint32_t)isr0, 0x08, 0x8E); // Exception 0
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E); // Keyboard (IRQ1)
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E); // R