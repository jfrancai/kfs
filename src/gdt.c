#include "gdt.h"

/* Pointing directly to 0x800.
   Paging is off so virtual address = physical address  */

volatile struct gdt_entry *gdt = (struct gdt_entry *)GDT_ADDRESS;

/* 6 byte pushing to GDTR */
static struct gdt_ptr gp;

/* assign 1 line of the table: put base/limit/access into the pieces of 8 byte */
static void gdt_set_gate(int num, uint32_t base, uint32_t limit,
                         uint8_t access, uint8_t gran)
{
    gdt[num].base_low    = (uint16_t)(base & 0xFFFF);
    gdt[num].base_middle = (uint8_t)((base >> 16) & 0xFF);
    gdt[num].base_high   = (uint8_t)((base >> 24) & 0xFF);

    gdt[num].limit_low   = (uint16_t)(limit & 0xFFFF);
    gdt[num].granularity = (uint8_t)(((limit >> 16) & 0x0F) | (gran & 0xF0));

    gdt[num].access      = access;
}
