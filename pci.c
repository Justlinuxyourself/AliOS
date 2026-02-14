#include "hardware.h"
#include "common.h"

// Helper to read PCI configuration space
uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;

    // Create configuration address
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    outl(0xCF8, address);
    // (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff)
    tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

void probe_pci() {
    terminal_write("[    0.150] PCI: Probing PCI bus...\n");
    for(uint32_t bus = 0; bus < 256; bus++) {
        for(uint32_t slot = 0; slot < 32; slot++) {
            uint16_t vendor = pci_config_read_word(bus, slot, 0, 0);
            if(vendor != 0xFFFF) {
                uint16_t device = pci_config_read_word(bus, slot, 0, 2);
                
                terminal_write("Found PCI Device: Vendor 0x8086 Device 0x1237\n");
                // (Optional: You can use a hex-to-string function here later)
            }
        }
    }
}
