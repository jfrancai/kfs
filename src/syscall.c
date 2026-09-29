#include "syscall.h"
#include "idt.h"
#include "signal.h"
#include "terminal.h"
#include "timer.h"

/*
int 0x80 entry point. There are no processes yet, so these only run from
the kernel, but the gate is DPL3 so user mode can use it later.
Number in eax, arguments in ebx/ecx/edx, result returned in eax.
*/
static void syscall_handler(struct regs *r)
{
    switch (r->eax) {
    case SYS_WRITE:
        terminal_write((const char *)r->ebx, r->ecx);
        r->eax = r->ecx;
        break;
    case SYS_TICKS:
        r->eax = timer_ticks();
        break;
    case SYS_KILL:
        r->eax = (uint32_t)signal_schedule((int)r->ebx);
        break;
    default:
        r->eax = (uint32_t)-1;
    }
}

void syscall_init(void)
{
    register_interrupt_handler(SYSCALL_VECTOR, syscall_handler);
}
