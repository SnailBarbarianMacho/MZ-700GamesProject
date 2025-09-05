<?php

declare(strict_types = 1);
/**
 * バイナリデータを c の配列風にダンプします
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2021.09.25
 */


// --------------------------------

// 引数チェック
if (count($argv) !== 3)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in-file out.c\n");
    exit(1);
}

// ファイル存在チェック
if (file_exists($argv[1]) === false)
{
    fwrite(STDERR, 'File not found:' . $argv[1] . "\n");
    exit(1);
}

// ロード
$data = file_get_contents($argv[1]);
if ($data === false) {
    fwrite(STDERR, 'File load error:' . $argv[1] . "\n");
    exit(1);
}

$bytes = unpack('C*', $data);   // unpack 配列の添え字は 1 から
$addr = 0x0000;
$out = '';
$dump = '';
foreach ($bytes as $i => $b) {
    $out .= sprintf('0x%02x,', $b);
    if (ctype_graph(chr($b)) && $b !== 0x5c) {    // バックスラッシュは含まない
        $dump .= chr($b);
    } else {
        $dump .= '.';
    }

    if ((($i - 1) & 15) == 15) {
        $out .= sprintf(" // 0x%04x %s\n", $addr, $dump);
        $addr += 0x0010;
        $dump = '';
    } else {
        $out .= " ";
    }
}
for ($i = 15 - (count($bytes) & 15); 0 <= $i; $i--) {
    $out .= '      ';
}
$out .=  sprintf("// 0x%04x %s\n", $addr, $dump);
$out .=  sprintf("// total size:0x%04x\n", count($bytes));
if (file_put_contents($argv[2], $out) === false) {
    fwrite(STDERR, 'File save error:' . $out_filename . "\n");
    exit(1);
}
