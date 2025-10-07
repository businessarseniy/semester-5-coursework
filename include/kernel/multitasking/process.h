#pragma once

#include <stdint.h>


typedef enum {
    SCOPE_KERNEL, SCOPE_USERSPACE
} process_scope_t;

typedef enum {
    STATE_RUNNING, STATE_READY, STATE_WAITING, STATE_START, STATE_DONE
} process_state_t;

typedef struct {
    uint32_t eax, ebx, ecx, edx, edi, esi, ebp, eflags;
} process_context_t;
