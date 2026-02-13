/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             
.set MEMINFO,  1<<1             
.set FLAGS,    ALIGN | MEMINFO  
.set MAGIC,    0x1BADB002       
.set CHECKSUM, -(MAGIC + FLAGS) 

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* Allocate room for a small stack. */
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.global _start
_start:
    mov $stack_top, %esp    # Initialize stack pointer
    call kernel_main

    cli
1:  hlt
    jmp 1b

# --- GDT Flush ---
.global gdt_flush
gdt_flush:
    mov 4(%esp), %eax
    lgdt (%eax)
    mov $0x10, %ax      # 0x10 is the offset to our data segment
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    ljmp $0x08, $.flush # 0x08 is the offset to our code segment
.flush:
    ret

# --- IDT Flush ---
.global idt_flush
idt_flush:
    mov 4(%esp), %eax
    lidt (%eax)        # Load the IDT pointer
    ret

# --- Macros for Exceptions (ISRs) ---
.macro ISR_NOERRCODE num
  .global isr\num
  isr\num:
    cli
    push $0
    push $\num
    jmp isr_common_stub
.endm

# --- Macro for Hardware Interrupts (IRQs) ---
.macro IRQ num, target
  .global irq\num
  irq\num:
    cli
    push $0
    push $\target
    jmp irq_common_stub
.endm

# --- Define the ISRs and IRQs ---
ISR_NOERRCODE 0     # Divide by Zero Exception

# IRQ 0 (Timer) is handled manually below to avoid macro conflicts
IRQ 1, 33           # Keyboard (IRQ1 maps to IDT 33)

# --- Exception Stub (Calls isr_handler in isr.c) ---
.extern isr_handler
isr_common_stub:
    pusha
    mov %ds, %ax
    push %eax
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    call isr_handler
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    popa
    add $8, %esp
    iret

# --- Hardware IRQ Stub (Calls keyboard_handler in isr.c) ---
.extern keyboard_handler
.global irq_common_stub
irq_common_stub:
    pusha
    mov %ds, %ax
    push %eax
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    call keyboard_handler
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    popa
    add $8, %esp
    iret
.extern rtc_handler
.global irq8
irq8:
    pusha
    mov %ds, %ax
    push %eax
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    call rtc_handler
    pop %eax
    mov %ax, %ds
    mov %ax, %es
    popa
    iret