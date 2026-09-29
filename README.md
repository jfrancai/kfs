# kfs-4
IDT - interrupt descriptor table

![alt text](<Screenshot 2026-09-03 at 16.52.46.png>)

If the system GRUB BIOS modules are unavailable, download and extract them
locally before building an ISO:

```sh
mkdir -p .local/grub
url=$(dnf repoquery --location grub2-pc-modules | tail -1)
curl -L "$url" -o .local/grub/grub2-pc-modules.rpm
rpm2cpio .local/grub/grub2-pc-modules.rpm | cpio -idm --quiet -D .local/grub
```

Run the kernel directly with QEMU:

```sh
make start (-iso?)
```


## KFS-4: interrupts

### Boot order (`src/kernel.c`)
`gdt_init` → `terminal_initialize` → `idt_init` (exceptions, PIC remap, `lidt`)
→ `signal_init` → `timer_init` (IRQ0, 100 Hz) → `keyboard_init` (IRQ1)
→ `syscall_init` (int 0x80) → `sti`. The main loop drains the keyboard
buffer, delivers pending signals, then sleeps with `sti; hlt`.

### How an interrupt travels
```
CPU ──IDT[n]──> isrN (isr_stubs.s)   push err_code(0) + n
            └─> isr_common           pusha, ds, load kernel segments
                └─> interrupt_dispatch(struct regs *)   (idt.c)
                     ├ 0x20-0x2F IRQ  -> registered handler, then PIC EOI
                     ├ handler registered -> call it (int3, int 0x80...)
                     └ other exception    -> panic_regs()
            <── popa, iret
```

| File | Role |
|---|---|
| `src/isr_stubs.s` | 49 entry stubs (0-47, 0x80), common save/restore, `cpu_halt_clean` |
| `src/idt.c` | 256-entry IDT, `idt_set_gate`, `register_interrupt_handler`, dispatcher, `idt` command |
| `src/pic.c` | 8259 remap (IRQ 0-15 → 0x20-0x2F), mask/unmask, EOI, spurious IRQ check |
| `src/timer.c` | PIT at 100 Hz, `timer_ticks`, `timer_sleep` |
| `src/keyboard.c` | IRQ1 pushes scancodes into a ring buffer; main loop processes them; Ctrl+C → SIGINT |
| `src/signal.c` | signal-callback API and scheduling |
| `src/panic.c` | `panic`, `panic_regs`, `system_halt` |
| `src/stack.c` | `stack_save` snapshot, call trace |
| `src/syscall.c` | bonus: `int 0x80` (write, ticks, kill) |

### Signals (`includes/signal.h`)
- `signal(sig, cb)` registers a callback (`SIG_DFL`, `SIG_IGN`; SIGKILL can't be caught)
- `signal_raise(sig)` delivers now; `signal_schedule(sig)` marks it pending;
  `signal_schedule_in(sig, ticks)` delivers after a delay (timer IRQ)
- Pending signals are delivered by `signal_process()` from the main loop, never
  inside an IRQ handler.
- Default actions: SIGTERM → graceful halt, SIGKILL/SIGSEGV/SIGFPE/... → panic,
  others → "ignored" message.

### Panic / halt
1. `cli`  2. `stack_save()` copies ESP..stack_top into a static snapshot
3. prints reason, exception, registers, saved stack and call trace
4. `cpu_halt_clean()` zeroes EAX..EBP and EFLAGS, resets ESP, `hlt` forever.

### Shell commands to try
`idt`, `uptime`, `int3`, `syscall`, `kill 10`, `alarm 2`, Ctrl+C,
`kill 15` (graceful), `kill 9`, `div0`, `gpf`, `panic`, `halt`.
