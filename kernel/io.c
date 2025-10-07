#include "kernel/io.h"


uint8_t inb(uint16_t port){
    uint8_t value;
    asm volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );
    return value;
}

uint16_t inw(uint16_t port){
    uint16_t value;
    asm volatile (
        "inw %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );
    return value;
}

void insb(uint16_t port, void* addr, int count){
    asm volatile (
        "rep insb"
        : "+D"(addr), "+c"(count)
        : "d"(port)
        : "memory"
    );
}

void insw(uint16_t port, void* addr, int count){
    asm volatile (
        "rep insw"
        : "+D"(addr), "+c"(count)
        : "d"(port)
        : "memory"
    );
}

void outb(uint16_t port, uint8_t value){
    asm volatile (
        "outb %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

void outw(uint16_t port, uint16_t value){
    asm volatile (
        "outw %0, %1"
        :
        : "a"(value), "Nd"(port)
    );
}

void outsb(uint16_t port, void* addr, int count){
    asm volatile(
        "rep outsb"
        : "+S"(addr), "+c"(count)
        : "d"(port)
        : "memory"
    );
}

void outsw(uint16_t port, void* addr, int count){
    asm volatile(
        "rep outsw"
        : "+S"(addr), "+c"(count)
        : "d"(port)
        : "memory"
    );
}
