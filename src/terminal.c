#include "terminal.h"
#include "string.h"

#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_DATA_PORT    0x60

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;


// Arrow key scancodes
#define SC_UP    0x48
#define SC_DOWN  0x50
#define SC_LEFT  0x4B
#define SC_RIGHT 0x4D

#define VIDEO_MEMORY (char*)0xB8000

// Use constant expressions to declare NUM_SCREENS and SCREEN_SIZE
#define NUM_SCREENS 5

static uint16_t screen_buffers[NUM_SCREENS][2000];
static size_t screen_rows[NUM_SCREENS] = {0}; // Initialize to zero
static size_t screen_columns[NUM_SCREENS] = {0}; // Initialize to zero
static uint8_t current_screen = 0;

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer;

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

static uint16_t cursor_x = 0, cursor_y = 0;

void move_cursor_up()    { if (terminal_row > 0) terminal_row--; update_cursor(); }
void move_cursor_down()  { if (terminal_row < VGA_HEIGHT - 1) terminal_row++; update_cursor(); }
void move_cursor_left()  { if (terminal_column > 0) terminal_column--; update_cursor(); }
void move_cursor_right() { if (terminal_column < VGA_WIDTH - 1) terminal_column++; update_cursor(); }

void (*key_handlers[128])(void) = { 0 };

// Initialize the key handler table
void init_key_handlers() {
    key_handlers[SC_UP] = move_cursor_up;
    key_handlers[SC_DOWN] = move_cursor_down;
    key_handlers[SC_LEFT] = move_cursor_left;
    key_handlers[SC_RIGHT] = move_cursor_right;
}

// Hardware text mode color constants.
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

void update_cursor() {
    uint16_t pos = terminal_row * VGA_WIDTH + terminal_column;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void move_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
    update_cursor();
}

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) 
{
    return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_setcolor(uint8_t color) 
{
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

uint16_t terminal_getentryat(size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    return terminal_buffer[index];
}

void terminal_putchar(char c) 
{
    if (c == '\n') { // Handle Enter key
        terminal_column = 0;
        terminal_row++;
    } 
    else if (c == '\b' && terminal_column > 0) { // Handle Backspace
        terminal_column--; // Move cursor left
        terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
        if (terminal_column == 0) {
            terminal_row--; 
            size_t last_col = VGA_WIDTH - 1; // Move to end of previous line
            while (last_col > 0 && (terminal_getentryat(last_col, terminal_row) & 0xFF) == ' ') {
                last_col--;
            }
            terminal_column = last_col + 1;
        }
    } 
    else { // Print normal character
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        terminal_column++;
    }

    if (terminal_column >= VGA_WIDTH) { // Move to next line if needed
        terminal_column = 0;
        terminal_row++;
    }

    if (terminal_row >= VGA_HEIGHT) { // Scroll screen if needed
        for (int y = 1; y < VGA_HEIGHT; y++) {
            for (int x = 0; x < VGA_WIDTH; x++) {
                uint16_t entry = terminal_getentryat(x, y); // Get previous row
                terminal_putentryat(entry, terminal_color, x, y - 1); // Move up
            }
        }

        for (int x = 0; x < VGA_WIDTH; x++) { // Clear last row
            terminal_putentryat(' ', terminal_color, x, VGA_HEIGHT - 1);
        }

        terminal_row = VGA_HEIGHT - 1;
    }

    // **Sync cursor with terminal position**
    cursor_x = terminal_column;
    cursor_y = terminal_row;

    update_cursor();
}

void terminal_write(const char* data, size_t size) 
{
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) 
{
    terminal_write(data, strlen(data));
}

void terminal_writehex(uint8_t num)
{
    const char hex_digits[] = "0123456789ABCDEF";

    char high_nibble = hex_digits[(num >> 4) & 0x0F]; 
    char low_nibble = hex_digits[num & 0x0F]; 

    terminal_putchar(high_nibble);
    terminal_putchar(low_nibble);
}

void switch_screen(uint8_t screen) {
    if (screen >= NUM_SCREENS) return;

    screen_rows[current_screen] = terminal_row;
    screen_columns[current_screen] = terminal_column;
    memcpy(screen_buffers[current_screen], terminal_buffer, 2000 * sizeof(uint16_t));

    current_screen = screen;
    terminal_row = screen_rows[current_screen];
    terminal_column = screen_columns[current_screen];
    memcpy(terminal_buffer, screen_buffers[current_screen], 2000 * sizeof(uint16_t));

    cursor_x = terminal_column;
    cursor_y = terminal_row;
    update_cursor();
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

void terminal_initialize(void) {
    for (size_t i = 0; i < NUM_SCREENS; i++) {
        screen_rows[i] = 0;
        screen_columns[i] = 0;
        memset(screen_buffers[i], ' ', 2000 * sizeof(uint16_t));
    }

    current_screen = 0;
    terminal_buffer = screen_buffers[current_screen];

    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);

	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

