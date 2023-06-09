#ifndef __PIC_H__
#define __PIC_H__

#include "system.h"
#include "printf.h"
#include "ports.h"

// PIC ports
// Master PIC
#define PIC1_COMMAND 0x20 
#define PIC1_DATA    0x21

// Slave PIC
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

// PIC command
#define PIC_EOI      0x20

void init_pic();
void pic_eoi(uint8_t irq);

#endif /*__PIC_H__*/