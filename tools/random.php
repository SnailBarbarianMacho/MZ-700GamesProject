<?php

declare(strict_types = 1);

// --------------------------------
/**
 * ランダム データ 作成
 */

for ($i = 0; $i < 10; $i++) {
    for ($j = 0; $j < 20; $j++) {
        $r = max(mt_rand(1, 10) + 10 - $j / 2, 1);
        printf("0x%02x, ", $r);

    }
    printf("\n");
}
