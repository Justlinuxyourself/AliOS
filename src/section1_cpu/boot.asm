; --- src/section1_cpu/boot.asm ---
; LINKED TO NOTEBOOK: SECTION I - Bootloader & Paging

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
    ; Initialize stack
    mov esp, stack_top

    ; 1. Clear Page Tables (0x1000 to 0x4000)
    ; This prevents junk data from causing a Triple Fault
    mov edi, 0x1000
    xor eax, eax
    mov ecx, 4096                ; Clear 16KB (PML4, PDPT, and PDT)
    rep stosd

    ; 2. Build the 4-Level Paging Hierarchy
    ; Structure: PML4 [0x1000] -> PDPT [0x2000] -> PDT [0x3000]
    ; Flags: 0x3 (Present + Read/Write)
    mov dword [0x1000], 0x2003   ; PML4 Entry 0 points to PDPT
    mov dword [0x2000], 0x3003   ; PDPT Entry 0 points to PDT
    
    ; 3. Identity Map 10MB of RAM using 2MB "Huge Pages"
    ; 0x83 = Present + R/W + Huge Page (Bit 7)
    mov dword [0x3000], 0x00000083  ; 0MB - 2MB (Kernel)
    mov dword [0x3008], 0x00200083  ; 2MB - 4MB (Heap Start)
    mov dword [0x3010], 0x00400083  ; 4MB - 6MB
    mov dword [0x3018], 0x00600083  ; 6MB - 8MB
    mov dword [0x3020], 0x00800083  ; 8MB - 10MB

    ; 4. Enable PAE (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; 5. Set CR3 to PML4 address
    mov eax, 0x1000
    mov cr3, eax

    ; 6. Enable Long Mode in EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; 7. Enable Paging in CR0
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; 8. Load 64-bit GDT and jump to 64-bit code
    lgdt [gdt64.ptr]
    jmp 0x08:init_64

[bits 64]
init_64:
    ; Ensure segment registers are clean for 64-bit
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    
    ; Call the C kernel
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
stack_bottom:
    resb 16384
stack_top: