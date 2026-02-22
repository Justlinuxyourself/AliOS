/* src/section1_cpu/speaker.c */
#include "io.h"
extern unsigned long long timer_ticks;

// Keep this as your core hardware logic
void play_sound(unsigned int nFrequence) {
    if (nFrequence == 0) return;
    unsigned int Div = 1193180 / nFrequence;
    
    outb(0x43, 0xb6);
    outb(0x42, (unsigned char) (Div) );
    outb(0x42, (unsigned char) (Div >> 8));

    unsigned char tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void nosound() {
    outb(0x61, inb(0x61) & 0xFC);
}

void beep_ex(int duration_ms, int freq) {
    play_sound(freq);

    if (duration_ms > 0) {
        // Calculate target based on 100Hz (10ms per tick)
        // If it still feels too fast, use (duration_ms / 5) to compensate for your 200Hz bug
        unsigned long long target = timer_ticks + (duration_ms / 10); 
        
        while (timer_ticks < target) {
            timer_wait_tick(); // Force a poll to update ticks
        }
    }
    nosound();
}


// Shell command beep
void beep() {
    beep_ex(40, 1000); 
}
/* src/section1_cpu/speaker.c */

void startup_melody() {
    // Note 1: A (440Hz)
    beep_ex(500, 440); 
    
    // Tiny gap between notes
    for(volatile int i = 0; i < 500000; i++); 

    // Note 2: E (659Hz)
    beep_ex(500, 659);
    
    for(volatile int i = 0; i < 500000; i++);

    // Note 3: High A (880Hz)
    beep_ex(500, 880);
}
