# kfs-2

## Usage

If the system GRUB BIOS modules are unavailable, download and extract them
locally before building an ISO:

```sh
mkdir -p .local/grub
url=$(dnf repoquery --location grub2-pc-modules | tail -1)
curl -L "$url" -o .local/grub/grub2-pc-modules.rpm
rpm2cpio .local/grub/grub2-pc-modules.rpm | cpio -idm --quiet -D .local/grub
```

Run the kernel directly with QEMU:

```sh
make start
```

To build and run the ISO image:

```sh
make re
make start-iso
```

## Verify the GDT at 0x800

The kernel prints the GDT after terminal initialization, without using the
QEMU monitor:

```
GDTR base=00000800 limit=0037  CS=0008 DS=0010 SS=0018
00 base=00000000 lim=00000000 acc=00 DPL0 null
08 base=00000000 lim=ffffffff acc=9a DPL0 code
10 base=00000000 lim=ffffffff acc=93 DPL0 data
18 base=00000000 lim=ffffffff acc=93 DPL0 data
20 base=00000000 lim=ffffffff acc=fa DPL3 code
28 base=00000000 lim=ffffffff acc=f2 DPL3 data
30 base=00000000 lim=ffffffff acc=f2 DPL3 data
```

The data descriptor is initialized with `0x92`; the processor sets the Accessed
bit, so it is displayed as `0x93`. The stack segment selector is `0x18`.

In the kernel shell, `gdt` prints the table again and `stack` prints the stack
dump and symbolic trace.


