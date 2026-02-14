#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>

// CPU Detection
void detect_cpu();

// ATA Disk Detection
void detect_ata();

// PCI Bus Probing
void probe_pci();

#endif
