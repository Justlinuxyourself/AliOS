#include "hardware.h"
#include "common.h"

char global_cpu_vendor[13]; // Global bucket for the Vendor ID
char global_cpu_brand[49];  // Global bucket for the Brand String

void detect_cpu() {
    uint32_t ebx, edx, ecx;
    // Leaf 0 gets the Vendor ID
    __asm__ volatile("cpuid" : "=b"(ebx), "=d"(edx), "=c"(ecx) : "a"(0));

    global_cpu_vendor[12] = '\0';
    *(uint32_t*)(global_cpu_vendor) = ebx;
    *(uint32_t*)(global_cpu_vendor + 4) = edx;
    *(uint32_t*)(global_cpu_vendor + 8) = ecx;

    // The techy boot log
    terminal_write("[    0.001] CPU Vendor: ");
    terminal_write(global_cpu_vendor);
    terminal_write(" YAY\n");
}

void detect_cpu_brand() {
    uint32_t regs[4]; 
    global_cpu_brand[48] = '\0';

    for (uint32_t i = 0; i < 3; i++) {
        __asm__ volatile("cpuid" 
                         : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3]) 
                         : "a"(0x80000002 + i));
        
        for (int j = 0; j < 4; j++) {
            // 2. Fill the global variable
            ((uint32_t*)global_cpu_brand)[(i * 4) + j] = regs[j];
        }
    }

    // Keep your cool boot log
    terminal_write("[    0.002] CPU Model: ");
    terminal_write(global_cpu_brand);
    terminal_write(" YAY\n");
}