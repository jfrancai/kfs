#include "terminal.h"
#include "keyboard.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif


void kernel_main(void) 
{
  terminal_initialize();
  terminal_writestring("Welcome - 42\n");

  update_cursor();
  init_key_handlers();

  while (1)
  {
    poll_keyboard();
  }
}
