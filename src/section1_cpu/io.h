/* 
Copyright (c) 2026 Ali  
All rights reserved.
*/
#ifndef IO_H
#define IO_H
#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40
#include <stdint.h>
// Read a byte from a port
static inline unsigned char inb(unsigned short port) {
    unsigned char val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

// Write a byte to a port
static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void cpuid(int code, unsigned int* a, unsigned int* b, unsigned int* c, unsigned int* d) {
    __asm__ volatile ("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(code));
}
static inline uint16_t inw(uint16_t port) {
	uint16_t result;
	__asm__ volatile ("inw %1, %0" : "=a"(result) : "Nd"(port));
	return result;
}
static inline void outw(uint16_t port, uint16_t data) {
	__asm__ volatile ("outw %w0, %w1" : : "a"(data), "Nd"(port));
}
#endif
