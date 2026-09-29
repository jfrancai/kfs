#ifndef SYSCALL_H
#define SYSCALL_H

/* Syscall numbers, passed in eax to int 0x80. Arguments in ebx, ecx, edx. */
enum {
    SYS_WRITE = 1,     /* write(buf, len)  -> bytes written */
    SYS_TICKS = 2,     /* ticks()          -> timer ticks since boot */
    SYS_KILL  = 3,     /* kill(sig)        -> 0 or -1 */
};

void syscall_init(void);

#endif
