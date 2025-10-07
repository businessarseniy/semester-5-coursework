#include "kernel/memory/virtual.h"


static void init(void){}

static void* malloc(int size){}

static void free(void* ptr){}


VAlloc_t valloc = {
    .init = &init,
    .malloc = &malloc,
    .free = &free,
};
