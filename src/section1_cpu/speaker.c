/* src/section1_cpu/speaker.c */
#include "io.h"



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
    // 1. Start the hardware
    play_sound(freq);

    // 2. THE FEATURE: If duration is 0, we exit and leave the sound ON
    // Useful for the Lockout!
    if (duration_ms == 0) return;

    // 3. THE MATH: If 1 second (1000ms) was duration * 1,000,000,
    // then 1ms is duration * 1,000.
    for(volatile unsigned long long i = 0; i < (unsigned long long)duration_ms * 1000; i++) {
        __asm__ volatile("pause"); 
    }

    // 4. Turn it off
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
