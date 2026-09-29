#include "signal.h"
#include "cpu.h"
#include "panic.h"
#include "printk.h"
#include "string.h"

#define MAX_TIMED_SIGNALS 8

static sighandler_t handlers[NSIG];

/* Bit n set = signal n waiting for delivery. Written from IRQ context. */
static volatile uint32_t pending;

/* Signals scheduled for later; sig == 0 marks a free slot. */
static struct {
    int      sig;
    uint32_t remaining;
} timed[MAX_TIMED_SIGNALS];

static int valid(int sig)
{
    return sig > 0 && sig < NSIG;
}

const char *signal_name(int sig)
{
    switch (sig) {
    case SIGHUP:  return "SIGHUP";
    case SIGINT:  return "SIGINT";
    case SIGQUIT: return "SIGQUIT";
    case SIGILL:  return "SIGILL";
    case SIGTRAP: return "SIGTRAP";
    case SIGABRT: return "SIGABRT";
    case SIGBUS:  return "SIGBUS";
    case SIGFPE:  return "SIGFPE";
    case SIGKILL: return "SIGKILL";
    case SIGUSR1: return "SIGUSR1";
    case SIGSEGV: return "SIGSEGV";
    case SIGUSR2: return "SIGUSR2";
    case SIGALRM: return "SIGALRM";
    case SIGTERM: return "SIGTERM";
    default:      return "SIG?";
    }
}

void signal_init(void)
{
    memset(handlers, 0, sizeof handlers);
    memset(timed, 0, sizeof timed);
    pending = 0;
}

sighandler_t signal(int sig, sighandler_t handler)
{
    /* Like on Unix, SIGKILL can be neither caught nor ignored. */
    if (!valid(sig) || sig == SIGKILL)
        return SIG_ERR;

    uint32_t flags = irq_save();
    sighandler_t old = handlers[sig];
    handlers[sig] = handler;
    irq_restore(flags);
    return old;
}

/* What happens when nobody registered a callback. */
static void default_action(int sig)
{
    switch (sig) {
    case SIGTERM:
        /* The gentle way: tell the user, then shut down cleanly. */
        printk("\nSIGTERM: shutting down gracefully...\n");
        system_halt("terminated by SIGTERM");
    case SIGKILL:
        panic("killed by SIGKILL");
    case SIGILL:
    case SIGABRT:
    case SIGBUS:
    case SIGFPE:
    case SIGSEGV:
    case SIGQUIT:
        panic(signal_name(sig));
    default:
        printk("\n%s (%d): no handler, ignored\n", signal_name(sig), sig);
    }
}

int signal_raise(int sig)
{
    if (!valid(sig))
        return -1;

    sighandler_t h = handlers[sig];
    if (h == SIG_IGN)
        return 0;
    if (h == SIG_DFL)
        default_action(sig);
    else
        h(sig);
    return 0;
}

int signal_schedule(int sig)
{
    if (!valid(sig))
        return -1;

    uint32_t flags = irq_save();
    pending |= 1u << sig;
    irq_restore(flags);
    return 0;
}

int signal_schedule_in(int sig, uint32_t ticks)
{
    if (!valid(sig))
        return -1;
    if (ticks == 0)
        return signal_schedule(sig);

    int ret = -1;
    uint32_t flags = irq_save();
    for (int i = 0; i < MAX_TIMED_SIGNALS; i++) {
        if (timed[i].sig == 0) {
            timed[i].sig = sig;
            timed[i].remaining = ticks;
            ret = 0;
            break;
        }
    }
    irq_restore(flags);
    return ret;
}

/* Runs inside the timer IRQ (interrupts already off). */
void signal_tick(void)
{
    for (int i = 0; i < MAX_TIMED_SIGNALS; i++) {
        if (timed[i].sig && --timed[i].remaining == 0) {
            pending |= 1u << timed[i].sig;
            timed[i].sig = 0;
        }
    }
}

int signal_pending(void)
{
    return pending != 0;
}

/*
Handlers are not run inside the interrupt that scheduled them: they run
here, from the main loop, with interrupts enabled.
*/
void signal_process(void)
{
    for (;;) {
        uint32_t flags = irq_save();
        uint32_t set = pending;
        int sig = 0;

        if (set) {
            while (!(set & (1u << sig)))
                sig++;
            pending &= ~(1u << sig);
        }
        irq_restore(flags);

        if (!sig)
            return;
        signal_raise(sig);
    }
}
