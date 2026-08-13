TARGET=i686-elf
CC=$(TARGET)-gcc

# Pick whichever grub-mkrescue exists: the plain one (Linux / 42 cluster) first,
# then the i686-elf cross build installed via Homebrew on macOS.
GRUB_MKRESCUE := $(shell command -v grub-mkrescue 2>/dev/null || command -v i686-elf-grub-mkrescue 2>/dev/null)

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

myos.bin: boot.o $(OBJFILES)
	$(CC) -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o $(OBJFILES) -lgcc

boot.o: boot.s
	$(TARGET)-as ./boot.s -o boot.o

myos.iso: myos.bin grub.cfg
	mkdir -p isodir/boot/grub
	cp myos.bin isodir/boot/myos.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o myos.iso isodir

all: myos.bin
# TODO: docker build -t kfs . -> build docker image
# docker-compose up -d 
# docker exec -it kfs bash
# TODO: mkdir -p tests 

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
