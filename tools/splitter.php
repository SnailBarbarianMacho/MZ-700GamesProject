<?php

declare(strict_types = 1);
/**
 * バイナリデータを指定したサイズに分割します.
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2024.02.26
 */


// --------------------------------

// 引数チェック
if (count($argv) !== 3)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in-file bytes\n");
    exit(1);
}
$in_file = $argv[1];

// 分割バイト値の読み取り
$size = intval($argv[2], 0);
if ($size === 0) {
    fwrite(STDERR, 'Parameter size is invalid:' . $argv[2] . "\n");
    exit(1);
}

// ファイル存在チェック
if (file_exists($in_file) === false)
{
    fwrite(STDERR, 'File not found:' . $in_file . "\n");
    exit(1);
}

// ロード
$data = file_get_contents($in_file);
if ($data === false) {
    fwrite(STDERR, 'File load error:' . $in_file . "\n");
    exit(1);
}

$pathinfo = pathinfo($in_file);
$out_dirname   = $pathinfo['dirname'];
$out_filename  = $pathinfo['filename'];
$out_extension = $pathinfo['extension'];

// 分割
$start = 0;
$ct    = 0;
while (true) {
    $subdata = substr($data, $start, $size);
    if (empty($subdata)) {
        break;
    }
    $out_name = $out_dirname.'/'.$out_filename.'_'.$ct.'.'.$out_extension;
    //echo($out_name);
    if (file_put_contents($out_name, $subdata) === false) {
        fwrite(STDERR, 'File save error:' . $out_name . "\n");
        exit(1);
    }
    $ct++;
    $start += $size;
}
