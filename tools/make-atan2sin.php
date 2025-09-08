<?php

declare(strict_types = 1);
/**
 * atan2 と sin テーブルの作成.
 * @author Snail Barbarian Macho (NWK) 2021.07.31
 */

// --------------------------------
printf("static u8 const sAtan2Tab[] = {\n");
for ($y = 0; $y < 16; $y++) {
    print("    ");
    for ($x = 0; $x < 16; $x++) {
        if (($x == 0) && ($y == 0)) {
            printf("0x00, ");
            continue;
        }
        $a = atan2($y, $x);
        // 全周 256段階に精度を落とす
        $a = (int)($a * 256 / (2 * M_PI));
        printf("0x%02x, ", $a);
    }
    print("\n");
}
printf("};\n");

// -------------------------------- sin は 1/4 PI だけ.  残りの部分とcos はこれから作ります
printf("static s8 const sSinTab[] = {\n");
for ($i = 0; $i <= 64; $i++) {
    if (($i % 16) == 0) { printf("    "); }
    $a = $i * 2 * M_PI / 256;
    printf("0x%02x, ", sin($a) * 0x40 + 0.5);
    if (($i % 16) == 15) { printf("\n"); }
}
printf("};\n");
