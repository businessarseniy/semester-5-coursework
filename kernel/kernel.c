#include "kernel/filesystem/vfs.h"
#include "kernel/hardware/hal.h"
#include "kernel/interrupt/idt.h"
#include "kernel/memory/gdt.h"
#include "kernel/memory/heap.h"
#include "kernel/memory/paging.h"
#include "kernel/memory/physical.h"
#include "kernel/memory/tss.h"
#include "kernel/multitasking/scheduler.h"
#include "kernel/tty.h"
extern void switch_ring3(uint32_t entry, uint32_t stack_top);


uint32_t stack[1024] = {};
void main(void){
    short volatile* vga = (void*)0xb8000;
    short color = 0xab;
    while (1){
        vga[0] = (color << 8) | 'x';
        color *= 1337;
        color &= 0xff;
    }
}

void foo(void){
    // pid=2
    short volatile* vga = (void*)0xb8000;
    short color = 0x01;
    for (int i = 0; i < 10000000; ++i){
        vga[77] = (color << 8) | 'f';
        color *= 1337;
        color &= 0xff;
    }
    asm volatile(
        "mov $16, %eax\n"
        "int $0x80\n"
    );
}
void baz(void){
    // pid=3
    short volatile* vga = (void*)0xb8000;
    short color = 0x12;
    for (int i = 0; i < 10000000; ++i){
        vga[78] = (color << 8) | 'z';
        color *= 1337;
        color &= 0xff;
    }
    asm volatile(
        "mov $16, %eax\n"
        "int $0x80\n"
    );
}
void bar(void){
    // pid=4
    short volatile* vga = (void*)0xb8000;
    short color = 0x23;
    for (int i = 0; i < 10000000; ++i){
        vga[79] = (color << 8) | 'r';
        color *= 1337;
        color &= 0xff;
    }
    asm volatile(
        "mov $16, %eax\n"
        "int $0x80\n"
    );
}

void ksmain(void){
    tty.print("Now kmain runs in scheduler\n");
    tty.print("Switch to userspace (ring3) via iret\n");
    switch_ring3((uint32_t)&main, (uint32_t)&stack[1024]);
    while (1){}
}

void kmain(void){
    tty.init();
    tty.print("Memory allocations initialization\n");
    palloc.init();
    halloc.init();
    tty.print("Setup hardware components\n");
    hal.init();
    tty.print("VFS initialization\n");
    vfs.init();
    tty.print("Going protected mode with flat segmentation model\n");
    gdt.init();
    tss.init();
    tty.print("Protected mode, need to implement interrupts\n");
    // tty.print("Enable paging\n");
    // paging.init();
    tty.printf("Scheduler initialization, pending EIP=%x\n", &ksmain);
    scheduler.init();
    scheduler.create((uint32_t)&ksmain);
    /* process_control_block_t* f = */scheduler.create((uint32_t)&foo);
    /* process_control_block_t* z = */scheduler.create((uint32_t)&baz);
    /* process_control_block_t* r = */scheduler.create((uint32_t)&bar);
    idt.init();
    tty.print("Interrupts will happen, including syscalls\n");
    while (1){}
}
