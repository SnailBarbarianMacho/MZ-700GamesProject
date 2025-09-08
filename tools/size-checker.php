<?php

declare(strict_types = 1);
/**
 * ファイルサイズをチェックします
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2023.09.20
 */


// --------------------------------

// 引数チェック
if (count($argv) !== 3) {
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in-file size(bin(0b-), oct(0-), dec or hex(0x-))\n");
    exit(1);
}

$in_size = $argv[1];

// サイズ値の読み取り
$max_size = intval($argv[2], 0);
if ($max_size === 0) {
    fwrite(STDERR, 'Parameter size is invalid:' . $argv[2] . "\n");
    exit(1);
}

// ファイル存在チェック
if (file_exists($in_size) === false) {
    fwrite(STDERR, 'File not found:' . $in_size . "\n");
    exit(1);
}

// ファイル サイズ チェック
$filesize = filesize($in_size);
$percent = (int)($filesize * 100 / $max_size);
$percent2 = min((int)($percent / 2), 50);
$col = "\e[32m";                                // 緑 ... OK
if (     $percent >= 98) { $col = "\e[31m"; }   // 赤 ... ギリギリ
else if ($percent >= 95) { $col = "\e[33m"; }   // 黄 ... やばい
$col_off = "\e[0m";
$bar = '[' . $col . str_repeat('*', $percent2) . $col_off . str_repeat('-', 50 - $percent2) . ']';
$out = "[$in_size]: $bar";
if ($filesize === false || $filesize > $max_size) {
    $out .= sprintf("0x%04x(%d%%)-0x%04x=0x%04x", $filesize, $percent, $filesize - $max_size, $max_size);
    fwrite(STDERR, "$out Size check failed\n");
    exit(1);
} else {
    $out .= sprintf("0x%04x(%d%%)+0x%04x=0x%04x", $filesize, $percent, $max_size - $filesize, $max_size);
    print("$out Size check OK\n");
}
