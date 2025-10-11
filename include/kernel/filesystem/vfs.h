#pragma once

#include <stdint.h>


typedef struct {
    void (*init)(void);
    int (*open)(char* file, int flags, int mode);
    int (*read)(int fd, void* buffer, int size);
    int (*close)(int fd);
    int (*lseek)(int fd, int pos, int whence)
} Vfs_t;


extern Vfs_t vfs;
