#pragma once

#include <stdint.h>


typedef struct {
    uint32_t link, esp0, ss0, esp1, ss1, esp2, ss2, cr3,
        eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi,
        es, cs, ss, ds, fs, gs, ldtr, iopb, ssp;
}__attribute__((packed)) tss_t;

typedef struct {
    void (*init)(void);
} Tss_t;


extern Tss_t tss;
