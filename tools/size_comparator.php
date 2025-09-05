<?php

declare(strict_types = 1);
/**
 * 「a.zxo は a.bin の xx%」のように,  2つのファイルサイズの比較をします
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2025.07.31
 */


// --------------------------------

// 引数チェック
if (count($argv) !== 3) {
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " cmp-file ref-file\n");
    exit(1);
}

$cmp_filename = $argv[1];
$ref_filename = $argv[2];

// ファイル サイズ比較
$cmp_filesize = filesize($cmp_filename);
if ($cmp_filesize === false) {
    fwrite(STDERR, 'File open failed:' . $cmp_filename . "\n");
    exit(1);
}
$ref_filesize = filesize($ref_filename);
if ($ref_filesize === false) {
    fwrite(STDERR, 'File open failed:' . $ref_filename . "\n");
    exit(1);
}

$percent = (int)($cmp_filesize * 100 / $ref_filesize);
$percent2 = min((int)($percent / 2), 50);
$col = "\e[32m";                                // 緑 ... そこそこ圧縮
if (     $percent > 100) { $col = "\e[35m"; }   // 紫 ... むしろ大きくなってる
else if ($percent >= 80) { $col = "\e[33m"; }   // 黄 ... あまり圧縮効いてない
$col_off = "\e[0m";
$bar = '[' . $col . str_repeat('=', $percent2) . $col_off . str_repeat('-', 50 - $percent2) . ']';
$out = "[$cmp_filename]: $bar" .
    sprintf("0x%04x/0x%04x=%d%%", $cmp_filesize, $ref_filesize, $percent) .
    " of [$ref_filename]\n";
fwrite(STDERR, $out);
