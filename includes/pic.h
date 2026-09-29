#ifndef PIC_H
#define PIC_H

#include "stdint.h"

void pic_remap(uint8_t master_offset, uint8_t slave_offset);
void pic_send_eoi(uint8_t irq);
void pic_mask(uint8_t irq);
void pic_unmask(uint8_t irq);
int  pic_is_spurious(uint8_t irq);

#endif
