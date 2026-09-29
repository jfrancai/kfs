#include "panic.h"
#include "cpu.h"
#include "printk.h"
#include "stack.h"
#include "string.h"
#include "terminal.h"
#include "vga.h"

/* Capture the current CPU state for a panic that did not come from an ISR.
   Inlined so EBP/ESP belong to panic(), whose frame stays alive. */
static inline __attribute__((always_inline))
void capture_regs(struct regs *r, uint32_t eip)
{
    memset(r, 0, sizeof *r);
    __asm__ volatile (
        "movl %%eax, %0\n\t"
        "movl %%ebx, %1\n\t"
        "movl %%ecx, %2\n\t"
        "movl %%edx, %3\n\t"
        "movl %%esi, %4\n\t"
        "movl %%edi, %5\n\t"
        "movl %%ebp, %6\n\t"
        "movl %%esp, %7\n\t"
        : "=m"(r->eax), "=m"(r->ebx), "=m"(r->ecx), "=m"(r->edx),
          "=m"(r->esi), "=m"(r->edi), "=m"(r->ebp), "=m"(r->esp));
    __asm__ volatile ("pushfl; popl %0" : "=r"(r->eflags));
    __asm__ volatile ("movl %%cs, %0" : "=r"(r->cs));
    __asm__ volatile ("movl %%ds, %0" : "=r"(r->ds));
    r->eip = eip;
    r->int_no = 0xFFFFFFFF;         /* not from an interrupt */
}

static void print_regs(const struct regs *r, uint32_t esp)
{
    printk("EAX=%08x EBX=%08x ECX=%08x EDX=%08x\n",
           r->eax, r->ebx, r->ecx, r->edx);
    printk("ESI=%08x EDI=%08x EBP=%08x ESP=%08x\n",
           r->esi, r->edi, r->ebp, esp);
    printk("EIP=%08x CS=%04x DS=%04x EFLAGS=%08x\n",
           r->eip, r->cs, r->ds, r->eflags);
}

static void print_snapshot(const struct stack_snapshot *s, int rows)
{
    printk("stack saved: %u bytes from %08x\n", s->size, s->esp);
    for (uint32_t off = 0; off < s->size && rows-- > 0; off += 16) {
        printk("%08x:", s->esp + off);
        for (uint32_t w = off; w < off + 16 && w < s->size; w += 4) {
            uint32_t v;
            memcpy(&v, s->data + w, sizeof v);
            printk(" %08x", v);
        }
        printk("\n");
    }
}

void panic_regs(const char *msg, const struct regs *r)
{
    __asm__ volatile ("cli");

    /* ESP before the interrupt: right after what the CPU pushed. */
    uint32_t esp = r->int_no == 0xFFFFFFFF ? r->esp
                                           : (uint32_t)&r->useresp;

    /* 1. Save the stack before anything else can change it. */
    const struct stack_snapshot *snap = stack_save(esp, r->ebp);

    /* 2. Report. */
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    terminal_clear();
    printk("*** KERNEL PANIC: %s ***\n", msg);
    if (r->int_no < 32)
        printk("exception 0x%02x (%s) error code %08x\n",
               r->int_no, exception_name(r->int_no), r->err_code);
    print_regs(r, esp);
    print_snapshot(snap, 4);
    printk("call trace:\n");
    print_stack_trace(r->ebp, esp, 6);
    printk("registers cleaned, CPU halted.");

    /* 3. Clean registers and halt for good. */
    cpu_halt_clean();
}

void panic(const char *msg)
{
    struct regs r;

    capture_regs(&r, (uint32_t)__builtin_return_address(0));
    panic_regs(msg, &r);
}

void system_halt(const char *reason)
{
    uint32_t esp, ebp;

    __asm__ volatile ("cli");
    __asm__ volatile ("movl %%esp, %0; movl %%ebp, %1" : "=r"(esp), "=r"(ebp));
    const struct stack_snapshot *snap = stack_save(esp, ebp);

    printk("%s\n", reason);
    printk("stack saved (%u bytes at %08x), registers cleaned. bye.\n",
           snap->size, snap->esp);
    cpu_halt_clean();
}
