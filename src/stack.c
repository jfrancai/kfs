#include "printk.h"
#include "stack.h"
#include "keyboard.h"
#include "shell.h"
#include "terminal.h"

#include <stddef.h>

extern uint8_t stack_bottom[];
extern uint8_t stack_top[];
extern void _start(void);
void kernel_main(void);

struct ksym {
    uint32_t addr;
    const char *name;
};

static const struct ksym ksyms[] = {
    { (uint32_t)_start, "_start" },
    { (uint32_t)kernel_main, "kernel_main" },
    { (uint32_t)poll_keyboard, "poll_keyboard" },
    { (uint32_t)handle_scancode, "handle_scancode" },
    { (uint32_t)shell_putchar, "shell_putchar" },
    { (uint32_t)print_kernel_stack, "print_kernel_stack" },
};

static const char *sym(uint32_t address, uint32_t *offset)
{
    const struct ksym *best = 0;

    for (size_t i = 0; i < sizeof ksyms / sizeof *ksyms; i++) {
        if (ksyms[i].addr <= address &&
            (!best || ksyms[i].addr > best->addr))
            best = &ksyms[i];
    }
    if (!best) {
        *offset = 0;
        return "??";
    }
    *offset = address - best->addr;
    return best->name;
}

struct stack_frame {
    struct stack_frame *ebp;
    uint32_t ret;
};

void print_kernel_stack(void)
{
    uint32_t esp;
    uint32_t ebp;
    uint32_t bottom = (uint32_t)stack_bottom;
    uint32_t top = (uint32_t)stack_top;

    __asm__ volatile ("mov %%esp, %0" : "=r"(esp));
    __asm__ volatile ("mov %%ebp, %0" : "=r"(ebp));
    terminal_clear();

    uint32_t used = esp > top ? 0 : top - esp;
    printk("stack [%08x-%08x] used %u/%u bytes\n",
           bottom, top, used, top - bottom);
    printk("ESP=%08x EBP=%08x\n", esp, ebp);

    if (esp < bottom)
        esp = bottom;
    if (esp > top)
        esp = top;
    for (uint32_t address = esp; address < top; address += 16) {
        printk("%08x:", address);
        for (uint32_t word = address; word < address + 16 && word < top;
             word += 4)
            printk(" %08x%c", *(uint32_t *)word, word == ebp ? '*' : ' ');
        printk("\n");
    }

    printk("call trace:\n");
    struct stack_frame *frame = (struct stack_frame *)ebp;
    for (int depth = 0; frame != 0 && (uint32_t)frame >= esp &&
         (uint32_t)frame + 8 <= top && depth < 16; depth++) {
         uint32_t offset;
        printk("  #%d %s+0x%x (ret %08x)\n", depth,
               sym(frame->ret, &offset), offset, frame->ret);
        frame = frame->ebp;
    }
}
