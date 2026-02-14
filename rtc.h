#ifndef RTC_H
#define RTC_H

#include "common.h"

// CMOS Ports
#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

// RTC Functions
void init_rtc();
uint8_t get_rtc_register(int reg);
void get_time(int *h, int *m, int *s);
void print_time_tui();

#endif