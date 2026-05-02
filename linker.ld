ENTRY(_start)

SECTIONS {
    /* 1. Start at a low physical offset for the header */
    . = 0x1000; 

    .multiboot : 
    {
        /* This MUST be at the beginning of the binary file */
        KEEP(*(.multiboot))
    }

    /* 2. Now jump to 1MB for the actual kernel code */
    . = 1M;
    _kernel_start = .;

    .text : ALIGN(4K)
    {
        *(.text)
    }

    .rodata : ALIGN(4K) { *(.rodata) }
    .data   : ALIGN(4K) { *(.data) }
    .bss    : ALIGN(4K) { *(.bss) *(COMMON) }

    _kernel_end = .;

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}
