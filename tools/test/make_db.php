<?php

declare(strict_types = 1);

// --------------------------------
// 音量調整
// 0～239 の対数を取り, 1～56 にまとめる

const N = 240;
$val_n = log10(N);

for ($x = 0; $x < N; $x++) {
    $val = log10($x + 1) / $val_n * 56; // 対数で調整
    //$val = $x / N * 56;                   // 線形で調整
    $val = 56 - (int)$val;
    if ($val >= 57) { $val = 0; }
    if ($x === 0) { $val = 0; }             // 56 以上はパルスが H にならないように大きな値にする
    echo("$val, ");
    if (($x & 0x0f) === 0x0f) { echo("\n"); }
    if (($x & 0x3f) === 0x3f) { echo("\n"); }
}
