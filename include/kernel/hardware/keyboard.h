#pragma once

#include "kernel/interrupt/idt.h"


typedef struct {
    void (*init)(void);
    void (*handler)(interrupt_frame_t* frame);
} Keyboard_t;


extern Keyboard_t keyboard;
