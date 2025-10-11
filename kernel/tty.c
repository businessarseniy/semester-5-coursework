#include <stdarg.h>
#include "kernel/tty.h"


static short volatile* const __tty = (void*)0xB8000;
static int const __width = 80;
static int const __height = 25;

static tty_color_t __color = COLOR_WHITE;
static int __row = 0;
static int __col = 0;


static void memset(void* dst, int size, int value){
    uint8_t* p_dst = dst;
    for (int i = 0; i < size; ++i){
        p_dst[i] = (uint8_t)value;
    }
}

static void memcpy(void* dst, void* src, int size){
    uint8_t* p_dst = dst;
    uint8_t* p_src = src;
    for (int i = 0; i < size; ++i){
        p_dst[i] = p_src[i];
    }
}

static void scroll(void){
    if ((__col + 1) >= __width){
        __col = 0;
        __row++;
    }
    if ((__row + 1) >= __height){
        memcpy((void*)&__tty[0], (void*)&__tty[__width], sizeof(short) * __width * (__height - 1));
        memset((void*)&__tty[__width * (__height - 1)], sizeof(short) * __width, 0);
        __row--;
    }
}

static void putchar(char c){
    scroll();
    switch (c){
        case '\n':
            __row++;
        case '\r':
            __col = 0;
            break;
        case '\0':
            break;
        default:
            __tty[__row * __width + __col] = __color << 8 | (short)c;
            __col++;
            break;
    }
}

static void print(char* s){
    for (char* ptr = s; *ptr != '\0'; ++ptr){
        putchar(*ptr);
    }
}

static void sitoa(char* buffer, int digit){
    char* ptr = buffer;
    char stack[16] = {0};
    int pos = 0;
    if (digit < 0){
        *ptr++ = '-';
        digit = -digit;
    }
    do {
        stack[pos++] = '0' + (digit % 10);
        digit /= 10;
    } while (digit > 0);
    while (pos > 0){
        *ptr++ = stack[--pos];
    }
}

static void sxtoa(char* buffer, unsigned int hex){
    char* ptr = buffer;
    char stack[16] = {0};
    int pos = 0;
    *ptr++ = '0';
    *ptr++ = 'x';
    do {
        int val = hex % 16;
        char c = 0;
        if (val < 10){ c = '0' + val; } else { c = 'A' + val - 10; } 
        stack[pos++] = c;
        hex /= 16;
    } while (hex != 0);
    while (pos > 0){
        *ptr++ = stack[--pos];
    }
}

static void printf(const char* format, ...){
    va_list args;
    va_start(args, format);
    for (const char* ptr = format; *ptr != '\0'; ++ptr){
        if (*ptr == '%' && *(ptr+1) != '\0'){
            ++ptr;
            switch (*ptr){
                case 'c': {
                    char c = (char)va_arg(args, int);
                    putchar(c);
                    break;
                }
                case 's': {
                    char* s = va_arg(args, char*);
                    print(s);
                    break;
                }
                case 'd': {
                    int d = va_arg(args, int);
                    char buffer[16] = {0};
                    sitoa(&buffer, d);
                    print(&buffer);
                    break;
                }
                case 'x': {
                    unsigned int x = va_arg(args, unsigned int);
                    char buffer[16] = {0};
                    sxtoa(&buffer, x);
                    print(buffer);
                    break;
                }
            }
        } else {
            putchar(*ptr);
        }
    }
    va_end(args);
}


static void init(void){
    memset(__tty, __width * __height, 0);
    __col = 0;
    __row = 0;
    __color = COLOR_WHITE;
}

static void color(tty_color_t color){
    __color = color;
}

Tty_t tty = {
    .init = &init,
    .putchar = &putchar,
    .print = &print,
    .printf = &printf,
    .color = &color,
};
