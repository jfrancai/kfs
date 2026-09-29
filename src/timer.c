#include "timer.h"
#include "idt.h"
#include "pic.h"
#include "ports.h"
#include "signal.h"

#define PIT_FREQUENCY 1193182u
#define PIT_CHANNEL0  0x40
#define PIT_COMMAND   0x43

static volatile uint32_t ticks;

/* IRQ 0: count time and expire signals scheduled with a delay. */
static void timer_callback(struct regs *r)
{
    (void)r;
    ticks++;
    signal_tick();
}

void timer_init(uint32_t hz)
{
    uint32_t divisor = PIT_FREQUENCY / hz;

    register_interrupt_handler(IRQ(0), timer_callback);
    outb(PIT_COMMAND, 0x36);        /* channel 0, lo/hi byte, square wave */
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
    pic_unmask(0);
}

uint32_t timer_ticks(void)
{
    return ticks;
}

/* Sleep without burning the CPU: hlt wakes up on the next interrupt. */
void timer_sleep(uint32_t duration)
{
    uint32_t end = ticks + duration;

    while ((int32_t)(end - ticks) > 0)
        __asm__ volatile ("hlt");
}
