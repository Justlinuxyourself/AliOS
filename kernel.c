#include <stdint.h>
#include <stddef.h>
#include "common.h"
#include "hardware.h"

int brand;

extern char global_cpu_brand[49];

// Tracks how many 'ticks' have happened since boot
volatile uint32_t timer_ticks = 0;

int waiting_for_color = 0;

// Add these to the TOP of kernel.c or common.h
void nosound();
void execute_command(char* input);
void detect_cpu_brand();
void put_char_at(char c, uint8_t color, int x, int y); // Fixes undefined reference!
void play_sound(uint32_t nFrequence);
 
// Track all 128 possible scancodes (0 = up, 1 = down)
uint8_t key_states[128] = {0};

int waiting_for_name = 0;

// 0heh. input/cmd buffer
char command_buffer[256];  // Changed from input_buffer
size_t command_len = 0;    // Changed from input_index
// 1. VGA Settings
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

// 2. Terminal State (Merged into one place)
uint16_t* terminal_buffer = (uint16_t*) 0xB8000; // Fixed: now points to VGA memory
size_t terminal_row = 0;
size_t terminal_column = 0;
uint8_t terminal_color = 0x07; // Light grey on black

// 3. Prototypes (So terminal_write can see putchar)
void terminal_putchar(char c);
void update_cursor(int x, int y);
// Add this near the top of kernel.c
void terminal_initialize(void);

 void put_char_at(char c, uint8_t color, int x, int y) {
    const size_t index = y * 80 + x;
    terminal_buffer[index] = (uint16_t) c | (uint16_t) color << 8;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } 
    else if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            put_char_at(' ', terminal_color, terminal_column, terminal_row);
        }
    } 
    else {
        put_char_at(c, terminal_color, terminal_column, terminal_row);
        terminal_column++;

        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            terminal_row++;
        }
    }

    // SCROLLING LOGIC START
    if (terminal_row >= VGA_HEIGHT) {
        // 1. Move rows 1 through 24 up to rows 0 through 23
        for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
            }
        }

        // 2. Clear the very last row (bottom line)
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
        }

        // 3. Set the current row back to the bottom line
        terminal_row = VGA_HEIGHT - 1;
    }
    // SCROLLING LOGIC END

    update_cursor(terminal_column, terminal_row);
}

void init_rtc() {
    uint8_t status;
    
    // Select Register B, and disable NMI
    outb(0x70, 0x8B);      
    // Read current value
    status = inb(0x71);    
    // Set bit 6 to enable Periodic Interrupt (PIE)
    outb(0x70, 0x8B);      
    outb(0x71, status | 0x40); 
}

void rtc_handler() {
    timer_ticks++; // Or timer_ticks += 512 depending on frequency
    
    // IMPORTANT: You MUST read Register C or the RTC won't fire again
    outb(0x70, 0x0C);
    inb(0x71);
    
    // Send EOI to both PICs (RTC is on the Slave PIC)
    outb(0xA0, 0x20); 
    outb(0x20, 0x20);
}

void play_note(uint32_t freq, int duration) {
    play_sound(freq);
    delay(duration);
    nosound();
    delay(5); // Tiny pause so notes don't mush together
}

void play_sound(uint32_t nFrequence) {
    uint32_t Div;
    uint8_t tmp;

    // Set the PIT to the desired frequency
    Div = 1193180 / nFrequence;
    outb(0x43, 0xb6); // Command: Channel 2, LSB/MSB, Square Wave
    outb(0x42, (uint8_t) (Div) );
    outb(0x42, (uint8_t) (Div >> 8));

    // Read port 0x61 and set bits 0 and 1 to enable speaker
    tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void play_startup_sound() {
    // A rising melody: C4 -> E4 -> G4 -> C5
    play_note(261, 15); // C4
    play_note(329, 15); // E4
    play_note(392, 15); // G4
    play_note(523, 30); // C5 (higher and longer)
}

void draw_cake() {
    terminal_write("\n");
    terminal_color = 0x0E; // Yellow for flames
    terminal_write("      i  i  i  i\n");
    terminal_write("      |  |  |  |\n");
    
    terminal_color = 0x0D; // Light Magenta for frosting
    terminal_write("    |____________|\n");
    terminal_write("    |            |\n");
    
    terminal_color = 0x0B; // Light Cyan for cake body
    terminal_write("  __|____________|__\n");
    terminal_write(" |                  |\n");
    terminal_write(" |__________________|\n");
    
    terminal_color = 0x0F; // White text
    terminal_write("\n   HAPPY BIRTHDAY!!\n");
    terminal_color = 0x07; // Back to standard grey
}

void sing_birthday(char* name) {
    terminal_write("\n");
    terminal_color = 0x0D; // Pink/Magenta

    // Line 1: Happy Birthday to You
    terminal_write("Happy ");    play_note(261, 15); // C4 (short)
    terminal_write("Birthday "); play_note(261, 5);  // C4 (very short)
    terminal_write("to ");       play_note(294, 20); // D4
    terminal_write("you...\n");  play_note(261, 20); // C4
    play_note(349, 20); // F4
    play_note(329, 40); // E4

    // Line 2: Happy Birthday to You
    terminal_write("Happy ");    play_note(261, 15);
    terminal_write("Birthday "); play_note(261, 5);
    terminal_write("to ");       play_note(294, 20);
    terminal_write("you...\n");  play_note(261, 20);
    play_note(392, 20); // G4
    play_note(349, 40); // F4

    // Line 3: Happy Birthday dear [Name]
    terminal_write("Happy ");    play_note(261, 15);
    terminal_write("Birthday "); play_note(261, 5);
    terminal_write("dear ");     play_note(523, 20); // C5 (high!)
    terminal_color = 0x0B; // Cyan Name
    terminal_write(name);        play_note(440, 20); // A4
    terminal_write("!\n");       play_note(349, 20); // F4
    play_note(329, 20); // E4
    play_note(294, 20); // D4

    // Line 4: Happy Birthday to You!
    terminal_color = 0x0D;
    terminal_write("Happy ");    play_note(466, 15); // Bb4
    terminal_write("Birthday "); play_note(466, 5);
    terminal_write("to ");       play_note(440, 20); // A4
    terminal_write("you!!\n");   play_note(349, 20); // F4
    play_note(392, 20); // G4
    play_note(349, 60); // F4 (Finish)

    draw_cake();
}

void nosound() {
    uint8_t tmp = inb(0x61) & 0xFC; // Mask bits 0 and 1 to 0
    outb(0x61, tmp);
}

void handle_shell_input(char c) {
    if (c == '\n') {
        terminal_putchar('\n');
        command_buffer[command_len] = '\0';

        // --- STATE 1: Theme Selection ---
        if (waiting_for_color) {
            char choice = command_buffer[0];
            if (choice == '1') {
                terminal_color = 0x02; // Matrix
                terminal_write("Matrix mode active.");
            } else if (choice == '2') {
                terminal_color = 0x1B; // BlueBird
                terminal_write("BlueBird mode active.");
            } else if (choice == '3') {
                terminal_color = 0x07; // Classic
                terminal_write("Classic mode active.");
            } else {
                terminal_write("Invalid choice.");
            }
            waiting_for_color = 0;
        } 
        // --- STATE 2: Birthday Mode ---
        else if (waiting_for_name) {
            sing_birthday(command_buffer);
            waiting_for_name = 0; 
            // Removed the color reset (0x0A) that was here!
        } 
        // --- STATE 3: Normal Commands ---
        else {
            execute_command(command_buffer);
        }

        // --- THE ONLY PROMPT ---
        // This ensures the prompt always matches the new theme
        command_len = 0;
        memset(command_buffer, 0, 256);
        terminal_write("\nAliOS> "); 
    } 
    else if (c == '\b') {
        if (command_len > 0) {
            command_len--;
            terminal_putchar('\b');
        }
    } 
    else {
        if (command_len < 255) {
            command_buffer[command_len++] = c;
            terminal_putchar(c);
        }
    }
}

void execute_command(char* input) {
    if (strcmp(input, "help") == 0) {
        terminal_color = 0x0B; // Cyan
        terminal_write("Commands: help, cls, info, beep, reboot, halt, bday, time, date, neofetch, color, specs, uptime\n");
    } 
    else if (strcmp(input, "cls") == 0) {
        terminal_initialize();
    }
    else if (strcmp(input, "info") == 0) {
        terminal_color = 0x0E; // Yellow
        terminal_write("--- AliOS Hardware Report ---\n");
        detect_cpu();
        detect_cpu_brand();
        detect_ata();
        probe_pci();
    }
    else if (strcmp(input, "beep") == 0) {
        play_sound(1000);
        delay(50);
        nosound();
    }
    else if (strcmp(input, "halt") == 0) {
        terminal_color = 0x0C; // Red
        terminal_write("System Halting. Goodbye!\n");
        __asm__ volatile("cli"); 
        while(1) { __asm__ ("hlt"); } 
    }
    else if (strcmp(input, "reboot") == 0) {
        terminal_write("Rebooting...\n");
        // Pulse the CPU reset line via the keyboard controller
        uint8_t good = 0x02;
        while (good & 0x02) good = inb(0x64);
        outb(0x64, 0xFE); 
    } else if (strcmp(input, "bday") == 0) {
    terminal_write("Enter the birthday person's name: ");
    waiting_for_name = 1;
    return; // Stop here and wait for the next 'Enter'
    } else if (strcmp(input, "time") == 0) {
    // Tell kernel.c that get_time exists in another file (rtc.c)
    extern void get_time(int *h, int *m, int *s); 
    
    int h, m, s;
    get_time(&h, &m, &s);

    terminal_color = 0x0E; // Yellow
    terminal_write("Real-Time Clock: ");
    
    // Print HH:MM:SS
    terminal_putchar((h / 10) + '0');
    terminal_putchar((h % 10) + '0');
    terminal_putchar(':');
    terminal_putchar((m / 10) + '0');
    terminal_putchar((m % 10) + '0');
    terminal_putchar(':');
    terminal_putchar((s / 10) + '0');
    terminal_putchar((s % 10) + '0');
    terminal_write("\n");
    } else if (strcmp(input, "date") == 0) {
    extern void get_date(int *day, int *month, int *year);
    int d, m, y;
    get_date(&d, &m, &y);

    terminal_color = 0x0E; // Yellow
    terminal_write("Current Date: ");
    terminal_putchar((m / 10) + '0'); terminal_putchar((m % 10) + '0');
    terminal_putchar('/');
    terminal_putchar((d / 10) + '0'); terminal_putchar((d % 10) + '0');
    terminal_putchar('/');
    terminal_write("20"); // Assuming 21st century
    terminal_putchar((y / 10) + '0'); terminal_putchar((y % 10) + '0');
    terminal_write("\n");
    } else if (strcmp(input, "neofetch") == 0) {
    extern char global_cpu_vendor[13];
    extern char global_cpu_brand[49];
    
    uint8_t accent_color;
    // Get the background bits (upper 4 bits)
    uint8_t current_bg = (terminal_color & 0xF0); 

    // --- THEME-BASED COLOR LOGIC ---
    if (current_bg == 0x10) { 
        accent_color = 0x0F; // Theme 2 (BlueBird): White logo on Blue
    } else if (terminal_color == 0x02) {
        accent_color = 0x0A; // Theme 1 (Matrix): Light Green logo
    } else {
        accent_color = 0x0B; // Theme 3 (Classic): Light Cyan logo
    }

    // --- PRINTING WITH ADAPTIVE COLORS ---
    terminal_color = accent_color; terminal_write("      ___      "); 
    terminal_color = 0x0F; terminal_write("OS: "); 
    terminal_color = 0x07; terminal_write("AliOS x86\n");

    terminal_color = accent_color; terminal_write("     /  /\\     "); 
    terminal_color = 0x0F; terminal_write("Kernel: "); 
    terminal_color = 0x07; terminal_write("0.0.1-alpha\n");

    terminal_color = accent_color; terminal_write("    /  / /     "); 
    terminal_color = 0x0F; terminal_write("Vendor: "); 
    terminal_color = 0x07; terminal_write(global_cpu_vendor); terminal_write("\n");

    terminal_color = accent_color; terminal_write("   /__/ /      "); 
    terminal_color = 0x0F; terminal_write("CPU: "); 
    terminal_color = 0x07; 
    for(int i = 0; i < 25 && global_cpu_brand[i] != '\0'; i++) {
        terminal_putchar(global_cpu_brand[i]);
    }
    terminal_write("\n");

    terminal_color = accent_color; terminal_write("   \\  \\/       "); 
    terminal_color = 0x0F; terminal_write("Shell: "); 
    terminal_color = 0x07; terminal_write("AliShell\n");

    terminal_color = accent_color; terminal_write("    \\__\\       "); 
    terminal_color = 0x0F; terminal_write("Arch: "); 
    terminal_color = 0x07; terminal_write("i686\n\n");

    // Reset prompt back to the actual theme color
    if (current_bg == 0x10) terminal_color = 0x1B;
    else if (terminal_color == 0x0A || terminal_color == 0x02) terminal_color = 0x02;
    else terminal_color = 0x07;
} else if (strcmp(input, "color") == 0) {
    terminal_write("Pick a theme (1: Matrix, 2: BlueBird, 3: Classic): ");
    waiting_for_color = 1;
    return; // Stop here to wait for user input
    } else if (strcmp(input, "specs") == 0) {
    // 1. External Links to Hardware Data
    extern char global_cpu_vendor[13];
    extern char global_cpu_brand[49];
    
    // 2. Dynamic Memory Detection (CMOS)
    outb(0x70, 0x30); uint16_t low = inb(0x71);
    outb(0x70, 0x31); uint16_t high = inb(0x71);
    uint16_t total_kb = (low | (high << 8)) + 1024; // Physical KB detected

    // 3. Dynamic VGA Data
    // We use the variables VGA_WIDTH and VGA_HEIGHT directly
    uint32_t buffer_addr = (uint32_t)terminal_buffer;
    char hex_chars[] = "0123456789ABCDEF";

    // Header
    terminal_color = 0x0F; // Bright White
    terminal_write("\n--- AliOS DYNAMIC HARDWARE REPORT ---\n");

    // CPU Section
    terminal_color = 0x07; // Grey
    terminal_write("PROCESSOR:\n");
    terminal_write("  Vendor string: "); terminal_write(global_cpu_vendor); terminal_write("\n");
    terminal_write("  Brand name:    "); terminal_write(global_cpu_brand); terminal_write("\n");

    // RAM Section
    terminal_write("MEMORY:\n");
    terminal_write("  Total CMOS RAM: ");
    int mb = total_kb / 1024;
    if (mb >= 100) terminal_putchar((mb / 100) + '0');
    if (mb >= 10)  terminal_putchar(((mb / 10) % 10) + '0');
    terminal_putchar((mb % 10) + '0');
    terminal_write(" MB (");
    // Show raw KB for precision
    int kb_temp = total_kb;
    terminal_putchar((kb_temp / 1000) + '0');
    terminal_putchar(((kb_temp / 100) % 10) + '0');
    terminal_putchar(((kb_temp / 10) % 10) + '0');
    terminal_putchar((kb_temp % 10) + '0');
    terminal_write(" KB)\n");

    // VGA Section
    terminal_write("DISPLAY:\n");
    terminal_write("  Resolution:    ");
    // Print Width Variable
    int w = (int)VGA_WIDTH;
    terminal_putchar((w / 10) + '0'); terminal_putchar((w % 10) + '0');
    terminal_putchar('x');
    // Print Height Variable
    int h = (int)VGA_HEIGHT;
    terminal_putchar((h / 10) + '0'); terminal_putchar((h % 10) + '0');
    
    terminal_write("\n  VGA Buffer:    0x");
    // Dynamic Hex Address of the terminal_buffer
    for(int i = 7; i >= 0; i--) {
        terminal_putchar(hex_chars[(buffer_addr >> (i * 4)) & 0xF]);
    }

    // Color/Theme Section
    terminal_write("\n  Attr Byte:     0x");
    terminal_putchar(hex_chars[(terminal_color >> 4) & 0x0F]);
    terminal_putchar(hex_chars[terminal_color & 0x0F]);
    
    terminal_write("\n  Total Cells:   ");
    // Dynamic math: Width * Height
    int cells = (int)(VGA_WIDTH * VGA_HEIGHT);
    // Basic number printing for cells
    terminal_putchar((cells / 1000) + '0');
    terminal_putchar(((cells / 100) % 10) + '0');
    terminal_putchar(((cells / 10) % 10) + '0');
    terminal_putchar((cells % 10) + '0');
    terminal_write("\n");
} else if (strcmp(input, "uptime") == 0) {
    // Assuming 100 ticks = 1 second
    uint32_t total_seconds = timer_ticks / 1024;
    
    uint32_t hours = total_seconds / 3600;
    uint32_t minutes = (total_seconds % 3600) / 60;
    uint32_t seconds = total_seconds % 60;

    terminal_color = 0x0E; // Yellow
    terminal_write("System Uptime: ");
    
    // Print Hours (only if > 0 to keep it clean)
    if (hours > 0) {
        if (hours > 9) terminal_putchar((hours / 10) + '0');
        terminal_putchar((hours % 10) + '0');
        terminal_write("h ");
    }

    // Print Minutes
    if (minutes > 0 || hours > 0) {
        terminal_putchar((minutes / 10) + '0');
        terminal_putchar((minutes % 10) + '0');
        terminal_write("m ");
    }

    // Print Seconds (always show seconds)
    terminal_putchar((seconds / 10) + '0');
    terminal_putchar((seconds % 10) + '0');
    terminal_write("s\n");
    } else if (input[0] != '\0') {
        terminal_color = 0x0C; // Red
        terminal_write("Err: command not found\n");
    }

    
} 
void delay(uint32_t ms) {
    // 1024 ticks per 1000ms
    uint32_t target_ticks = timer_ticks + (ms * 1024 / 1000);
    while (timer_ticks < target_ticks) {
        __asm__ volatile("hlt"); // Wait for the next RTC interrupt
    }
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = (uint16_t) c | (uint16_t) color << 8;
}

// 4. Initialization Logic
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    // Remove terminal_color = 0x07; so it uses the CURRENT theme color!
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        terminal_buffer[i] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
    }
}

void draw_window(int x, int y, int width, int height, char* title) {
    uint8_t border_color = 0x1F; // White on Blue background

    // Draw Corners
    terminal_putentryat(201, border_color, x, y);               // Top Left
    terminal_putentryat(187, border_color, x + width, y);       // Top Right
    terminal_putentryat(200, border_color, x, y + height);       // Bottom Left
    terminal_putentryat(188, border_color, x + width, y + height); // Bottom Right

    // Draw Horizontal Lines
    for (int i = 1; i < width; i++) {
        terminal_putentryat(205, border_color, x + i, y);
        terminal_putentryat(205, border_color, x + i, y + height);
    }

    // Draw Vertical Lines
    for (int i = 1; i < height; i++) {
        terminal_putentryat(186, border_color, x, y + i);
        terminal_putentryat(186, border_color, x + width, y + i);
    }

    // Print Title in the middle of the top bar
    terminal_row = y;
    terminal_column = x + 2;
    terminal_color = 0x1E; // Yellow on Blue
    terminal_write(title);
}


void append_to_buffer(char c) {
    if (command_len < 255) {
        command_buffer[command_len++] = c;
        command_buffer[command_len] = '\0';
    }
}

void clear_buffer() {
    // Use memset to be safe, or just reset the index
    command_len = 0;
    command_buffer[0] = '\0';
}


// 5. Cursor Logic
void update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void init_timer(uint32_t frequency) {
    // The PIT has an internal clock of 1.193182 MHz
    uint32_t divisor = 1193180 / frequency;

    // Send the command byte (0x36): Binary, Square Wave, LSB then MSB, Channel 0
    outb(0x43, 0x36);

    // Split the divisor into two bytes and send to the PIT
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

    outb(0x40, l);
    outb(0x40, h);
}

// Inside your Timer Interrupt Service Routine (ISR)
void timer_handler() {
    timer_ticks++;
    // Important: Tell the PIC we've handled the interrupt
    outb(0x20, 0x20); 
}

// 7. Write Function (Exported for isr.c)
void terminal_write(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

// 8. The Entry Point (Matches 'call kernel_main' in boot.s)
extern void init_gdt();
extern void init_idt();
extern void pic_remap();

  
void kernel_main(void) {
    // 1. Basic CPU setup
    init_gdt();
    init_idt();
    pic_remap();
    
    // 2. Clear screen and show boot message
    terminal_initialize();
    terminal_write("AliOS x86 Kernel Loading...\n");

    // 3. Hardware detection
    detect_cpu();
    detect_cpu_brand();
    detect_ata();
    probe_pci();

    // 4. Initialize RTC (The new heartbeat)
    init_rtc();
    terminal_write("RTC Heartbeat Initialized.\n");

    // 5. Play startup music (Interrupts are still CLI here)
    terminal_write("Playing startup melody...\n");
    play_startup_sound();
    nosound(); // Ensure speaker is dead before unmasking

    /* 6. The Master/Slave PIC Masking Logic:
       Port 0x21 (Master): 0xFD = 11111101 
          - Unmasks IRQ 1 (Keyboard) 
          - Unmasks IRQ 2 (The bridge to Slave PIC - REQUIRED for RTC)
       Port 0xA1 (Slave): 0xFE = 11111110
          - Unmasks IRQ 8 (RTC)
    */
    outb(0x21, 0xFD & ~(1 << 2)); // Unmask Keyboard AND Bridge (IRQ 2)
    outb(0xA1, 0xFE);             // Unmask RTC

    // 7. Open the floodgates
    __asm__ volatile("sti");
    
    terminal_write("\nInitialization Complete.\n");
    terminal_write("AliOS> ");

    // 8. Yield the CPU until an interrupt happens
    while(1) {
        __asm__ ("hlt");
    }
}
