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
    printk("  echo   - print text back\n");
    printk("  duckie - dancing duck :)\n");
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

/* pause 1 round, empty looop, volatile so the compiler doesn't avoid */
static void delay(void)
{
    for (volatile uint32_t i = 0; i < 30000000; i++)
        ;
}

static void cmd_duckie(void)
{
    const char *frame_a =
        "\n"
        "     _\n"
        "   >(o )\n"
        "    (  )>\n"
        "    ~~~~\n";
    const char *frame_b =
        "\n"
        "       _\n"
        "    ( o)<\n"
        "   <(  )\n"
        "    ~~~~\n";

    for (int i = 0; i < 8; i++) {
        terminal_clear();
        printk("%s", (i % 2 == 0) ? frame_a : frame_b);
        delay();
    }
    terminal_clear();
    printk("quack quack! :)\n");
}

/* return 1 if 's' starting wiht 'prefix' */
static int starts_with(const char *s, const char *prefix)
{
    while (*prefix) {
        if (*s != *prefix)
            return 0;
        s++;
        prefix++;
    }
    return 1;
}

static void run_command(const char *cmd)
{
    if (cmd[0] == '\0')              return;   /* ignore empty line */
    else if (!strcmp(cmd, "help"))   cmd_help();
    else if (!strcmp(cmd, "stack"))  print_kernel_stack(256);
    else if (!strcmp(cmd, "clear"))  terminal_clear();
    else if (!strcmp(cmd, "reboot")) cmd_reboot();
    else if (!strcmp(cmd, "halt"))   cmd_halt();
    else if (!strcmp(cmd, "duckie")) cmd_duckie();
    else if (starts_with(cmd, "echo") && (cmd[4] == ' ' || cmd[4] == '\0')) {
        const char *arg = cmd + 4;         /* skip "echo" */
        while (*arg == ' ')                /* skip empty chars */
            arg++;
        printk("%s\n", arg);
    }
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
