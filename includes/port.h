#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>

static inline uint8_t inb(uint16_t port);
static inline void outb(uint16_t port, uint8_t data);

#endif // PORTS_H
