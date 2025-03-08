TARGET=i386-elf

kernel.o: kernel.c
	$(TARGET)-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

boot.o: boot.s
	$(TARGET)-as boot.s -o boot.o

myos.bin: boot.o kernel.o
	$(TARGET)-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc

myos.iso: myos.bin grub.cfg
	mkdir -p isodir/boot/grub
	cp myos.bin isodir/boot/myos.bin
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso isodir

all: myos.bin

clean:
	rm -f boot.o
	rm -f kernel.o
	rm -f myos.bin
	rm -f myos.iso

re: clean all

start: myos.bin
	qemu-system-i386 -kernel myos.bin

start-iso: myos.iso
	qemu-system-i386 -cdrom myos.iso

.PHONY: all clean re start start-iso
