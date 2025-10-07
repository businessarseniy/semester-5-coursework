#pragma once

#include "kernel/interrupt/idt.h"


typedef void pit_listener_t(interrupt_frame_t* frame);

typedef struct {
    void (*init)(void);
    void (*handler)(interrupt_frame_t* frame);
    void (*listen)(pit_listener_t* func);
} Pit_t;


extern Pit_t pit;
