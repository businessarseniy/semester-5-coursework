#pragma once

#include <stdint.h>


typedef struct {
    void (*init)(void);
} Vfs_t;


extern Vfs_t vfs;
