<?php

declare(strict_types = 1);

// --------------------------------
mulTest();

/**
 * 乗算テスト
 */
function mulTest()
{
    $hist = [];
    for ($i = 0; $i < 256; $i++) {
        for ($j = 0; $j < 65536; $j++) {
            // 計算
            $calc = 0;
            for ($k = 8, $mask = 0x80; $k != 0; $k--, $mask >>= 1) {
                if (($i & $mask) != 0) {
                    $calc += $j;
                }
                if ($mask != 1) {
                    $calc <<= 1;
                }
            }
            $calc >>= (8 + 4);

            // 真の答え
            $ans = ($i * $j) >> (8 + 4);

            // 答え合わせ
            $diff = abs($ans - $calc);
//            if ($diff != 0) {
//                echo("calc failed! ($i * $j) >> 8 = $calc != $ans\n");
//                return;
//            }

            if (isset($hist[$diff])) {
                $hist[$diff]++;
            } else {
                $hist[$diff] = 1;
            }
        }
    }

    // 結果表示
    $ct = 0;
    foreach ($hist as $diff => $count) {
        if (100 < $ct) {
            break;
        }
        echo("hist[$diff] = $count\n");
        $ct++;
    }
}