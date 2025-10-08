#pragma once

#include "kernel/interrupt/idt.h"
#include <stdint.h>

#define PIO_IO 0x1F0
#define PIO_IO_DATA (PIO_IO + 0)
#define PIO_IO_ERROR (PIO_IO + 1)
#define PIO_IO_FEATURES (PIO_IO + 1)
#define PIO_IO_COUNT (PIO_IO + 2)
#define PIO_IO_LBAlow (PIO_IO + 3)
#define PIO_IO_LBAmid (PIO_IO + 4)
#define PIO_IO_LBAhi (PIO_IO + 5)
#define PIO_IO_DRIVE (PIO_IO + 6)
#define PIO_IO_STATUS (PIO_IO + 7)
#define PIO_IO_COMMAND (PIO_IO + 7)

#define PIO_CONTROL 0x3F6
#define PIO_CONTROL_STATUS (PIO_CONTROL + 0)
#define PIO_CONTROL_DEVICE (PIO_CONTROL + 1)
#define PIO_CONTROL_DRIVE (PIO_CONTROL + 1)


typedef enum {
    PIO_READ, PIO_WRITE,
} pio_task_type_t;

typedef struct {
    pio_task_type_t type;
    void* buffer;
    uint32_t lba;
    int count_total;
    int count_done;
} pio_task_t;

typedef struct {
    void (*init)(void);
    void (*read)(void* dst, uint32_t lba, int count);
    void (*write)(void* src, uint32_t lba, int count);
    void (*handler)(interrupt_frame_t* frame);
} Pio_t;


extern Pio_t pio;
