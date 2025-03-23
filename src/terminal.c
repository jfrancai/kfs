#include "terminal.h"
#include "string.h"

#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_DATA_PORT    0x60

#define VIDEO_MEMORY (char*)0xB8000

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


/* Hardware text mode color constants. */
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
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;

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
        terminal_column--;
        terminal_putentryat(' ', terminal_color, terminal_column, terminal_row); // Clear char
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

void terminal_initialize(void) 
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_GREEN , VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xB8000;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_writehex(uint8_t num)
{
    // Hexadecimal characters
    const char hex_digits[] = "0123456789ABCDEF";

    // Get the high nibble (4 bits) of the number
    char high_nibble = hex_digits[(num >> 4) & 0x0F]; // Get the higher nibble (4 bits)
    
    // Get the low nibble (4 bits) of the number
    char low_nibble = hex_digits[num & 0x0F]; // Get the lower nibble (4 bits)

    // Print the high nibble (MSB)
    terminal_putchar(high_nibble);

    // Print the low nibble (LSB)
    terminal_putchar(low_nibble);
}

void handle_scancode(uint8_t scancode)
{
	if (scancode < 128) {
		char c = scancode_to_char[scancode];
		terminal_putchar(c);
	}
	/* terminal_writehex(scancode); */
}

void poll_keyboard()
{
	if (inb(KEYBOARD_STATUS_PORT) & 1) 
	{
		uint8_t scancode = inb(KEYBOARD_DATA_PORT);
		handle_scancode(scancode);
	}
}
