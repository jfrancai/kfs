#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void init_key_handlers(void);
void poll_keyboard(void);
void handle_scancode(uint8_t scancode);

#endif // KEYBOARD_H
