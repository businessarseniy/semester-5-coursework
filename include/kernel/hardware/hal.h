#pragma once

#include "kernel/interrupt/idt.h"


typedef struct {
    void (*init)(void);
    void (*handler)(interrupt_frame_t* frame);
} Hal_t;


extern Hal_t hal;
