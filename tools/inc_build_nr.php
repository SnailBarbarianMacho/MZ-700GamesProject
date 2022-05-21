<?php

declare(strict_types = 1);
/**
 * ビルド番号のインクリメント
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2021.07.11
 */

 // --------------------------------
// 引数チェック
if (count($argv) !== 2)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " buildNrFile.asm\n");
    exit(1);
}

$buildNrFile = $argv[1];

// ファイル存在チェック
if (file_exists($buildNrFile) === false)
{
    fwrite(STDERR, "File not found[$buildNrFile]\n");
    exit(1);
}

$file = fopen($argv[1], "r");
if (!$file) {
    fwrite(STDERR, "File open error[$buildNrFile]\n");
    exit(1);
}

$str = fgets($file);
fclose($file);

$arr = explode(' ', $str);
foreach($arr as $a) {
    if (is_numeric($a)) {
        $a = $a + 1;
        $file = fopen($buildNrFile, "w");
        fwrite($file, "defw ".$a);
        fclose($file);
        fwrite(STDERR, "==== Build Nr[$a] ====\n");
    }
}
