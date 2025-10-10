#pragma once

#include <stdint.h>


typedef struct {
    void (*init)(void);
    int (*open)(char* file, int flags, int mode);
    int (*read)(int fd, void* buffer, int size);
    int (*close)(int fd);
} Vfs_t;


extern Vfs_t vfs;
