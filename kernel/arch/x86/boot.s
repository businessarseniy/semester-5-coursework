.equ ALIGN, 1 << 0
.equ MEMINFO, 1 << 1
.equ MAGIC, 0x1BADB002
.equ FLAGS, ALIGN | MEMINFO
.equ CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot.data, "aw"
.align 4
.int MAGIC
.int FLAGS
.int CHECKSUM

# Allocate the initial stack.
.section .bss, "aw", @nobits
.align 16
.space 4096
__stack:

# The kernel entry point.
.section .multiboot.text, "a"
.globl _start
.type _start, @function
_start:
	mov $__stack, %esp
	call kmain

	cli
1:	hlt
	jmp 1b