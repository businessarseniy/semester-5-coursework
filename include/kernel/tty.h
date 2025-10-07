#pragma once

#include <stdint.h>


typedef enum {
    COLOR_RED = 0x4,
    COLOR_WHITE = 0xf,
} tty_color_t;

typedef struct {
    void (*init)(void);
    void (*print)(char* s);
    void (*printf)(const char* fmt, ...);
    void (*color)(tty_color_t color);
} Tty_t;


extern Tty_t tty;
