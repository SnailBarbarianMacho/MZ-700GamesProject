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
$out = '[' . $in_size . "]: $filesize (max $max_size) bytes.";
if ($filesize === false || $filesize > $max_size) {
    fwrite(STDERR, "$out Size check failed\n");
    exit(1);
} else {
    print("$out Size check OK\n");
}
