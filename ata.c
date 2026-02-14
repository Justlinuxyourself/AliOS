#include "hardware.h"
#include "common.h" // for outb/inb/terminal_write

void detect_ata() {
    // Select Master drive on Primary Bus (Port 0x1F6)
    outb(0x1F6, 0xA0); 
    
    // Send the IDENTIFY command (0xEC)
    outb(0x1F7, 0xEC); 

    // Wait for status
    uint8_t status = inb(0x1F7);
    if (status == 0) {
        terminal_write("[    0.050] ATA: No drive detected on primary bus.\n");
        return;
    }

    // Wait for the drive to clear the Busy bit and set Data Request
    while (inb(0x1F7) & 0x80); 
    
    terminal_write("[    0.120] ATA: HARD DISK FOUND YIPPIE\n");
}
