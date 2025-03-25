TARGET=i386-elf
CC=$(TARGET)-gcc

PROJDIRS := src includes tests

SRCFILES := $(shell find $(PROJDIRS) -type f -name "*.c")
HDRFILES := $(shell find $(PROJDIRS) -type f -name "*.h")

OBJFILES := $(patsubst src/%,%, $(patsubst %.c,%.o, $(SRCFILES)))
TSTFILES := $(patsubst %.c,%_t,$(SRCFILES))

DEPFILES    := $(patsubst %.c,%.d,$(SRCFILES))
TSTDEPFILES := $(patsubst %,%.d,$(TSTFILES))

ALLFILES := $(SRCFILES) $(HDRFILES) $(AUXFILES)

WARNINGS := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
            -Wwrite-strings -Wmissing-prototypes -Wmissing-declarations \
            -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
            -Wconversion -Wstrict-prototypes

CFLAGS := -I ./includes/ -g -ffreestanding -O2 -std=gnu99 $(WARNINGS)

%.o: src/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

boot.o: boot.s
	$(TARGET)-as ./boot.s -o boot.o

# interrupts.o: interrupts.s
# 	nasm -f elf32 interrupts.s -o interrupts.o

myos.bin: boot.o $(OBJFILES)
	$(CC) -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o $(OBJFILES) -lgcc

myos.iso: myos.bin grub.cfg
	mkdir -p isodir/boot/grub
	cp myos.bin isodir/boot/myos.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

all: myos.bin

clean:
	-@$(RM) $(wildcard $(OBJFILES) $(DEPFILES) $(TSTFILES) pdclib.a pdclib.tgz)
	$(RM) boot.o 
	$(RM) $(OBJFILES)
	$(RM) myos.bin
	$(RM) myos.iso

re: clean all

start: myos.bin
	qemu-system-i386 -kernel myos.bin

start-iso: myos.iso
	qemu-system-i386 -cdrom myos.iso

todolist:
	-@for file in $(ALLFILES:Makefile=); do fgrep -H -e TODO -e FIXME $$file; done; true

-include $(DEPFILES)

.PHONY: boot.o all clean re start start-iso
