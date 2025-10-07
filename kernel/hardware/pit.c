#include "kernel/hardware/pit.h"
#include "kernel/tty.h"


static pit_listener_t* __listeners[4] = {};
static int __pos = 0;


static void init(void){
    // Adjust timer for frequency
}

static void handler(interrupt_frame_t* frame){
    for (int i = 0; i < 4; ++i){
        pit_listener_t* func = __listeners[i];
        if ((void*)0 != func) func(frame);
    }
}

static void listen(pit_listener_t* func){
    // Subscribtion to PIT events
    if (__pos < 4) __listeners[__pos++] = func;
}

Pit_t pit = {
    .init = &init,
    .handler = &handler,
    .listen = &listen,
};
