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
 * n*2-1  ファイル n オフセット(2 bytes)
 * n*2+1  ファイル末端オフセット(2 bytes)
 * n*2+3  ファイル 1 #(ファイル 2 オフセット - ファイル 1 オフセット)
 * 0xXXXX ファイル 2 #(ファイル 3 オフセット - ファイル 2 オフセット)
 *             :
 * 0xXXXX ファイル n #(ファイル末端オフセット - ファイル n オフセット)
 *
 * @author Snail Barbarian Macho (NWK) 2021.09.25
 */

// 引数チェック
if (count($argv) < 3) {
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in-file1 in-file2 ... out.pak\n");
    exit(1);
}

// ファイル読み込み
$err = false;
$data_arr   = [];
for ($i = 1; $i < count($argv) - 1; $i ++) {
    $filename = $argv[$i];
    $data = file_get_contents($filename);
    if ($data === false) {
        fwrite(STDERR, "File load error[$filename]\n");
        $err = true;
    }
    $data_arr[] = $data;
}
if ($err) {
    exit(1);
}


// オフセット計算
//echo(sprintf("count:0x%02x\n", count($data_arr)));
$out = pack('C1', count($data_arr));

$addr = count($data_arr) * 2 + 2 + 1;
foreach ($data_arr as $idx => $data) {
    //echo(sprintf("0x%02x:addr:0x%04x\n", $idx, $addr));
    $out .= pack('v', $addr);
    $addr += strlen($data);
}

//echo(sprintf("end:0x%04x\n", $addr));
$out .= pack('v', $addr);

// データ合成
foreach ($data_arr as $data) {
    $out .= $data;
}

// 出力
file_put_contents($argv[count($argv) - 1], $out);
