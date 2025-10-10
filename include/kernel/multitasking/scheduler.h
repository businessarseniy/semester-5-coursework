#pragma once

#include "kernel/multitasking/process.h"


typedef struct __process_control_block {
    uint32_t pid;
    process_context_t context;
    uint32_t* stack;
    uint32_t esp;
    uint32_t eip;
    process_state_t state;
    uint32_t exit_code;
    uint32_t signal;
    struct __process_control_block* parent;
    uint32_t child_completed_bitmap;
    uint32_t child_bitmap;
    uint32_t child[32];
    uint32_t* cr3;
} process_control_block_t;

typedef struct {
    void (*init)(void);
    process_control_block_t* (*create)(uint32_t entry);
    process_control_block_t* (*current)(void);
    void (*schedule)(interrupt_frame_t* frame);
    process_control_block_t* (*get)(uint32_t pid);
} Scheduler_t;


extern Scheduler_t scheduler;
