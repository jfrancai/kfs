#ifndef TERMINAL_H
#define TERMINAL_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void terminal_initialize(void);
void terminal_writestring(const char* data);
#endif
