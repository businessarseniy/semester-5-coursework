#pragma once

#include <stdint.h>


uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
void insb(uint16_t port, void* addr, int count);
void insw(uint16_t port, void* addr, int count);

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
void outsb(uint16_t port, void* addr, int count);
void outsw(uint16_t port, void* addr, int count);
