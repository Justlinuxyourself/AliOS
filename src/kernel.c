/* src/kernel.c */
#include "section1_cpu/io.h"
#include "section1_cpu/heap.h"
#include "section4_shell/shell.h"
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;  // In 32-bit/64-bit GCC, 'int' is 32 bits
typedef unsigned long long uint64_t;
typedef struct {
    unsigned short* buffer;
    int cursor_pos;
    char command_buffer[80];
    int buffer_idx;
} tty_t;

extern char kbd_get_char(unsigned char scancode);
extern void timer_init();
extern void timer_wait_tick();
extern void vga_init_ttys();
extern void vga_draw_status_bar(); 
extern void vga_clear();
extern void vga_write(const char* data);
extern void vga_putchar(char c);
extern char* itoa(int value, char* str);
extern unsigned char get_failed_attempts();
extern void set_failed_attempts(unsigned char count);

extern tty_t ttys[];        
extern int current_tty;     

int strcmp_custom(char* s1, char* s2) {
    int i = 0;
    while (s1[i] != '\0' || s2[i] != '\0') {
        if (s1[i] != s2[i]) return 0;
        i++;
    }
    return 1;
}

/* * permanent_lockout_siren: 
 * A siren that never stops. Call this when strikes >= 3.
 */
void permanent_lockout_siren() {
    while(1) {
        // High Tone (1200 Hz)
        play_sound(1200);
        sleep(1);

        // Low Tone (800 Hz)
        play_sound(800);
        sleep(1);
    }
}
void emergency_siren_slide() {
    while(1) {
        // Slide Up
        for (int freq = 400; freq < 1200; freq += 10) {
            play_sound(freq);
            sleep(1); // Very short sleep for a smooth slide
        }
        // Slide Down
        for (int freq = 1200; freq > 400; freq -= 10) {
            play_sound(freq);
            sleep(1);
        }
    }
}
void lock_system_hardened() {
    char* secret = "Ali123";
    char input[32];
    int idx = 0;
    int strikes = get_failed_attempts(); 
    int clock_ticks = 0;

    vga_clear();

    while (1) {
        if (strikes >= 3) {
    vga_clear();
    vga_write("!!! SECURITY BREACH DETECTED !!!\n");
    vga_write("System has been permanently locked.\n");
    vga_write("Contact your administrator.");

    // This function never returns, so the CPU stays here forever.
    permanent_lockout_siren(); 
}
        vga_write("          Enter Password: ");

        while (1) {
            // SYNC: Keep the clock moving while user types password
            timer_wait_tick();
            clock_ticks++;
            if (clock_ticks >= 100) {
                vga_draw_status_bar();
                clock_ticks = 0;
            }

            if (inb(0x64) & 0x01) {
                unsigned char scancode = inb(0x60);
                char c = kbd_get_char(scancode);
                if (c == 0) continue; 

                if (c == '\n') {
                    input[idx] = '\0';
                    if (strcmp_custom(input, secret)) {
                        set_failed_attempts(0); 
                        vga_clear();
                        return; 
                    } else {
                        strikes++;
                        set_failed_attempts(strikes); 
                        vga_write("\n          [ ACCESS DENIED ] - Strike ");
                        char s_buf[4];
                        vga_write(itoa(strikes, s_buf));
                        vga_write("/3\n");
                        idx = 0;
                        break; 
                    }
                } 
                else if (c == '\b') {
                    if (idx > 0) { idx--; vga_putchar('\b'); }
                } 
                else if (c >= ' ' && idx < 31) {
                    input[idx++] = c;
                    vga_putchar('*'); 
                }
            }
        }
    }
}

void kernel_main() {
    vga_clear();
    shell_init(); 
    timer_init();
    vga_init_ttys();

    while (inb(0x64) & 0x01) { inb(0x60); }

    lock_system_hardened();

    vga_write("AliOS 4 - madde by a 12yo - Multi-TTY Mode\n");
    vga_write("System Ready. Use Ctrl+Alt+F1-F10 to switch.\n");
    vga_write("> ");

    int clock_ticks = 0;

    while(1) {
        // FIXED: This ensures 1 loop = 1 hardware tick (10ms)
        timer_wait_tick(); 

        clock_ticks++;
        if (clock_ticks >= 100) { 
            vga_draw_status_bar();
            clock_ticks = 0;
        }

        if (inb(0x64) & 0x01) {
            unsigned char scancode = inb(0x60);
            tty_t* active = &ttys[current_tty];

            if (scancode == 0x0F) {
                shell_tab_complete(active->command_buffer, &active->buffer_idx);
            } 
            else {
                char c = kbd_get_char(scancode);
                if (c == 0) continue; 

                if (c == '\n') {
                    active->command_buffer[active->buffer_idx] = '\0';
                    shell_dispatch(active->command_buffer);
                    active->buffer_idx = 0;
                    vga_draw_status_bar();
                } 
                else if (c == '\b') {
                    if (active->buffer_idx > 0) {
                        active->buffer_idx--;
                        vga_putchar('\b');
                    }
                } 
                else if (c >= ' ' && active->buffer_idx < 79) {
                    active->command_buffer[active->buffer_idx++] = c;
                    vga_putchar(c);
                }
            }
        }
    }
}
