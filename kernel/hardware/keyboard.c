#include "kernel/hardware/keyboard.h"


static void init(void){
    // Some KB initialization
}

static void handler(interrupt_frame_t* frame){
    // Driver implementation
}


Keyboard_t keyboard = {
    .init = &init,
    .handler = &handler,
};
