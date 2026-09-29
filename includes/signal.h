#ifndef SIGNAL_H
#define SIGNAL_H

#include "stdint.h"

#define NSIG 32

enum {
    SIGHUP  = 1,
    SIGINT  = 2,
    SIGQUIT = 3,
    SIGILL  = 4,
    SIGTRAP = 5,
    SIGABRT = 6,
    SIGBUS  = 7,
    SIGFPE  = 8,
    SIGKILL = 9,
    SIGUSR1 = 10,
    SIGSEGV = 11,
    SIGUSR2 = 12,
    SIGALRM = 14,
    SIGTERM = 15,
};

typedef void (*sighandler_t)(int sig);

#define SIG_DFL ((sighandler_t)0)       /* default action */
#define SIG_IGN ((sighandler_t)1)       /* ignore the signal */
#define SIG_ERR ((sighandler_t)-1)      /* returned on error */

void signal_init(void);

/* Register a callback for sig; returns the previous one, or SIG_ERR. */
sighandler_t signal(int sig, sighandler_t handler);

/* Deliver sig right now, in the caller's context. */
int signal_raise(int sig);

/* Mark sig pending; it is delivered by the next signal_process(). */
int signal_schedule(int sig);

/* Deliver sig after `ticks` timer ticks (TIMER_HZ per second). */
int signal_schedule_in(int sig, uint32_t ticks);

/* Deliver every pending signal. Called from the kernel main loop. */
void signal_process(void);
int  signal_pending(void);

/* Called by the timer IRQ on every tick to expire delayed signals. */
void signal_tick(void);

const char *signal_name(int sig);

#endif
