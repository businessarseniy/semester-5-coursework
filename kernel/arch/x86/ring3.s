.section .text
.globl switch_ring3
switch_ring3:
    push %ebp
    mov %esp, %ebp
    # userspace function pointer
    mov 0x8(%ebp), %ebx
    # userspace stack pointer
    mov 0xc(%ebp), %ecx

    # set data registers
    mov $0x23, %eax
    mov %ax, %ds
    mov %ax, %es

    # stack frame for iret
    push $0x23
    push %ecx
    pushf
    push $0x1b
    push %ebx
    iret
