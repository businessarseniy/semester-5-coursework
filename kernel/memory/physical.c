#include "kernel/memory/physical.h"
#include "kernel/tty.h"


static uint32_t const __start = 0x400000;
static uint32_t const __end = 0x800000;
static uint32_t __bitmap[32] = {};


static void* malloc(int size){
    // Ignore size, just allocate
    for (int i = 0; i < 32; ++i){
        uint32_t bitmap = __bitmap[i];
        for (int j = 0; j < 32; ++j){
            if (0 == (bitmap & (1U << j))){
                __bitmap[i] |= (1U << j);
                uint32_t ptr = __start + 0x1000 * (32 * i + j);
                return (void*)ptr;
            }
        }
    }
    return (void*)0;
}

static void free(void* ptr){
    int offset = ((uint32_t)ptr - __start) / 0x1000;
    int index = offset / 32;
    int bit = offset % 32;
    __bitmap[index] ^= 1U << bit;
}

static void init(void){
    void* a = malloc(-1);
    void* b = malloc(-1);
    tty.printf("Allocated: %x, %x\n", a, b);
    free(b);
    void* c = malloc(-1);
    tty.printf("New alloc: %x\n", c);
    free(a);
    free(c);
}


PAlloc_t palloc = {
    .init = &init,
    .malloc = &malloc,
    .free = &free,
};
