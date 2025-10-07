#pragma once

#include "kernel/interrupt/idt.h"


typedef struct {
    void (*handler)(interrupt_frame_t* frame);
} Exception_t;


extern Exception_t exception;
