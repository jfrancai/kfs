#include "keyboard.h"
#include "ports.h"
#include "terminal.h"
#include "shell.h"
#include "idt.h"
#include "pic.h"
#include "signal.h"

#define KBD_BUFFER_SIZE 128

static bool shift_pressed = false;
static bool alt_pressed = false;
static bool ctrl_pressed = false;

/* Scancodes filled by the IRQ handler, drained by the main loop. */
static volatile uint8_t kbd_buffer[KBD_BUFFER_SIZE];
static volatile uint32_t kbd_head;   /* next write (IRQ) */
static volatile uint32_t kbd_tail;   /* next read (main loop) */

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
    '\t', 
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 
	'\n',  
    0, 
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, 
	'|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 
    0, 
	'*', 0, ' ',
	0
};


// Function pointer array for special key handlers
void (*key_handlers[MAX_SCANCODE])(void) = { 0 };

// Cursor movement handlers
static void move_cursor_up(void)    { if (terminal.row > 0) terminal.row--; update_cursor(); }
static void move_cursor_down(void)  { if (terminal.row < VGA_HEIGHT - 1) terminal.row++; update_cursor(); }
static void move_cursor_left(void)  { if (terminal.column > 0) terminal.column--; update_cursor(); }
static void move_cursor_right(void) { if (terminal.column < VGA_WIDTH - 1) terminal.column++; update_cursor(); }

// Initialize the key handler table
void init_key_handlers(void) {
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
    } else if (scancode == SC_CTRL) {
        ctrl_pressed = true;
    } else if (scancode == SC_CTRL_RELEASE) {
        ctrl_pressed = false;
    } else if (ctrl_pressed && scancode == SC_C) {
        signal_schedule(SIGINT);           /* Ctrl+C */
    } else if (alt_pressed && scancode >= SC_F1 && scancode <= SC_F9) { 
        switch_screen(scancode - SC_F1);
    } else if (scancode < MAX_SCANCODE) {
        if (key_handlers[scancode]) {
            key_handlers[scancode]();
        } else {
            char c = shift_pressed ? scancode_to_char_shifted[scancode] : scancode_to_char_normal[scancode];
            if (c) {
                //terminal_putchar(c); kfs-1
		shell_putchar(c);
            }
        }
    }
}

/* IRQ 1: only grab the scancode; the heavy work runs outside the IRQ. */
static void keyboard_irq(struct regs *r)
{
    (void)r;
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);   /* always read to ack */
    uint32_t next = (kbd_head + 1) % KBD_BUFFER_SIZE;

    if (next != kbd_tail) {                       /* drop if full */
        kbd_buffer[kbd_head] = scancode;
        kbd_head = next;
    }
}

void keyboard_init(void)
{
    init_key_handlers();
    while (inb(KEYBOARD_STATUS_PORT) & 1)         /* flush stale bytes */
        inb(KEYBOARD_DATA_PORT);
    register_interrupt_handler(IRQ(1), keyboard_irq);
    pic_unmask(1);
}

int keyboard_pending(void)
{
    return kbd_head != kbd_tail;
}

/* Handle every scancode received since the last call. */
void keyboard_process(void)
{
    while (keyboard_pending()) {
        uint8_t scancode = kbd_buffer[kbd_tail];
        kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
        handle_scancode(scancode);
    }
}
