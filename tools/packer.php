<?php

declare(strict_types = 1);
/**
 * 複数のファイルを結合して, 1つのファイルにします.
 * 使い方は, Usage: 行を参照してください
 *
 * フォーマット
 * 0x0000 ファイル個数(1 byte)
 * 0x0001 ファイル 1 オフセット(2 bytes)
 * 0x0003 ファイル 2 オフセット(2 bytes)
 *             :
 * 0xXXXX ファイル n オフセット(2 bytes)
 * 0xXXXX ファイル末端オフセット(2 bytes)
 * 0xXXXX ファイル 1 (ファイル 2 オフセット - ファイル 1 オフセット bytes)
 * 0xXXXX ファイル 2 (ファイル 3 オフセット - ファイル 2 オフセット bytes)
 *             :
 * 0xXXXX ファイル n (ファイル末端オフセット - ファイル n オフセット bytes)
 *
 * @author Snail Barbarian Macho (NWK) 2021.09.25
 */

// 引数チェック
if ((count($argv) < 4) || ((count($argv) - 2) % 2 != 0)) {
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in-file1 max-size1 in-file2 max-size2 ... out.pak\n");
    exit(1);
}

// ファイル存在チェック
$err = false;
$filenames = [];
for ($i = 1; $i < count($argv) - 1; $i += 2) {
    $param = $argv[$i];
    $filenames[] = $param;
    if (file_exists($param) === false) {
        fwrite(STDERR, 'File not found[' . $param . "]\n");
        $err = true;
    }
}

// 最大サイズ読み込み
$maxSizeArr   = [];
for ($i = 2; $i < count($argv) - 1; $i += 2) {
    $param = $argv[$i];
    if (is_numeric($param)) {
        $maxSizeArr[] = (int)$param;
    } else {
        if (strpos($param, '0x') === 0) {
            $param = substr($param, 2);
            if (ctype_xdigit($param)) {
                $maxSizeArr[] = hexdec($param);
            } else {
                fwrite(STDERR, 'Invalid max size[' . $param . "]\n");
                $err = true;
            }
        }
    }
}
if ($err) {
    exit(1);
}
//print_r($maxSizeArr);


// ファイル読み込み
$dataArr   = [];
foreach ($filenames as $i => $filename) {
    $data = file_get_contents($filename);
    if ($data === false) {
        fwrite(STDERR, "File load error[$filename]\n");
        $err = true;
    }
    $dataArr[] = $data;

    // サイズチェック
    if ($maxSizeArr[$i] < strlen($data)) {
        fwrite(STDERR, "[$filename] File size[" . strlen($data) . '] is largeer than [' . $maxSizeArr[$i] . "]\n");
    }
}
if ($err) {
    exit(1);
}


// オフセット計算
//echo(sprintf("count:0x%02x\n", count($dataArr)));
$out = pack('C1', count($dataArr));
$addr = count($dataArr) * 2 + 2 + 1;
foreach ($dataArr as $data) {
    //echo(sprintf("addr:0x%04x\n", $addr));
    $out .= pack('v', $addr);
    $addr += strlen($data);
}
//echo(sprintf("end:0x%04x\n", $addr));
$out .= pack('v', $addr);

// データ合成
foreach ($dataArr as $data) {
    $out .= $data;
}

// 出力
file_put_contents($argv[count($argv) - 1], $out);
