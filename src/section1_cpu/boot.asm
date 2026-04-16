; --- src/section1_cpu/boot.asm ---
; LINKED TO NOTEBOOK: SECTION I - Bootloader & Paging
;Copyright (c) 2026 Ali  
;All rights reserved.
[bits 32]
section .multiboot_header
align 8
header_start:
    dd 0xe85250d6                ; Magic number (Multiboot 2)
    dd 0                         ; Architecture (i386)
    dd header_end - header_start ; Length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; Checksum
    dw 0, 0
    dd 8
header_end:

section .text
global _start
extern kernel_main

_start:
    ; 1. CRITICAL: Stop interrupts immediately. 
    ; Real hardware sends timer ticks that will crash you during the switch.
    cli 

    ; 2. Initialize stack using your .bss label
    mov esp, stack_top

    ; 3. Clear Page Tables in .bss
    ; Using 'pml4' instead of 0x1000 ensures safety on real silicon.
    mov edi, pml4
    xor eax, eax
    mov ecx, 3072                ; Clear 3 pages (4096 * 3 / 4)
    rep stosd

    ; 4. Build the 4-Level Paging Hierarchy
    ; Flags: 0x3 (Present + Read/Write)
    mov eax, pdpt
    or eax, 0x3
    mov [pml4], eax              ; PML4 Entry 0 points to PDPT

    mov eax, pdpt                ; Pointer to PDPT
    mov eax, pdt
    or eax, 0x3
    mov [pdpt], eax              ; PDPT Entry 0 points to PDT
    
    ; 5. Identity Map 10MB of RAM using 2MB "Huge Pages"
    ; 0x83 = Present + R/W + Huge Page (Bit 7)
    mov dword [pdt], 0x00000083      ; 0MB - 2MB (Kernel)
    mov dword [pdt + 8], 0x00200083  ; 2MB - 4MB (Heap Start)
    mov dword [pdt + 16], 0x00400083 ; 4MB - 6MB
    mov dword [pdt + 24], 0x00600083 ; 6MB - 8MB
    mov dword [pdt + 32], 0x00800083 ; 8MB - 10MB

    ; 6. Enable PAE (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; 7. Set CR3 to point to your PML4 in .bss
    mov eax, pml4
    mov cr3, eax

    ; 8. Enable Long Mode in EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; 9. Enable Paging in CR0 to activate 64-bit mode
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; 10. Load 64-bit GDT and perform the jump
    lgdt [gdt64.ptr]
    jmp 0x08:init_64


[bits 64]
init_64:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    call kernel_main

    
    ; If kernel returns, halt the CPU
.halt:
    hlt
    jmp .halt

section .rodata
gdt64:
    dq 0 ; Null Descriptor
    ; Code Descriptor: Access 0x9A, Flags 0x20 (64-bit)
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) 
.ptr:
    dw $ - gdt64 - 1
    dq gdt64

section .bss
align 4096
pml4: resb 4096
pdpt: resb 4096
pdt:  resb 4096
stack_bottom:
    resb 16384
stack_top:
