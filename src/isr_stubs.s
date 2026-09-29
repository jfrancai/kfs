/*
Low-level interrupt entry points.

The CPU jumps to one of these stubs through the IDT. Each stub pushes the
vector number (and a dummy error code when the CPU does not push one), so
every interrupt reaches isr_common with the same stack layout. isr_common
saves the registers, builds a `struct regs` on the stack and hands a
pointer to it to interrupt_dispatch() in idt.c.

Stack layout seen by interrupt_dispatch (low -> high address):
    ds | edi esi ebp esp ebx edx ecx eax | int_no err_code | eip cs eflags
*/

.section .text

/* Exceptions without an error code: push a fake one to keep the layout. */
.macro ISR_NOERR num
.global isr\num
.type isr\num, @function
isr\num:
	pushl $0
	pushl $\num
	jmp isr_common
.endm

/* Exceptions where the CPU already pushed an error code. */
.macro ISR_ERR num
.global isr\num
.type isr\num, @function
isr\num:
	pushl $\num
	jmp isr_common
.endm

/* CPU exceptions 0x00-0x1F */
.irp n, 0,1,2,3,4,5,6,7,9,15,16,18,19,20,22,23,24,25,26,27,28,31
	ISR_NOERR \n
.endr
.irp n, 8,10,11,12,13,14,17,21,29,30
	ISR_ERR \n
.endr

/* Hardware IRQs 0-15, remapped by the PIC to vectors 0x20-0x2F */
.irp n, 32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47
	ISR_NOERR \n
.endr

/* Software interrupt used for syscalls */
ISR_NOERR 128

.global isr_common
.type isr_common, @function
isr_common:
	pusha                   /* save general purpose registers */
	xorl %eax, %eax
	mov %ds, %ax
	pushl %eax              /* save the data segment */

	mov $0x10, %ax          /* kernel data selector */
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs

	pushl %esp              /* argument: struct regs * */
	cld                     /* C code expects DF = 0 */
	call interrupt_dispatch
	addl $4, %esp

	popl %eax               /* restore the data segment */
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	popa                    /* restore registers (eax may hold a syscall result) */
	addl $8, %esp           /* drop int_no and err_code */
	iret
.size isr_common, . - isr_common

/*
Last step of a panic or halt: disable interrupts, reset the stack and
zero every general purpose register and EFLAGS, so nothing is left
behind, then halt the CPU forever.
*/
.global cpu_halt_clean
.type cpu_halt_clean, @function
cpu_halt_clean:
	cli
	mov $stack_top, %esp
	pushl $0
	popfl                   /* EFLAGS = 0 (IF stays cleared) */
	xorl %eax, %eax
	xorl %ebx, %ebx
	xorl %ecx, %ecx
	xorl %edx, %edx
	xorl %esi, %esi
	xorl %edi, %edi
	xorl %ebp, %ebp
1:	hlt
	jmp 1b
.size cpu_halt_clean, . - cpu_halt_clean

/* Addresses of stubs 0-47, used by idt_init() to fill the table. */
.section .rodata
.global isr_stub_table
isr_stub_table:
.irp n, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47
	.long isr\n
.endr
