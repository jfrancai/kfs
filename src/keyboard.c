#include "keyboard.h"
#include "ports.h"
#include "terminal.h"

static bool shift_pressed = false;
static bool alt_pressed = false;

char scancode_to_char_normal[MAX_SCANCODE] = {
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

const char scancode_to_char_shifted[MAX_SCANCODE] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',  
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',  
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 
    0, '*', 0, ' ', 0
};


// Function pointer array for special key handlers
void (*key_handlers[MAX_SCANCODE])(void) = { 0 };

// Cursor movement handlers
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

// Process scancode input
void handle_scancode(uint8_t scancode) {
    if (scancode == SC_LSHIFT || scancode == SC_RSHIFT) { 
        shift_pressed = true;
    } else if (scancode == SC_LSHIFT_RELEASE || scancode == SC_RSHIFT_RELEASE) { 
        shift_pressed = false;
    } else if (scancode == SC_ALT) { 
        alt_pressed = true;
    } else if (scancode == SC_ALT_RELEASE) { 
        alt_pressed = false;
    } else if (alt_pressed && scancode >= SC_F1 && scancode <= SC_F3) { 
        switch_screen(scancode - SC_F1);
    } else if (scancode < MAX_SCANCODE) {
        if (key_handlers[scancode]) {
            key_handlers[scancode]();
        } else {
            char c = shift_pressed ? scancode_to_char_shifted[scancode] : scancode_to_char_normal[scancode];
            if (c) {
                terminal_putchar(c);
            }
        }
    }
}

// Poll keyboard input
void poll_keyboard() {
    if (inb(KEYBOARD_STATUS_PORT) & 1) { 
        uint8_t scancode = inb(KEYBOARD_DATA_PORT);
        handle_scancode(scancode);
    }
}
