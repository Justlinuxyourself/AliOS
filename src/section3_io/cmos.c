/* src/section3_io/cmos.c */
/* LINKED TO NOTEBOOK: SECTION IV - CMOS & Real-Time Clock */

#include "../section1_cpu/io.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

unsigned char read_cmos(unsigned char reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

// CMOS values are often in BCD (Binary Coded Decimal)
// This converts them to standard integers
static unsigned char bcd_to_bin(unsigned char bcd) {
    return (bcd & 0x0F) + ((bcd / 16) * 10);
}

int cmos_get_hour()   { return bcd_to_bin(read_cmos(0x04)); }
int cmos_get_min()    { return bcd_to_bin(read_cmos(0x02)); }
int cmos_get_sec()    { return bcd_to_bin(read_cmos(0x00)); }
int cmos_get_day()    { return bcd_to_bin(read_cmos(0x07)); }
int cmos_get_month()  { return bcd_to_bin(read_cmos(0x08)); }

unsigned long long get_total_ram_bytes() {
    unsigned char low, high;
    low = read_cmos(0x17);
    high = read_cmos(0x18);
    unsigned long long extended_kb = (high << 8) | low;
    return (1024 + extended_kb) * 1024;
}
/* Add these to your cmos.c */

void write_cmos(unsigned char reg, unsigned char val) {
    outb(0x70, reg);
    outb(0x71, val);
}

// Store failed attempts in CMOS register 0x34
void set_failed_attempts(unsigned char count) {
    write_cmos(0x34, count);
}

unsigned char get_failed_attempts() {
    return read_cmos(0x34);
}