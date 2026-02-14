#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h>

extern uint8_t key_states[];

// Kernel functions used by snake
void terminal_initialize(void);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
void delay(uint32_t ms);

// I/O Port Helpers
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ( "outl %0, %w1" : : "a"(val), "Nd"(port) );
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ( "inl %w1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

int strcmp(const char* s1, const char* s2);
size_t strlen(const char* str);

void* memset(void* bufptr, int value, size_t size);

// Terminal Exports (so isr.c can see them)
extern void terminal_putchar(char c);

// Global States
extern int caps_lock;
extern int shift_pressed;
extern uint8_t terminal_color;

// Function Exports
void handle_shell_input(char c);
void terminal_write(const char* data);
void update_leds(); // New!

#endif
