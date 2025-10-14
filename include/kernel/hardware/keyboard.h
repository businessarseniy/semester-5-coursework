#pragma once

#include "kernel/interrupt/idt.h"


typedef void keyboard_listener_t(interrupt_frame_t* frame);

typedef struct {
    void (*init)(void);
    void (*handler)(interrupt_frame_t* frame);
    void (*listen)(keyboard_listener_t* func);
} Keyboard_t;


extern Keyboard_t keyboard;
