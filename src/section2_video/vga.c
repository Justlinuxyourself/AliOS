/* src/section2_video/vga.c */
/* LINKED TO NOTEBOOK: SECTION II - Enhanced Video & Independent TTYs */

#include "../section1_cpu/heap.h"

#define VGA_ADDRESS 0xB8000
#define NOTEBOOK_YELLOW 0x1E
#define WIDTH 80
#define HEIGHT 25
#define MAX_TTYS 10
#define VIDEO_SIZE (WIDTH * HEIGHT * 2)
/* Ensure these declarations are at the top of vga.c */
extern int cmos_get_hour();
extern int cmos_get_min();
extern int cmos_get_sec();
/* src/section2_video/vga.c */
int timezone_offset_seconds = 0; // The "Master" variable
// Independent TTY structure
typedef struct {
    unsigned short* buffer;
    int cursor_pos;
    char command_buffer[80];
    int buffer_idx;
} tty_t;

tty_t ttys[MAX_TTYS];
int current_tty = 0;

void vga_write_num_at(int pos, int num) {
    unsigned short* vga_hardware = (unsigned short*)VGA_ADDRESS;
    vga_hardware[pos] = (unsigned short)((num / 10) + '0') | (unsigned short)0x1F << 8;
    vga_hardware[pos + 1] = (unsigned short)((num % 10) + '0') | (unsigned short)0x1F << 8;
}

void vga_set_cursor(int x, int y) {
    // Ensure we don't go out of bounds or hit the status bar (Row 24)
    if (x >= WIDTH) x = WIDTH - 1;
    if (y >= HEIGHT - 1) y = HEIGHT - 2;

    tty_t* active = &ttys[current_tty];
    
    // Calculate the 1D array index from 2D coordinates
    // index = (Row * 80) + Column
    active->cursor_pos = (y * WIDTH) + x;

    // Move the actual blinking hardware cursor to this spot
    update_hardware_cursor(active->cursor_pos);
}

void vga_draw_status_bar() {
    unsigned short* vga_hardware = (unsigned short*)VGA_ADDRESS;
    int base_pos = 24 * 80; // The 25th row (index 1920)

    // 1. Get raw units from CMOS
    int s = cmos_get_sec();
    int m = cmos_get_min();
    int h = cmos_get_hour();

    // 2. Convert to total seconds of the day and apply the shell offset
    // (Hours * 3600) + (Minutes * 60) + Seconds
    long total_seconds = (h * 3600) + (m * 60) + s + timezone_offset_seconds;

    // 3. Handle Day Rollover (Underflow and Overflow)
    // 86400 seconds = 24 hours
    while (total_seconds < 0) total_seconds += 86400;
    while (total_seconds >= 86400) total_seconds -= 86400;

    // 4. Extract final H:M:S from the adjusted total
    int final_h = total_seconds / 3600;
    int final_m = (total_seconds % 3600) / 60;
    int final_s = total_seconds % 60;

    // 5. Determine AM/PM and convert to 12-hour format
    const char* am_pm = (final_h >= 12) ? "PM" : "AM";
    int hour12 = final_h % 12;
    if (hour12 == 0) hour12 = 12; 

    // 6. Draw Date (MM/DD)
    vga_write_num_at(base_pos, cmos_get_month());
    vga_hardware[base_pos + 2] = (unsigned short)'/' | (unsigned short)0x1F << 8;
    vga_write_num_at(base_pos + 3, cmos_get_day());

    // 7. Draw Time (HH:MM:SS AM/PM)
    vga_hardware[base_pos + 6] = (unsigned short)'|' | (unsigned short)0x1F << 8;
    vga_write_num_at(base_pos + 8, hour12);
    vga_hardware[base_pos + 10] = (unsigned short)':' | (unsigned short)0x1F << 8;
    vga_write_num_at(base_pos + 11, final_m);
    vga_hardware[base_pos + 13] = (unsigned short)':' | (unsigned short)0x1F << 8;
    vga_write_num_at(base_pos + 14, final_s);
    
    // Draw AM/PM text
    vga_hardware[base_pos + 17] = (unsigned short)am_pm[0] | (unsigned short)0x1F << 8;
    vga_hardware[base_pos + 18] = (unsigned short)am_pm[1] | (unsigned short)0x1F << 8;

    // 8. Draw TTY ID (Far Right)
    int tty_pos = base_pos + 70;
    vga_hardware[tty_pos] = (unsigned short)'T' | (unsigned short)0x1F << 8;
    vga_hardware[tty_pos+1] = (unsigned short)'T' | (unsigned short)0x1F << 8;
    vga_hardware[tty_pos+2] = (unsigned short)'Y' | (unsigned short)0x1F << 8;
    vga_hardware[tty_pos+3] = (unsigned short)':' | (unsigned short)0x1F << 8;
    vga_hardware[tty_pos+5] = (unsigned short)(current_tty + '0') | (unsigned short)0x1E << 8;
}


/* Update the physical hardware cursor */
void update_hardware_cursor(int pos) {
    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)0x0F), "Nd"((unsigned short)0x3D4));
    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)(pos & 0xFF)), "Nd"((unsigned short)0x3D5));
    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)0x0E), "Nd"((unsigned short)0x3D4));
    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)((pos >> 8) & 0xFF)), "Nd"((unsigned short)0x3D5));
}

/* Initialize all TTY buffers in the heap */
void vga_init_ttys() {
    for(int i = 0; i < MAX_TTYS; i++) {
        ttys[i].buffer = (unsigned short*)kmalloc(VIDEO_SIZE);
        ttys[i].cursor_pos = 0;
        ttys[i].buffer_idx = 0;
        
        // Clear each virtual screen with AliOS Yellow/Blue
        for (int j = 0; j < WIDTH * HEIGHT; j++) {
            ttys[i].buffer[j] = (unsigned short)' ' | (unsigned short)NOTEBOOK_YELLOW << 8;
        }
    }
}

/* Swap the active TTY struct and refresh hardware memory */
void switch_tty(int new_tty) {
    if (new_tty == current_tty) return;
    unsigned short* vga_hardware = (unsigned short*)VGA_ADDRESS;
    
    // 1. Save current hardware state to the current TTY's buffer
    for(int i = 0; i < WIDTH * HEIGHT; i++) {
        ttys[current_tty].buffer[i] = vga_hardware[i];
    }
    
    // 2. Switch active index
    current_tty = new_tty;
    
    // 3. Load the new TTY's buffer into hardware memory
    for(int i = 0; i < WIDTH * HEIGHT; i++) {
        vga_hardware[i] = ttys[current_tty].buffer[i];
    }
    vga_draw_status_bar();
    update_hardware_cursor(ttys[current_tty].cursor_pos);
}

void vga_clear() {
    unsigned short* vga_ptr = (unsigned short*)VGA_ADDRESS;
    // Clear both the hardware and the current active buffer
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        unsigned short blank = (unsigned short)' ' | (unsigned short)NOTEBOOK_YELLOW << 8;
        vga_ptr[i] = blank;
        ttys[current_tty].buffer[i] = blank;
    }
    ttys[current_tty].cursor_pos = 0;
    vga_draw_status_bar();
    update_hardware_cursor(0);
}

void vga_scroll() {
    tty_t* active = &ttys[current_tty];
    unsigned short* vga_hardware = (unsigned short*)VGA_ADDRESS;

    // Shift only Rows 0-22 up into Rows 1-23.
    // Loop limit: (HEIGHT - 2) * WIDTH = 1840.
    for (int i = 0; i < (HEIGHT - 2) * WIDTH; i++) {
        active->buffer[i] = active->buffer[i + WIDTH];
    }

    // Clear only Row 23 (the typing line).
    // Range: 1840 to 1919.
    for (int i = (HEIGHT - 2) * WIDTH; i < (HEIGHT - 1) * WIDTH; i++) {
        active->buffer[i] = (unsigned short)' ' | (unsigned short)NOTEBOOK_YELLOW << 8;
    }

    // Sync ONLY the workspace to hardware.
    for (int i = 0; i < WIDTH * (HEIGHT - 1); i++) {
        vga_hardware[i] = active->buffer[i];
    }

    // Reset cursor to the start of the newly cleared Row 23.
    active->cursor_pos = (HEIGHT - 2) * WIDTH;

    // Refresh the status bar on the protected last row.
    vga_draw_status_bar();
}

void vga_putchar(char c) {
    tty_t* active = &ttys[current_tty];
    unsigned short* vga_hardware = (unsigned short*)VGA_ADDRESS;

    // Boundary check: Row 24 starts at index 1920 (24 * 80)
    // If the next character would land on Row 24, we scroll first.
    if (active->cursor_pos >= WIDTH * (HEIGHT - 1)) {
        vga_scroll();
    }

    if (c == '\n') {
        active->cursor_pos += WIDTH - (active->cursor_pos % WIDTH);
    } else if (c == '\b') {
        if (active->cursor_pos > 0) {
            active->cursor_pos--;
            unsigned short blank = (unsigned short)' ' | (unsigned short)NOTEBOOK_YELLOW << 8;
            vga_hardware[active->cursor_pos] = blank;
            active->buffer[active->cursor_pos] = blank;
        }
    } else {
        unsigned short glyph = (unsigned short)c | (unsigned short)NOTEBOOK_YELLOW << 8;
        vga_hardware[active->cursor_pos] = glyph;
        active->buffer[active->cursor_pos] = glyph;
        active->cursor_pos++;
    }

    // Double check after writing to ensure we didn't just fill the last slot of Row 23
    if (active->cursor_pos >= WIDTH * (HEIGHT - 1)) {
        vga_scroll();
    }

    update_hardware_cursor(active->cursor_pos);
}

void vga_write(const char* data) {
    for (int i = 0; data[i] != '\0'; i++) {
        vga_putchar(data[i]);
    }
}
