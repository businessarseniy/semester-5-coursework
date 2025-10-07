.section .text
.globl idt_commit
.type idt_commit, @function
idt_commit:
    push %ebp
    mov %esp, %ebp

    mov 0x8(%ebp), %eax
    lidt (%eax)

    sti
    leave
    ret
