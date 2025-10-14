#include "kernel/hardware/keyboard.h"
#include "kernel/io.h"
#include "kernel/tty.h"


static void init(void){
    // Some KB initialization
}

static void handler(interrupt_frame_t* frame){
    // Driver implementation
    uint8_t scancode = inb(0x60);
    tty.printf("Scancode: %x", scancode);
}


Keyboard_t keyboard = {
    .init = &init,
    .handler = &handler,
};
