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
  update_cursor();
  init_key_handlers();

  //kfs-2:p1
  //printk("KFS_2 booted!\n");
  //printk("GDT base = %p, decimal = %d, hex = 0x%x\n", (void *)0x800, 2048, 2048);
  //print_kernel_stack(256);

  //kfs-2: bonus
  printk("KFS-2 ready. Type 'help'\n");
  shell_init();  


  while (1)
  {
    poll_keyboard();
  }
}
