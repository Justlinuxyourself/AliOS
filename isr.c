#include <stdint.h>
#include "snake.h"
#include "common.h"

int shift_pressed = 0;
int caps_lock = 0;

extern void handle_shell_input(char c);

// The standard map
unsigned char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*', 0, ' '
};

// The shifted map
unsigned char kbd_us_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',   0,
    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*', 0, ' '
};


extern void terminal_write(const char* data);

// This is a simple error handler for now
void isr_handler() {
    // We'll use your terminal functions to print the error
    terminal_write("SYSTEM EXCEPTION: The kernel has halted to prevent damage.\n");
    while(1); 
}

// Tell the keyboard controller to update lights
void update_leds() {
    uint8_t status = (caps_lock << 2); // Bit 2 is Caps Lock LED
    outb(0x60, 0xED); // Tell keyboard "we want to set LEDs"
    // In a real OS, we'd wait for an ACK here, but for now:
    outb(0x60, status);
}

void keyboard_handler(struct regs *r) {
    uint8_t scancode = inb(0x60);

    // 1. Release Logic
    if (scancode & 0x80) {
        if (scancode == 0xAA || scancode == 0xB6) shift_pressed = 0;
        outb(0x20, 0x20);
        return;
    }

    // 2. Special Keys
    if (scancode == 0x2A || scancode == 0x36) { 
        shift_pressed = 1; 
        outb(0x20, 0x20); 
        return; 
    }
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        update_leds(); // Sync the physical light!
        outb(0x20, 0x20);
        return;
    }

    // 3. Character Conversion & XOR Logic
    char c = kbd_us[scancode];
    if (c != 0) {
        if (c >= 'a' && c <= 'z') {
            if (shift_pressed ^ caps_lock) c = kbd_us_shift[scancode];
        } else if (shift_pressed) {
            c = kbd_us_shift[scancode];
        }
        handle_shell_input(c);
    }

    outb(0x20, 0x20);
}
