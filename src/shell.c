#include "shell.h"
#include "printk.h"
#include "terminal.h"
#include "string.h"
#include "stack.h"
#include "ports.h"

#define SHELL_BUFSIZE 256

static char line[SHELL_BUFSIZE];
static size_t len = 0;

static void prompt(void)
{
    printk("kfs> ");
}

static void cmd_help(void)
{
    printk("available commands:\n");
    printk("  help   - show this list\n");
    printk("  stack  - dump the kernel stack\n");
    printk("  clear  - clear the screen\n");
    printk("  reboot - restart the machine\n");
    printk("  halt   - stop the CPU\n");
}

static void cmd_reboot(void)
{
    uint8_t status = 0x02;
    while (status & 0x02)          /* wait input buffer of 8042 empty */
        status = inb(0x64);
    outb(0x64, 0xFE);              /* signal reset CPU */
    __asm__ volatile ("hlt");
}

static void cmd_halt(void)
{
    printk("halted. bye.\n");
    __asm__ volatile ("cli; hlt");  /* turn off then stop CPU */
}

static void run_command(const char *cmd)
{
    if (cmd[0] == '\0')              return;   /* ignore empty line */
    else if (!strcmp(cmd, "help"))   cmd_help();
    else if (!strcmp(cmd, "stack"))  print_kernel_stack(256);
    else if (!strcmp(cmd, "clear"))  terminal_clear();
    else if (!strcmp(cmd, "reboot")) cmd_reboot();
    else if (!strcmp(cmd, "halt"))   cmd_halt();
    else printk("unknown command: %s\n", cmd);
}

void shell_init(void)
{
    len = 0;
    prompt();
}

/* receive input chars, stock in buffer, Enter -> execute */
void shell_putchar(char c)
{
    if (c == '\n') {
        line[len] = '\0';
        terminal_putchar('\n');
        run_command(line);
        len = 0;
        prompt();
    } else if (c == '\b') {
        if (len > 0) {
            len--;
            terminal_putchar('\b');   /* clear on screen + buffer */
        }
    } else if (len < SHELL_BUFSIZE - 1) {
        line[len++] = c;
        terminal_putchar(c);          /* echo char on terminal */
    }
}
