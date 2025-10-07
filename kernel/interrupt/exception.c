#include "kernel/interrupt/exception.h"
#include "kernel/tty.h"


static char* __names[32] = {
    [0] = "Division Error", [1] = "Debug", [2] = "Non-maskable Interrupt", [3] = "Breakpoint",
    [4] = "Overflow", [5] = "Bound Range Exceeded", [6] = "Invalid Opcode", [7] = "Device Not Available",
    [8] = "Double Fault", [9] = "Coprocessor Segment Overrun", [10] = "Invalid TSS", [11] = "Segment Not Present",
    [12] = "Stack-Segment Fault", [13] = "General Protection Fault", [14] = "Page Fault", [15] = "(null)",
    [16] = "x87 Floating-Point Exception", [17] = "Alignment Check", [18] = "Machine Check", [19] = "SIMD Floating-Point Exception",
    [20] = "Virtualization Exception", [21] = "Control Protection Exception", [22] = "(null)", [23] = "(null)",
    [24] = "(null)", [25] = "(null)", [26] = "(null)", [27] = "(null)",
    [28] = "Hypervisor Injection Exception", [29] = "VMM Communication Exception", [30] = "Security Exception", [31] = "(null)",
};


static void handler(interrupt_frame_t* frame){
    tty.color(COLOR_RED);
    tty.printf("Exception: %s!\n", __names[frame->interrupt]);
    tty.printf("    @ %x:%x\n", frame->cs, frame->eip);
    asm volatile("cli");
    asm volatile("hlt");
}


Exception_t exception = {
    .handler = &handler,
};
