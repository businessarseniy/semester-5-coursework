#pragma once

#include <stdint.h>


typedef struct {
    void (*init)(void);
    void* (*malloc)(int size);
    void (*free)(void* ptr);
} VAlloc_t;


extern VAlloc_t valloc;
