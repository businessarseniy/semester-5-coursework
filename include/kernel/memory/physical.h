#pragma once

#include <stdint.h>


typedef struct {
    void (*init)(void);
    void* (*malloc)(int size);
    void (*free)(void* ptr);
} PAlloc_t;


extern PAlloc_t palloc;
