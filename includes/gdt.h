#ifndef GDT_H
#define GDT_H

#include <stdint.h>

#define GDT_ADDRESS 0x00000800   /* obligated by the suject */
#define GDT_ENTRIES 7            /* null + 6 segment */

/* One line GDT = exactly 8 byte, layout is cut by historical reason */
/* base low middle high: before when CPU 286 there is only base 24 bit, when CPU 386, another 8 bit is added at the end to avoid destroying the old layout */
struct gdt_entry {
    uint16_t limit_low;    /* limit  bit 0..15  */
    uint16_t base_low;     /* base   bit 0..15  */
    uint8_t  base_middle;  /* base   bit 16..23 */
    uint8_t  access;       /* label right (P, DPL, code/data, r/w) */
    uint8_t  granularity;  /* 4 bit flags + limit bit 16..19 */
    uint8_t  base_high;    /* base   bit 24..31 */
} __attribute__((packed));

/* __attribute__((packed)): very important, interdit compiler to add byte padding -> avoid looop reboot */
/* 6 byte to give to lgdt: "how long is the table, and its adresse" */
struct gdt_ptr {
    uint16_t limit;   /* table's total length - 1 */
    uint32_t base;    /* table's starting addresse = 0x800 */
} __attribute__((packed));

void gdt_init(void);

#endif
