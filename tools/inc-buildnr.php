<?php

declare(strict_types = 1);
/**
 * ビルド番号のインクリメント
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2023.07.01
 */

 // --------------------------------
// 引数チェック
if (count($argv) !== 2)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " build-nr-file.asm\n");
    exit(1);
}

$filename_build_nr = $argv[1];

// ファイル存在チェック
if (file_exists($filename_build_nr) === false) {
    fwrite(STDERR, "File not found[$filename_build_nr]\n");
    exit(1);
}

// ファイルを読み取る
$file_build_nr = fopen($filename_build_nr, "r");
if (!$file_build_nr) {
    fwrite(STDERR, "File open error[$filename_build_nr]\n");
    exit(1);
}
$str = fgets($file_build_nr);
fclose($file_build_nr);

// インクリメントして書き込む. エラー起きたら知らん
$arr = explode(' ', $str);
foreach($arr as $a) {
    if (is_numeric($a)) {
        $a = $a + 1;
        $file_build_nr = fopen($filename_build_nr, "w");
        fwrite($file_build_nr, "defw ".$a);
        if (!$file_build_nr) {
            fwrite(STDERR, "File open error[$filename_build_nr]\n");
            exit(1);
        }
        fclose($file_build_nr);
        fwrite(STDERR, "==== Build Nr[$a] ====\n");
        break;
    }
}
