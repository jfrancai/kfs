#include "terminal.h"
#include "keyboard.h"
#include "gdt.h"
#include "idt.h"
#include "printk.h"
#include "signal.h"
#include "syscall.h"
#include "timer.h"
#include "shell.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

void kernel_main(void);

void kernel_main(void) 
{
  gdt_init();
  terminal_initialize();
  idt_init();            /* exceptions + remapped PIC, all IRQs masked */
  signal_init();
  timer_init(TIMER_HZ);  /* IRQ 0 */
  keyboard_init();       /* IRQ 1 */
  syscall_init();        /* int 0x80 */
  update_cursor();

  printk("KFS-4 ready. Type 'help'\n");
  shell_init();
  __asm__ volatile ("sti");

  while (1)
  {
    keyboard_process();
    signal_process();

    /* Sleep until the next interrupt, unless work arrived meanwhile.
       "sti; hlt" is atomic, so no wake-up can be lost in between. */
    __asm__ volatile ("cli");
    if (keyboard_pending() || signal_pending())
      __asm__ volatile ("sti");
    else
      __asm__ volatile ("sti; hlt");
  }
}
