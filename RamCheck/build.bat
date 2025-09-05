z80asm -mz80 -b -r0x1200 ram_check.asm -m
php ../tools/make_mzt.php ram_check.bin "RAM CHECK" 0x1200 0x1200 bin/ram_check.mzt
php ../tools/mzt2wav.php bin/ram_check.mzt bin/ram_check.wav