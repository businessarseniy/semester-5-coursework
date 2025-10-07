#include "kernel/hardware/hal.h"
#include "kernel/interrupt/exception.h"
#include "kernel/interrupt/idt.h"
#include "kernel/interrupt/syscall.h"
extern uint32_t isr[256];
extern void idt_commit(idtr_t*);


static idt_descriptor_t __idt[256] = {};
static idtr_t __idtr = {};


extern void interrupt_handler(interrupt_frame_t* frame){
    if (frame->interrupt < 0x20){
        // Exceptions
        exception.handler(frame);
        return;
    }
    if (frame->interrupt < 0x30){
        // Hardware
        hal.handler(frame);
        return;
    }
    if (0x80 == frame->interrupt){
        syscall.handler(frame);
        return;
    }
}

static void init(void){
    for (int i = 0; i < 0x30; ++i){
        uint32_t offset = isr[i];
        idt_descriptor_t descriptor = {
            .selector = 0x08, .attributes = 0x8e, .__reserved = 0,
            .offset_high = (offset >> 16) & 0xffff, .offset_low = offset & 0xffff,
        };
        __idt[i] = descriptor;
    }
    
    // Add syscall
    uint32_t offset = isr[0x80];
    idt_descriptor_t descriptor = {
        .selector = 0x08, .attributes = 0xef, .__reserved = 0,
        .offset_high = (offset >> 16) & 0xffff, .offset_low = offset & 0xffff,
    };
    __idt[0x80] = descriptor;
    
    __idtr.size = sizeof(__idt) - 1;
    __idtr.offset = &__idt[0];
    
    idt_commit(&__idtr);
}


Idt_t idt = {
    .init = &init,
};
