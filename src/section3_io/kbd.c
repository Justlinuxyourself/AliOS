/* src/section3_io/kbd.c */
/* LINKED TO NOTEBOOK: SECTION III - Keyboard & Scancodes with TTY Support */

// Helper to read from an I/O port
static inline unsigned char inb(unsigned short port) {
    unsigned char val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

// External function from vga.c to handle the screen swap
extern void switch_tty(int n);

// State trackers for modifier keys
static int ctrl_held = 0;
static int alt_held = 0;
static int shift_held = 0; // NEW: Tracks Shift state

// Standard US-QWERTY Map (Lowercase/Numbers)
char kbd_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

// Shifted US-QWERTY Map (Uppercase/Symbols)
char kbd_map_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};



/* Pass the scancode we already read in kernel_main */
char kbd_get_char(unsigned char scancode) {
    // 1. Detect Modifier Key Presses (Make codes)
    if (scancode == 0x1D) { ctrl_held = 1; return 0; }
    if (scancode == 0x38) { alt_held = 1; return 0; }
    // 0x2A = Left Shift, 0x36 = Right Shift
    if (scancode == 0x2A || scancode == 0x36) { shift_held = 1; return 0; }

    // 2. Detect Modifier Key Releases (Break codes = Make + 0x80)
    if (scancode == 0x9D) { ctrl_held = 0; return 0; }
    if (scancode == 0xB8) { alt_held = 0; return 0; }
    // 0xAA = L-Shift Release, 0xB6 = R-Shift Release
    if (scancode == 0xAA || scancode == 0xB6) { shift_held = 0; return 0; }

    // 3. Handle TTY Switching (Ctrl + Alt + F1-F10)
    if (ctrl_held && alt_held) {
        if (scancode >= 0x3B && scancode <= 0x44) {
            int target_tty = scancode - 0x3B;
            switch_tty(target_tty);
            return 0; 
        }
    }

    // 4. Standard mapping for printable characters
    // Only return characters on "Press" events (scancodes < 128)
    if (scancode < 128) {
        if (shift_held) {
            return kbd_map_shift[scancode]; // Return symbols/uppercase
        } else {
            return kbd_map[scancode];       // Return lowercase/numbers
        }
    }
    
    return 0;
}