#include "terminal.h"
#include "keyboard.h"
#include "gdt.h"
#include "printk.h"
#include "stack.h"
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
  gdt_print();
  update_cursor();
  init_key_handlers();

  printk("KFS-2 ready. Type 'help'\n");
  shell_init();  


  while (1)
  {
    poll_keyboard();
  }
}
