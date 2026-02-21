/* src/section1_cpu/timer.c */
#include "io.h"

unsigned long long timer_ticks = 0;
void timer_init() {
    // 100Hz: 1193182 / 100 = 11931
    unsigned int divisor = 11931; 

    // 0x36 = Square wave mode, LSB then MSB
    outb(0x43, 0x36);
    outb(0x40, (unsigned char)(divisor & 0xFF));        // LSB
    outb(0x40, (unsigned char)((divisor >> 8) & 0xFF)); // MSB
}

void timer_wait_tick() {
    static unsigned short last_val = 0xFFFF;
    unsigned short current_val;

    while (1) {
        // 1. Latch the PIT (Freezes the count so we can read it safely)
        outb(0x43, 0x00);
        
        // 2. Read LSB then MSB
        unsigned char low = inb(0x40);
        unsigned char high = inb(0x40);
        current_val = (high << 8) | low;

        // 3. Since PIT counts DOWN (from 11931 to 0):
        // If current is HIGHER than last, it definitely wrapped around (Tick!)
        if (current_val > last_val) {
            last_val = current_val;
            timer_ticks++;
            return; // Success! One tick recorded.
        }

        last_val = current_val;
        
        // 4. Give the hardware a tiny breather
        __asm__ volatile("pause");
    }
}

unsigned int get_uptime_seconds() {
    // CRITICAL: Must divide by 100 because frequency is 100Hz
    return (unsigned int)(timer_ticks / 200); 
}

void sleep(int seconds) {
    unsigned long long target = timer_ticks + (seconds * 200);
    while (timer_ticks < target) {
        timer_wait_tick();
    }
}
