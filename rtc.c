#include "rtc.h"
#include <stdint.h>

// Link to your VGA function
extern void put_char_at(char c, uint8_t color, int x, int y);

// Convert BCD to Binary
#define BCD_TO_BIN(bcd) ((((bcd) & 0xF0) >> 4) * 10 + ((bcd) & 0x0F))

// Helper to read CMOS registers
uint8_t read_rtc_register(int reg) {
    outb(0x70, reg);
    return BCD_TO_BIN(inb(0x71));
}

// THIS FIXES THE LINKER ERROR: kernel.o is looking for this name
void get_time(int *h, int *m, int *s) {
    *s = read_rtc_register(0x00);
    *m = read_rtc_register(0x02);
    *h = read_rtc_register(0x04);
}

// This keeps your live corner clock option
void update_clock_display() {
    int h, m, s;
    get_time(&h, &m, &s);

    uint8_t clock_color = 0x0F; // White text
    
    put_char_at((h / 10) + '0', clock_color, 72, 0);
    put_char_at((h % 10) + '0', clock_color, 73, 0);
    put_char_at(':', clock_color, 74, 0);
    put_char_at((m / 10) + '0', clock_color, 75, 0);
    put_char_at((m % 10) + '0', clock_color, 76, 0);
    put_char_at(':', clock_color, 77, 0);
    put_char_at((s / 10) + '0', clock_color, 78, 0);
    put_char_at((s % 10) + '0', clock_color, 79, 0);
}
// Add these to your get_date function
void get_date(int *day, int *month, int *year) {
    outb(0x70, 0x07); *day = BCD_TO_BIN(inb(0x71));
    outb(0x70, 0x08); *month = BCD_TO_BIN(inb(0x71));
    outb(0x70, 0x09); *year = BCD_TO_BIN(inb(0x71));
}