#include "ports.h"

void outb(uint16_t port, uint8_t data)
{
    asm volatile ( "outb %0, %1" : : "a"(data), "Nd"(port) );
}

void outw(uint16_t port, uint16_t data)
{
    asm volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

void outl(uint16_t port, uint32_t data)
{
    asm volatile("outl %0, %1" : : "a"(data), "Nd"(port));
}

uint8_t inb(uint16_t port)
{
    uint8_t ret = 0;
        asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

uint16_t inw(uint16_t port)
{
    uint16_t ret = 0;
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint32_t inl(uint16_t port)
{
    uint32_t ret = 0;
    asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}