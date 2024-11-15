<?php

declare(strict_types = 1);
/**
 * 複数のバイナリ ファイルを連結します
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2021.09.xx
 */


// --------------------------------

// 引数チェック
if (count($argv) < 4) {
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " infile1 infile2 ... outfile\n");
    exit(1);
}

// ロード
$data = '';
for ($i = 1; $i < count($argv) - 1; $i ++) {
    $filename = $argv[$i];
    if (!file_exists($filename)) {
        fwrite(STDERR, "File not found [$filename]");
        exit(1);
    }
    $d = file_get_contents($filename);
    if ($d === false) {
        fwrite(STDERR, "File read error [$filename]\n");
        exit(1);
    }
    $data .= $d;
}

file_put_contents($argv[count($argv) - 1], $data);;
