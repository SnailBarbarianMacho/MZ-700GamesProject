<?php

declare(strict_types = 1);
/**
 * ビルド ログを取ります
 * - CSV 形式で「ビルド日付, ビルド番号, ファイルサイズ1 ファイルサイズ2」を書き出します
 *   プログラムの成長を眺めて楽しみます?
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2023.07.01
 */

 // --------------------------------
// 引数チェック
if (count($argv) !== 5)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " build-nr-file.asm file1 file2 build-log.csv\n");
    exit(1);
}

$filename_build_nr  = $argv[1];
$filename1          = $argv[2];
$filename2          = $argv[3];
$filename_build_log = $argv[4];

// ファイル存在チェック
if (file_exists($filename_build_nr) === false) {
    fwrite(STDERR, "File not found[$filename_build_nr]\n");
    exit(1);
}
$filesize1 = filesize($filename1);
if ($filesize1 === false) {
    fwrite(STDERR, "File not found[$filename1]\n");
    exit(1);
}
$filesize2 = filesize($filename2);
if ($filesize2 === false) {
    fwrite(STDERR, "File not found[$filename2]\n");
    exit(1);
}
if (file_exists($filename_build_log) === false) {
    fwrite(STDERR, "File not found[$filename_build_log]\n");
    exit(1);
}

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
        $build_nr = $a;
        break;
    }
}

// ログファイル追記
$file_build_log = fopen($filename_build_log, "a+");
if (!$file_build_log) {
    fwrite(STDERR, "File open error[$filename_build_log]\n");
    exit(1);
}
date_default_timezone_set('Asia/Tokyo');
$date = date('Y-m-d H:i:s');
fwrite($file_build_log, "$date,$build_nr,$filesize1,$filesize2\n");
fclose($file_build_log);
