#pragma once

#include <stdint.h>


typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t flags;
    uint8_t base_high;
}__attribute__((packed)) gdt_descriptor_t;

typedef struct {
    uint16_t size;
    gdt_descriptor_t* offset;
}__attribute__((packed)) gdtr_t;

typedef struct {
    void (*init)(void);
    void (*write)(int index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
} Gdt_t;


extern Gdt_t gdt;
