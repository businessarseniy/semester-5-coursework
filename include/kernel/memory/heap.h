#pragma once

#include <stdint.h>


typedef struct __heap_block_header {
    uint32_t size;
    uint32_t available;
    struct __heap_block_header* next;
}__attribute__((packed)) heap_block_header_t;

typedef struct {
    void (*init)(void);
    void* (*malloc)(int size);
    void (*free)(void* ptr);
} HAlloc_t;


extern HAlloc_t halloc;
