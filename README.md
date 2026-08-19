# kfs-2


## How to use:


```
make start
```

## How to check 0x800 

```
make re
qemu-system-i386 -kernel myos.bin -monitor stdio
info registers
```
-> Check GDT = 0x800

GDT=     00000800 00000037          ← table at 0x800 ✅
CS =0008 00000000 ffffffff 00cf9a00 ← CS use selector @ 0x08, access 9a, flat 4GB
DS =0010 00000000 ffffffff 00cf9300 ← DS/ES/SS/FS/GS use 0x10, access 93


