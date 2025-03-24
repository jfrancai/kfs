#include "keyboard.h"
#include "ports.h"
#include "terminal.h"

#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_DATA_PORT    0x60

// Arrow key scancodes
#define SC_UP    0x48
#define SC_DOWN  0x50
#define SC_LEFT  0x4B
#define SC_RIGHT 0x4D

char scancode_to_char[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',  
    '\t',  
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',  
    0,    
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,    
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 
    0,    
    '*',
    0,    
    ' ',  
    0,    
};


void (*key_handlers[128])(void) = { 0 };


void move_cursor_up()    { if (terminal.row > 0) terminal.row--; update_cursor(); }
void move_cursor_down()  { if (terminal.row < VGA_HEIGHT - 1) terminal.row++; update_cursor(); }
void move_cursor_left()  { if (terminal.column > 0) terminal.column--; update_cursor(); }
void move_cursor_right() { if (terminal.column < VGA_WIDTH - 1) terminal.column++; update_cursor(); }


// Initialize the key handler table
void init_key_handlers() {
    key_handlers[SC_UP] = move_cursor_up;
    key_handlers[SC_DOWN] = move_cursor_down;
    key_handlers[SC_LEFT] = move_cursor_left;
    key_handlers[SC_RIGHT] = move_cursor_right;
}


void handle_scancode(uint8_t scancode)
{
    static uint8_t alt_pressed = 0;

    if (scancode == 0x38) { // ALT pressed
        alt_pressed = 1;
    } else if (scancode == 0xB8) { // ALT released
        alt_pressed = 0;
    } else if (alt_pressed && scancode >= 0x3B && scancode <= 0x3D) { 
        switch_screen(scancode - 0x3B);
    } else {
        if (scancode < 128 && key_handlers[scancode]) {
            key_handlers[scancode]();
        } else {
            char c = scancode_to_char[scancode];
            if (c) {
                terminal_putchar(c);
            }
        }
    }
}

void poll_keyboard()
{
    if (inb(KEYBOARD_STATUS_PORT) & 1) 
    {
        uint8_t scancode = inb(KEYBOARD_DATA_PORT);
        handle_scancode(scancode);
    }
}
