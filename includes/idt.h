#ifndef IDT_H
#define IDT_H

#include "stdint.h"

#define IDT_ENTRIES 256
#define IRQ_BASE    0x20        /* IRQ 0 is remapped to vector 0x20 */
#define IRQ(n)      (IRQ_BASE + (n))
#define SYSCALL_VECTOR 0x80

/* Gate type/attribute bytes */
#define IDT_GATE_KERNEL 0x8E    /* present, DPL0, 32-bit interrupt gate */
#define IDT_GATE_USER   0xEE    /* present, DPL3, 32-bit interrupt gate */

/* One IDT entry is 8 bytes: handler address split around selector/flags. */
struct idt_entry {
    uint16_t offset_low;   /* handler bit 0..15 */
    uint16_t selector;     /* code segment selector (0x08) */
    uint8_t  zero;
    uint8_t  type_attr;    /* P, DPL, gate type */
    uint16_t offset_high;  /* handler bit 16..31 */
} __attribute__((packed));

/* Six bytes passed to lidt. */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* CPU state pushed by the stubs in isr_stubs.s, in stack order. */
struct regs {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;   /* pusha */
    uint32_t int_no, err_code;                          /* pushed by stub */
    uint32_t eip, cs, eflags;                           /* pushed by CPU */
    uint32_t useresp, ss;               /* only on a privilege change */
};

typedef void (*isr_t)(struct regs *r);

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t handler, uint16_t selector,
                  uint8_t type_attr);
void register_interrupt_handler(uint8_t num, isr_t handler);
void interrupt_dispatch(struct regs *r);
const char *exception_name(uint32_t num);
void idt_print(void);

#endif
