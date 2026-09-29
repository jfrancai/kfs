#include "pic.h"
#include "ports.h"

/* 8259 Programmable Interrupt Controller: master + slave on IRQ 2. */
#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

#define PIC_EOI      0x20
#define PIC_READ_ISR 0x0B
#define ICW1_INIT    0x11   /* init + ICW4 needed */
#define ICW4_8086    0x01

/* Give the old PIC time to settle between commands. */
static void io_wait(void)
{
    outb(0x80, 0);
}

/*
By default IRQ 0-7 are delivered on vectors 0x08-0x0F, the same vectors the
CPU uses for exceptions (0x08 = double fault!). Move them to our offsets.
*/
void pic_remap(uint8_t master_offset, uint8_t slave_offset)
{
    outb(PIC1_CMD, ICW1_INIT);  io_wait();
    outb(PIC2_CMD, ICW1_INIT);  io_wait();
    outb(PIC1_DATA, master_offset); io_wait();   /* ICW2: vector base */
    outb(PIC2_DATA, slave_offset);  io_wait();
    outb(PIC1_DATA, 0x04); io_wait();            /* ICW3: slave on IRQ2 */
    outb(PIC2_DATA, 0x02); io_wait();            /* ICW3: cascade id */
    outb(PIC1_DATA, ICW4_8086); io_wait();
    outb(PIC2_DATA, ICW4_8086); io_wait();

    /* Mask everything except the cascade; drivers unmask what they use. */
    outb(PIC1_DATA, 0xFB);
    outb(PIC2_DATA, 0xFF);
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
        outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}

void pic_mask(uint8_t irq)
{
    uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    outb(port, (uint8_t)(inb(port) | (1u << (irq & 7))));
}

void pic_unmask(uint8_t irq)
{
    uint16_t port = irq < 8 ? PIC1_DATA : PIC2_DATA;
    outb(port, (uint8_t)(inb(port) & ~(1u << (irq & 7))));
}

/* IRQ 7/15 can fire without a real request; those must not get an EOI. */
int pic_is_spurious(uint8_t irq)
{
    if (irq == 7) {
        outb(PIC1_CMD, PIC_READ_ISR);
        return !(inb(PIC1_CMD) & 0x80);
    }
    if (irq == 15) {
        outb(PIC2_CMD, PIC_READ_ISR);
        if (!(inb(PIC2_CMD) & 0x80)) {
            outb(PIC1_CMD, PIC_EOI);     /* master did see the cascade */
            return 1;
        }
    }
    return 0;
}
