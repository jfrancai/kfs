#ifndef TERMINAL_H
#define TERMINAL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


void terminal_initialize(void);
void terminal_writestring(const char* data);
void terminal_putchar(char c);
void terminal_writehex(uint8_t num);
void update_cursor();
void poll_keyboard();
void init_key_handlers();

#endif
