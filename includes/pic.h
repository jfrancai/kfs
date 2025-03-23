#ifndef PIC_H
#define PIC_H
#include <stdint.h>

void PIC_sendEOI(uint8_t irq);
uint16_t pic_get_irr(void);
uint8_t inb(uint16_t port);

#endif
