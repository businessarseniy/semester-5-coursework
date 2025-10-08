#include "kernel/hardware/pio.h"
#include "kernel/memory/heap.h"
#include "kernel/io.h"
#include "kernel/queue.h"
#include "kernel/tty.h"


static queue_t __tasks = {};
static pio_task_t* __current = (void*)0;


static void memset(void* dst, int size, int val){
    uint8_t* p_dst = dst;
    for (int i = 0; i < size; ++i){
        p_dst[i] = (uint8_t)val;
    }
}

static inline void wait(void){
    uint8_t v;
    while (1){
        v = inb(PIO_IO_STATUS);
        if (!(v & 0x80) && (v & 0x08)) break;
    }
}

static void execute_task(void){
    if ((void*)0 == __current){
        __current = __tasks.pop(&__tasks);
    }

    outb(PIO_IO_DRIVE, 0xe0 | ((__current->lba >> 24) & 0xf));
    outb(PIO_IO_COUNT, __current->count_total);
    outb(PIO_IO_LBAlow, __current->lba & 0xff);
    outb(PIO_IO_LBAmid, (__current->lba >> 8) & 0xff);
    outb(PIO_IO_LBAhi, (__current->lba >> 16) & 0xff);
    outb(PIO_IO_COMMAND, 0x20);
    for (int i = 0; i < __current->count_total; ++i){
        wait();
        insw(PIO_IO_DATA, __current->buffer + i*512, 256);
    }
    halloc.free(__current);
    __current = (void*)0;
}

static void read(void* dst, uint32_t lba, int count){
    pio_task_t* task = halloc.malloc(sizeof(pio_task_t));
    memset(task, sizeof(pio_task_t), 0);
    task->type = PIO_READ;
    task->buffer = dst;
    task->lba = lba;
    task->count_total = count;
    __tasks.push(&__tasks, task);
    execute_task();
}

static void write(void* src, uint32_t lba, int count){
    pio_task_t* task = halloc.malloc(sizeof(pio_task_t));
    memset(task, sizeof(pio_task_t), 0);
    task->type = PIO_WRITE;
    task->buffer = src;
    task->lba = lba;
    task->count_total = count;
    __tasks.push(&__tasks, task);
    execute_task();
}

static void handler(interrupt_frame_t* frame){
    // tty.print("PIO handler!\n");
}

static void init(void){
    queue_init(&__tasks);
}


Pio_t pio = {
    .init = &init,
    .read = &read,
    .write = &write,
    .handler = &handler,
};
