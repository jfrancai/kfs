#include "printk.h"
#include "stack.h"

void print_kernel_stack(uint32_t nbytes)
{
    uint32_t esp;
    __asm__ volatile ("mov %%esp, %0" : "=r"(esp));  /* take top of current stack */

    uint8_t *base = (uint8_t *)esp;

    for (uint32_t off = 0; off < nbytes; off += 16) {
        printk("%p: ", (void *)(base + off));        /* address colum */

        for (uint32_t j = 0; j < 16; j++) {          /* colum 16 byte hex */
            uint8_t b = base[off + j];
            if (b < 0x10)
                printk("0");                         /* padding to always have 2 digits */
            printk("%x ", b);
        }

        printk(" ");
        for (uint32_t j = 0; j < 16; j++) {          /* colum ASCII */
            uint8_t b = base[off + j];
            printk("%c", (b >= 32 && b < 127) ? (char)b : '.');
        }
        printk("\n");
    }
}
