#pragma once

#include <stdint.h>


enum {
    PRESENT = 1,
    READWRITE = 2,
    USER = 4,
};

typedef struct {
    void (*init)(void);
    void* (*default_directory)(void);
    void* (*create_directory)(void);
    void (*destroy_directory)(void* directory);
    void* (*create_table)(int index, void* directory, int flags);
    void (*destroy_table)(int index, void* directory);
    void (*create_page)(int index, void* table, int flags);
    void (*destroy_page)(int index, void* table);
} Paging_t;


extern Paging_t paging;
