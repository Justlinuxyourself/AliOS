#include "../section1_cpu/io.h"
#include <stdint.h>
#define STATUS_BSY 0x80
#define STATUS_RDY 0x40
#define STATUS_DRQ 0x08
#define STATUS_DF 0x20
#define STATUS_ERR 0x01
extern uint16_t inw(uint16_t port);
static void ide_wait_bsy(){
	while (inb(0x1F7) & STATUS_BSY);
}
static void ide_wait_drq() {
	while (!(inb(0x1F7) & STATUS_DRQ));
}
void ide_read_sector_bytes(uint32_t lba, uint8_t* buffer) {
	ide_wait_bsy();
	outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
	outb(0x1F2, 1);
	outb(0x1F3, (uint8_t)lba);
	outb(0x1F4, (uint8_t)(lba >> 8));
	outb(0x1F5, (uint8_t)(lba >> 16));
	outb(0x1F7, 0x20);
	ide_wait_bsy();
	ide_wait_drq();
	for (int i = 0; i < 256; i++) {
		uint16_t data = inw(0x1F0);
		buffer[i * 2] = (uint8_t)data;
		buffer[i * 2 + 1] = (uint8_t)(data >> 8);
	}
}
void ide_write_sector_bytes(uint32_t lba, uint8_t* buffer) {
	ide_wait_bsy();
	outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
	outb(0x1F2, 1);
	outb(0x1F3, (uint8_t)lba);
	outb(0x1F4, (uint8_t)(lba >> 8));
	outb(0x1F5, (uint8_t)(lba >> 16));
	outb(0x1F7, 0x30);
	ide_wait_bsy();
	ide_wait_drq();
	for (int i = 0; i < 256; i++) {
		uint16_t data = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
		outw(0x1F0, data);
	}
	outb(0x1F7, 0xE7);
	ide_wait_bsy();
}
