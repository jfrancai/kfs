#include <stdarg.h>
#include <stddef.h>
#include "printk.h"
#include "terminal.h"

/* Printing a chain of int, return number of int printed */
static int print_str(const char *s)
{
    int n = 0;
    while (*s) { terminal_putchar(*s++); n++; }
    return n;
}

/* printing numbers in base (10 or 16) */
static int print_uint(uint32_t value, uint32_t base, int upper,
                      int width, char pad)
{
    char buf[32];
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    int i = 0;

    if (value == 0)
        buf[i++] = '0';
    while (value) {
        buf[i++] = digits[value % base];   /* taking the last number */
        value /= base;
    }

    int n = i;
    while (n < width) {
        terminal_putchar(pad);
        n++;
    }
    while (i--)                            /* printing REVERSE */
        terminal_putchar(buf[i]);
    return n;
}

/* In số CÓ dấu (hệ 10) */
static int print_int(int32_t value)
{
    if (value < 0) {
        terminal_putchar('-');
        return 1 + print_uint((uint32_t)0 - (uint32_t)value, 10, 0, 0, ' ');
    }
    return print_uint((uint32_t)value, 10, 0, 0, ' ');
}

int printk(const char *fmt, ...)
{
    va_list ap;
    int count = 0;

    va_start(ap, fmt);
    for (size_t i = 0; fmt[i]; i++) {
        if (fmt[i] != '%') {                       /* normal char */
            terminal_putchar(fmt[i]);
            count++;
            continue;
        }
        i++;
        if (!fmt[i])
            break;

        char pad = ' ';
        int width = 0;
        if (fmt[i] == '0') {
            pad = '0';
            i++;
        }
        while (fmt[i] >= '0' && fmt[i] <= '9') {
            width = width * 10 + (fmt[i] - '0');
            i++;
        }

        switch (fmt[i]) {
            case 'c': terminal_putchar((char)va_arg(ap, int)); count++; break;
            case 's': count += print_str(va_arg(ap, const char *)); break;
            case 'd':
            case 'i': count += print_int(va_arg(ap, int)); break;
            case 'u': count += print_uint(va_arg(ap, unsigned int), 10, 0, width, pad); break;
            case 'x': count += print_uint(va_arg(ap, unsigned int), 16, 0, width, pad); break;
            case 'X': count += print_uint(va_arg(ap, unsigned int), 16, 1, width, pad); break;
            case 'p': count += print_str("0x");
                      count += print_uint((uint32_t)va_arg(ap, void *), 16, 0, width, pad); break;
            case '%': terminal_putchar('%'); count++; break;
            default:  terminal_putchar('%'); terminal_putchar(fmt[i]); count += 2; break;
        }
    }
    va_end(ap);
    return count;
}
