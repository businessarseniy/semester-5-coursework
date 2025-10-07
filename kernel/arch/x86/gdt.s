.section .text
.globl gdt_commit
.type gdt_commit, @function
gdt_commit:
    push %ebp
    mov %esp, %ebp

    mov 0x8(%ebp), %eax
    lgdt (%eax)

    mov %cr0, %eax
    or $1, %eax
    mov %eax, %cr0

    jmp $0x08, $1f
1:  mov $0x10, %eax
    mov %ax, %ds
    mov %ax, %ss

    leave
    ret