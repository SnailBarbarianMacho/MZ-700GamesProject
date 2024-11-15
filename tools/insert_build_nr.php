<?php

declare(strict_types = 1);
/**
 * バイナリの一部をビルド番号に書き換えます
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2023.07.01
 */

 // --------------------------------
// 引数チェック
if (count($argv) !== 4)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " build-nr-file.asm rewritten-file address-offset\n");
    exit(1);
}

$filename_build_nr  = $argv[1];
$filename           = $argv[2];
$addr               = $argv[3];

// ビルド番号を読み取る
$file_build_nr = fopen($filename_build_nr, "r");
if (!$file_build_nr) {
    fwrite(STDERR, "File open error[$filename_build_nr]\n");
    exit(1);
}
$str = fgets($file_build_nr);
fclose($file_build_nr);
$arr = explode(' ', $str);
$build_nr = 0;
foreach($arr as $a) {
    if (is_numeric($a)) {
        $build_nr = intval($a);
        break;
    }
}

// アドレスを読み取る
$addr = (stripos($addr, '0x') === 0) ? intval(substr($addr, 2), 16) : (int)$addr;

// バイナリを読む
$binary = file_get_contents($filename);
if (!$binary) {
    fwrite(STDERR, "File open error[$filename]\n");
    exit(1);
}

//echo($build_nr. "\n");
$binary = substr($binary, 0, $addr) . pack('v', $build_nr) . substr($binary, $addr + 2);


// バイナリを書き換えて出力
file_put_contents($filename, $binary);
