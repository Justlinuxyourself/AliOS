/* Copyright (c) 2026 Ali  
All rights reserved.
*/
#include "io.h"

extern unsigned long long timer_ticks;
extern void timer_wait_tick(); // From my timer.c

// Core Hardware Control
void play_sound(unsigned int nFrequence) {
    if (nFrequence == 0) {
        nosound();
        return;
    }
    // PIT frequency is 1.193182 MHz
    unsigned int Div = 1193182 / nFrequence;
    
    outb(0x43, 0xb6);
    outb(0x42, (unsigned char) (Div & 0xFF));
    outb(0x42, (unsigned char) ((Div >> 8) & 0xFF));

    unsigned char tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void nosound() {
    outb(0x61, inb(0x61) & 0xFC);
}


void beep_ex(int duration_ms, int freq) {
    if (freq <= 0) return;
    
    play_sound(freq);
    
    // 200Hz logic: ticks = (ms * 200) / 1000 -> ms / 5
    unsigned long long ticks_to_wait = duration_ms / 5;
    if (ticks_to_wait == 0) ticks_to_wait = 1;
    
    unsigned long long target = timer_ticks + ticks_to_wait;
    
    while (timer_ticks < target) {
        timer_wait_tick(); 
    }
    
    nosound();
}

// Quick shell beep
void beep() {
    beep_ex(50, 1000); 
}

// Professional AliOS Startup
void startup_melody() {
    // Note: Frequency, Duration
    beep_ex(150, 440); // A
    beep_ex(150, 554); // C#
    beep_ex(150, 659); // E
    beep_ex(300, 880); // High A
}
