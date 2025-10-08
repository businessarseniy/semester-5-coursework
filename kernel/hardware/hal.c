#include "kernel/hardware/hal.h"
#include "kernel/hardware/keyboard.h"
#include "kernel/hardware/pic.h"
#include "kernel/hardware/pio.h"
#include "kernel/hardware/pit.h"
#include "kernel/tty.h"


static void init(void){
    // Initialize here hardware components
    pic.init();
    pit.init();
    pio.init();
    keyboard.init();
}

static void handler(interrupt_frame_t* frame){
    // Do hardware tasks based on interrupt
    switch (frame->interrupt - 0x20){
        case 0:
            pit.handler(frame);
            break;
        case 1:
            keyboard.handler(frame);
            break;
        case 14:
            pio.handler(frame);
            break;
        default:
            // Just masking or logging what happened
            tty.printf("Unhandled IRQ %d\n", frame->interrupt - 0x20);
            break;
    }
}


Hal_t hal = {
    .init = &init,
    .handler = &handler,
};
