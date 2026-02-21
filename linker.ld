ENTRY(_start)
OUTPUT_FORMAT(elf64-x86-64)

SECTIONS {
    . = 1M;
    .text : {
        KEEP(*(.multiboot_header))
        *(.text)
    }
    .data : { *(.data) }
    .bss  : { *(.bss) }
}