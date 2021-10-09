<?php

declare(strict_types = 1);

// --------------------------------
/**
 * ランダム データ 作成
 */

for ($i = 0; $i < 10; $i++) {
    for ($j = 0; $j < 20; $j++) {
        $r = mt_rand(1, 10) + 20 - $j;
        printf("0x%02x, ", $r);
    }
    printf("\n");
}
