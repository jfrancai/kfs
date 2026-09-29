#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define GDT_ADDRESS 0x00000800   /* required by the subject */
#define GDT_ENTRIES 7            /* null + 6 segment */

/* One GDT entry is exactly 8 bytes; the layout is historical. */
/* The 386 added the high base byte without changing the older layout. */
struct gdt_entry {
    uint16_t limit_low;    /* limit  bit 0..15  */
    uint16_t base_low;     /* base   bit 0..15  */
    uint8_t  base_middle;  /* base   bit 16..23 */
    uint8_t  access;       /* label right (P, DPL, code/data, r/w) */
    uint8_t  granularity;  /* 4 bit flags + limit bit 16..19 */
    uint8_t  base_high;    /* base   bit 24..31 */
} __attribute__((packed));

/* packed prevents compiler padding, which would make lgdt load bad data. */
/* Six bytes passed to lgdt: the table size and its starting address. */
struct gdt_ptr {
    uint16_t limit;   /* table's total length - 1 */
    uint32_t base;    /* table's starting address = 0x800 */
} __attribute__((packed));

void gdt_init(void);
void gdt_print(void);

#endif
