#include "printk.h"
#include "stack.h"

extern uint8_t stack_bottom[];
extern uint8_t stack_top[];

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

    uint32_t used = esp > top ? 0 : top - esp;
    printk("stack [%08x-%08x] used %u/%u bytes\n",
           bottom, top, used, top - bottom);
    printk("ESP=%08x EBP=%08x\n", esp, ebp);

    if (esp < bottom)
        esp = bottom;
    if (esp > top)
        esp = top;
    for (uint32_t address = esp; address + 4 <= top; address += 4) {
        printk("  %08x: %08x%s\n", address, *(uint32_t *)address,
               address == ebp ? "  <- EBP" : "");
    }

    printk("call trace:\n");
    struct stack_frame *frame = (struct stack_frame *)ebp;
    for (int depth = 0; frame != 0 && (uint32_t)frame >= esp &&
         (uint32_t)frame + 8 <= top && depth < 16; depth++) {
        printk("  #%d ret=%08x  (frame %08x)\n", depth, frame->ret,
               (uint32_t)frame);
        frame = frame->ebp;
    }
}
