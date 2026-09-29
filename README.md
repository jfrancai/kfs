# kfs-4
IDT - interrupt descriptor table

![alt text](<Screenshot 2026-09-03 at 16.52.46.png>)

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
make start (-iso?)
```

## hmmm
