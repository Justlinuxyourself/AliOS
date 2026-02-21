/* src/section1_cpu/io.h */
#ifndef IO_H
#define IO_H
#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40
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

/* Add this below your outb function */
static inline void cpuid(int code, unsigned int* a, unsigned int* b, unsigned int* c, unsigned int* d) {
    __asm__ volatile ("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(code));
}
#endif
