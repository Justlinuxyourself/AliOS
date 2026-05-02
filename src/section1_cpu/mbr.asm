[BITS 16]
[ORG 0x7C00]

_start:
    ; --- Step 1: Normalize Segments ---
    cli                         ; Disable interrupts during setup
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x7C00              ; Stack grows down from MBR
    sti

    ; --- Step 2: Visual Life Sign ---
    mov ah, 0x0E
    mov al, '!'                 ; '!' = MBR is executing
    int 0x10

    ; --- Step 3: Check for LBA Support ---
    ; Some very old BIOS might not support AH=42h
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, 0x80                ; HDD 1
    int 0x13
    jc lba_not_supported

    ; --- Step 4: Load Kernel Head via DAP ---
    ; We load 127 sectors (enough for boot.asm and kernel init)
    mov ah, 0x42                ; Extended Read
    mov dl, 0x80                ; Drive number (0x80 = C:)
    mov si, disk_address_packet ; DS:SI points to the DAP
    int 0x13
    jc disk_error               ; Jump if carry flag (read failed)

    ; --- Step 5: Success Sign & Jump ---
    mov ah, 0x0E
    mov al, 'J'
    int 0x10

    ; Instead of jumping to 0x7E00 (the start of the ELF header)
    ; Jump to 0x7E40 (0x7E00 + 64 bytes) to skip the ELF64 header
    jmp 0x0000:0x7E40 


; --- Error Handlers ---
lba_not_supported:
    mov al, 'L'                 ; 'L' = No LBA support
    jmp error_halt

disk_error:
    mov al, 'E'                 ; 'E' = Disk Read Error
    jmp error_halt

error_halt:
    mov ah, 0x0E
    int 0x10
    hlt
    jmp $

; --- Disk Address Packet (DAP) ---
; This structure tells the BIOS where to find our 4MB kernel
align 4
disk_address_packet:
    db 0x10                     ; Size of packet (16 bytes)
    db 0x00                     ; Reserved (always 0)
    dw 127                      ; Number of sectors to read (Stage 1 Head)
    dw 0x7E00                   ; Target Offset (Load to 0x7E00)
    dw 0x0000                   ; Target Segment
    dq 2048                     ; Starting LBA (Sector 2048)

; --- MBR Padding ---
times 510-($-$$) db 0
dw 0xAA55                       ; The Magic Boot Signature
