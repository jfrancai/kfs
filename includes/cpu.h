#ifndef CPU_H
#define CPU_H

#include "stdint.h"

#define EFLAGS_IF 0x200

/* Disable interrupts and return the previous EFLAGS. */
static inline uint32_t irq_save(void)
{
    uint32_t flags;
    __asm__ volatile ("pushfl; popl %0; cli" : "=r"(flags) : : "memory");
    return flags;
}

/* Re-enable interrupts only if they were enabled before irq_save(). */
static inline void irq_restore(uint32_t flags)
{
    if (flags & EFLAGS_IF)
        __asm__ volatile ("sti" : : : "memory");
}

/* Zero all registers, reset the stack and halt forever (isr_stubs.s). */
void cpu_halt_clean(void) __attribute__((noreturn));

#endif
