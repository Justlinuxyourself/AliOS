ENTRY(_start)

SECTIONS
{
	/* The bootloader will look for the Multiboot header here. */
	. = 1M;

	.text BLOCK(4K) : ALIGN(4K) { *(.multiboot) *(.text) }
	.rodata BLOCK(4K) : ALIGN(4K) { *(.rodata) }
	.data BLOCK(4K) : ALIGN(4K) { *(.data) }
	.bss BLOCK(4K) : ALIGN(4K) { *(COMMON) *(.bss) }
}
