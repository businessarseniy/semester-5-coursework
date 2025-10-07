#include "kernel/memory/gdt.h"
#include "kernel/memory/tss.h"


static tss_t __tss = {};
static uint32_t stack[1024] = {};

static void init(void){

    __tss.ss0 = 0x10;
    __tss.esp0 = (uint32_t)&stack[1024];
    // __tss.iopb = sizeof(__tss) << 16;

    // GDT entry
    gdt.write(5, (uint32_t)&__tss, sizeof(__tss) - 1, 0x89, 0);

    asm volatile(
        "mov $0x28, %%ax\n"
        "ltr %%ax\n"
        :
        :
        : "eax"
    );
}


Tss_t tss = {
    .init = &init,
};
