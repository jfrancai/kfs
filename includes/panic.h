#ifndef PANIC_H
#define PANIC_H

#include "idt.h"

/* Fatal error from C code: save the stack, report, clean registers, halt. */
void panic(const char *msg) __attribute__((noreturn));

/* Same, with the CPU state captured by an interrupt stub. */
void panic_regs(const char *msg, const struct regs *r)
    __attribute__((noreturn));

/* Normal shutdown: save the stack, clean registers, halt. */
void system_halt(const char *reason) __attribute__((noreturn));

#endif
