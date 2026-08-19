# Dùng Clang để cross-compile trực tiếp sang target i386 freestanding
CC := clang --target=i386-unknown-none-elf -march=i386
AS := as --32
LD := ld -m elf_i386

# Tìm grub-mkrescue có sẵn trên hệ thống
GRUB_MKRESCUE := $(shell command -v grub-mkrescue 2>/dev/null || command -v i386-elf-grub-mkrescue 2>/dev/null || command -v i686-elf-grub-mkrescue 2>/dev/null)

PROJDIRS := src includes

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

all: myos.bin

%.o: src/%.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

# Sửa boot.s: Dùng `as --32` (hoặc `nasm -f elf32` nếu boot.s viết bằng cú pháp Intel)
boot.o: boot.s
	$(AS) ./boot.s -o boot.o

# Sửa link: Dùng LD trực tiếp để không bị phụ thuộc vào libgcc ngoài
myos.bin: boot.o $(OBJFILES) linker.ld
	$(LD) -T linker.ld -nostdlib boot.o $(OBJFILES) -o myos.bin

myos.iso: myos.bin grub.cfg
	mkdir -p isodir/boot/grub
	cp myos.bin isodir/boot/myos.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o myos.iso isodir

clean:
	-@$(RM) $(wildcard $(OBJFILES) $(DEPFILES) $(TSTFILES) pdclib.a pdclib.tgz)
	$(RM) boot.o 
	$(RM) $(OBJFILES)
	$(RM) myos.bin
	$(RM) myos.iso
	$(RM) -r isodir

re: clean all

start: myos.bin
	qemu-system-i386 -kernel myos.bin

start-iso: myos.iso
	qemu-system-i386 -cdrom myos.iso

todolist:
	-@for file in $(ALLFILES:Makefile=); do fgrep -H -e TODO -e FIXME $$file; done; true

-include $(DEPFILES)

.PHONY: all clean re start start-iso