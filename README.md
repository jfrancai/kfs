kfs-2


make re
qemu-system-i386 -kernel myos.bin -monitor stdio
info registers
-> Check GDT = 0x800

GDT=     00000800 00000037          ← bảng ở 0x800, limit 0x37=55=7×8−1 ✅
CS =0008 00000000 ffffffff 00cf9a00 ← CS dùng selector 0x08, access 9a, flat 4GB
DS =0010 00000000 ffffffff 00cf9300 ← DS/ES/SS/FS/GS dùng 0x10, access 93
