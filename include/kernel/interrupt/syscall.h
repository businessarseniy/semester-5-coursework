#pragma once

#include "kernel/interrupt/idt.h"


enum {
    SYSCALL_EXIT, SYSCALL_CLOSE, SYSCALL_EXECVE, SYSCALL_FORK,
    SYSCALL_FSTAT, SYSCALL_GETPID, SYSCALL_ISATTY, SYSCALL_KILL,
    SYSCALL_LINK, SYSCALL_LSEEK, SYSCALL_OPEN, SYSCALL_READ,
    SYSCALL_SBRK, SYSCALL_STAT, SYSCALL_TIMES, SYSCALL_UNLINK,
    SYSCALL_WAIT, SYSCALL_WRITE
};

typedef struct {
    void (*handler)(interrupt_frame_t* frame);
} Syscall_t;


extern Syscall_t syscall;
