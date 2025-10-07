#pragma once

#include <stdint.h>


typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t __reserved;
    uint8_t attributes;
    uint16_t offset_high;
}__attribute__((packed)) idt_descriptor_t;

typedef struct {
    uint16_t size;
    idt_descriptor_t* offset;
}__attribute__((packed)) idtr_t;

typedef struct {
    uint32_t cr3, ds;
    uint32_t edi, esi, ebp, esp_kernel, ebx, edx, ecx, eax;
    uint32_t interrupt, error_code;
    uint32_t eip, cs, eflags, esp_original, ss;
}__attribute__((packed)) interrupt_frame_t;

typedef struct {
    void (*init)(void);
} Idt_t;


extern Idt_t idt;
