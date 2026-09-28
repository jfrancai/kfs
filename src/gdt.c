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

/* assign table to CPU -> force register segment use the new table */
static inline void gdt_flush(void)
{
    __asm__ volatile (
        "lgdt %0             \n\t"   /* 1. assign {limit,base} of gp into GDTR   */
        "mov $0x10, %%ax     \n\t"   /* 2. 0x10 = selector kernel data        */
        "mov %%ax, %%ds      \n\t"
        "mov %%ax, %%es      \n\t"
        "mov %%ax, %%fs      \n\t"
        "mov %%ax, %%gs      \n\t"
        "mov $0x18, %%ax     \n\t"   /* kernel stack segment */
        "mov %%ax, %%ss      \n\t"
        "ljmp $0x08, $1f     \n\t"   /* 3. 0x08 = kernel code → far jump reload CS */
        "1:                  \n\t"
        : : "m"(gp) : "ax", "memory"
    );
}

void gdt_init(void)
{
    gp.limit = (uint16_t)(sizeof(struct gdt_entry) * GDT_ENTRIES - 1);
    gp.base  = GDT_ADDRESS;

    gdt_set_gate(0, 0, 0x00000, 0x00, 0x00);  /* 0x00  null (obligated)   */
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF);  /* 0x08  kernel code       */
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF);  /* 0x10  kernel data       */
    gdt_set_gate(3, 0, 0xFFFFF, 0x92, 0xCF);  /* 0x18  kernel stack      */
    gdt_set_gate(4, 0, 0xFFFFF, 0xFA, 0xCF);  /* 0x20  user code         */
    gdt_set_gate(5, 0, 0xFFFFF, 0xF2, 0xCF);  /* 0x28  user data         */
    gdt_set_gate(6, 0, 0xFFFFF, 0xF2, 0xCF);  /* 0x30  user stack        */

    gdt_flush();
}
