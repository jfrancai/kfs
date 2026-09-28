#include "gdt.h"
#include "printk.h"

/* Point directly to 0x800. Paging is off, so virtual = physical address. */

volatile struct gdt_entry *gdt = (struct gdt_entry *)GDT_ADDRESS;

/* Six-byte value loaded into GDTR. */
static struct gdt_ptr gp;

/* Fill one table entry by splitting base, limit, and access into 8 bytes. */
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

/* Load the table and make the segment registers use it. */
static inline void gdt_flush(void)
{
    __asm__ volatile (
        "lgdt %0             \n\t"   /* Load gp into GDTR. */
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

    gdt_set_gate(0, 0, 0x00000, 0x00, 0x00);  /* 0x00  null (required)     */
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF);  /* 0x08  kernel code       */
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF);  /* 0x10  kernel data       */
    gdt_set_gate(3, 0, 0xFFFFF, 0x92, 0xCF);  /* 0x18  kernel stack      */
    gdt_set_gate(4, 0, 0xFFFFF, 0xFA, 0xCF);  /* 0x20  user code         */
    gdt_set_gate(5, 0, 0xFFFFF, 0xF2, 0xCF);  /* 0x28  user data         */
    gdt_set_gate(6, 0, 0xFFFFF, 0xF2, 0xCF);  /* 0x30  user stack        */

    gdt_flush();
}

void gdt_print(void)
{
    struct gdt_ptr r;
    uint16_t cs, ds, ss;

    __asm__ volatile ("sgdt %0" : "=m"(r));
    __asm__ volatile ("mov %%cs,%0; mov %%ds,%1; mov %%ss,%2"
                      : "=r"(cs), "=r"(ds), "=r"(ss));
    printk("GDTR base=%08x limit=%04x  CS=%04x DS=%04x SS=%04x\n",
           r.base, r.limit, cs, ds, ss);

    const struct gdt_entry *entries = (const struct gdt_entry *)r.base;
    for (uint32_t i = 0; i * 8 < (uint32_t)r.limit + 1; i++) {
        uint32_t base = entries[i].base_low |
                        ((uint32_t)entries[i].base_middle << 16) |
                        ((uint32_t)entries[i].base_high << 24);
        uint32_t limit = entries[i].limit_low |
                         ((uint32_t)(entries[i].granularity & 0x0F) << 16);
        if (entries[i].granularity & 0x80)
            limit = (limit << 12) | 0xFFF;
        printk("%02x base=%08x lim=%08x acc=%02x DPL%u %s\n", i * 8,
               base, limit, entries[i].access,
               (entries[i].access >> 5) & 3,
               !(entries[i].access & 0x80) ? "null" :
               (entries[i].access & 0x08) ? "code" : "data");
    }
}
