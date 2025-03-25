#include "vga.h"
#include "terminal.h"
#include "ports.h"
#include "string.h"

#define VIDEO_MEMORY (char*)0xB8000

#define NUM_SCREENS 9

static size_t cursor_x = 0, cursor_y = 0;

static uint16_t screen_buffers[NUM_SCREENS][2000];
static size_t screen_rows[NUM_SCREENS] = {0}; // Initialize to zero
static size_t screen_columns[NUM_SCREENS] = {0}; // Initialize to zero
static uint8_t current_screen = 0;

// Define the global terminal variable
Terminal terminal = {
    .row = 0,
    .column = 0,
    .color = 0,  // Default color, can be updated later
    .buffer = (uint16_t*)0xB8000  // Default VGA memory address
};

void terminal_setcolor(uint8_t color) 
{
    terminal.color = color;
}

static void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
    const size_t index = y * VGA_WIDTH + x;
    terminal.buffer[index] = vga_entry(c, color);
}

static uint16_t terminal_getentryat(size_t x, size_t y)
{
    const size_t index = y * VGA_WIDTH + x;
    return terminal.buffer[index];
}

void terminal_putchar(char c) 
{
    if (c == '\n') { // Handle Enter key
        terminal.column = 0;
        terminal.row++;
    } 
    else if (c == '\b' && terminal.column > 0) { // Handle Backspace
        terminal.column--; // Move cursor left
        terminal_putentryat(' ', terminal.color, terminal.column, terminal.row);
        if (terminal.column == 0) {
            terminal.row--; 
            size_t last_col = VGA_WIDTH - 1; // Move to end of previous line
            while (last_col > 0 && (terminal_getentryat(last_col, terminal.row) & 0xFF) == ' ') {
                last_col--;
            }
            terminal.column = last_col + 1;
        }
    } 
    else { // Print normal character
        terminal_putentryat(c, terminal.color, terminal.column, terminal.row);
        terminal.column++;
    }

    if (terminal.column >= VGA_WIDTH) { // Move to next line if needed
        terminal.column = 0;
        terminal.row++;
    }

    if (terminal.row >= VGA_HEIGHT) { // Scroll screen if needed
        for (size_t y = 1; y < VGA_HEIGHT; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                uint16_t entry = terminal_getentryat(x, y); // Get previous row
                terminal_putentryat((char)entry, terminal.color, x, y - 1); // Move up
            }
        }

        for (size_t x = 0; x < VGA_WIDTH; x++) { // Clear last row
            terminal_putentryat(' ', terminal.color, x, VGA_HEIGHT - 1);
        }

        terminal.row = VGA_HEIGHT - 1;
    }

    // **Sync cursor with terminal position**
    cursor_x = terminal.column;
    cursor_y = terminal.row;

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

    screen_rows[current_screen] = terminal.row;
    screen_columns[current_screen] = terminal.column;
    memcpy(screen_buffers[current_screen], terminal.buffer, 2000 * sizeof(uint16_t));

    current_screen = screen;
    terminal.row = screen_rows[current_screen];
    terminal.column = screen_columns[current_screen];
    memcpy(terminal.buffer, screen_buffers[current_screen], 2000 * sizeof(uint16_t));

    cursor_x = terminal.column;
    cursor_y = terminal.row;
    update_cursor();
}

void terminal_initialize(void) {
  for (size_t i = 0; i < NUM_SCREENS; i++) {
    screen_rows[i] = 0;
    screen_columns[i] = 0;
    uint8_t color = vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
      for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = y * VGA_WIDTH + x;
        screen_buffers[i][index] = vga_entry(' ', color);
      }
    }
  }

  current_screen = 0;
  terminal.buffer = screen_buffers[current_screen];

  terminal.row = 0;
  terminal.column = 0;
  terminal.color = vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);

  terminal.buffer = (uint16_t*) 0xB8000;
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      terminal.buffer[index] = vga_entry(' ', terminal.color);
    }
  }
}

void update_cursor() {
    size_t pos = terminal.row * VGA_WIDTH + terminal.column;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void move_cursor(size_t x, size_t y) {
    cursor_x = x;
    cursor_y = y;
    update_cursor();
}
