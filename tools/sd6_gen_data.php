<?php

declare(strict_types = 1);
require_once('nwk-classes/utils/error.class.php');

/**
 * パルス幅テーブルとドラムデータから sd6 用データを作成します
 * 上3bit がドラムデータ, 下5bit が各テーブル
 * @author Snail Barbarian Macho (NWK) 2024.07.29
 */

// --------------------------------
$error = new nwk\utils\Error();

// ---- 引数チェック
if (count($argv) != 6) {
    $error->error('Usage: php ' . $argv[0] . " in-pulse.txt in-drum0.txt in-drum1.txt in-drum2.txt out-data.h");
    exit(1);
}

// ---- ファイル名チェック
$filenames_in_txt = array_slice($argv, 1, count($argv) - 2);
$filename_out_c  = end($argv);

foreach($filenames_in_txt as $filename_in_txt) {
    if (strcmp($filename_in_txt, $filename_out_c) == 0) {
        $error->setFilename($filename_in_txt);
        $error->error("There is an input file with the same name as the output file");
        exit(1);
    }
}

// ---------------- ファイルを読み込む
$in_strs = [];
foreach($filenames_in_txt as $filename_in_txt) {
    $in_str = file_get_contents($filename_in_txt);
    if ($in_str === false) {
        $error->setFilename($filename_in_txt);
        $error->error("file open error");
        exit(1);
    }
    $in_strs[] = $in_str;
}


// ---------------- データ作成
$data = array_fill(0, 256, 0x00);

foreach($in_strs as $b=>$in_str) {
    $in_str = preg_replace('/\/\/.*\n/', '', $in_str); // C++行コメント カット
    $in_str = preg_replace('/\s*/', '', $in_str); // 改行スペース全部カット
    if ($b == 0) {  // パルスデータ
        $len = min(512, strlen($in_str));
        // 16進2桁 x 256
        for ($i = 0; $i < $len; $i += 2) {
            $data[$i / 2] = hexdec(substr($in_str, $i, 2));
        }
    } else {    // ドラムデータ($b = 1,2,3)
        $len = min(256, strlen($in_str));
        // '1' 以外は 0 扱い
        $bit = 1 << ($b + 4);
        for ($i = 0; $i < $len; $i++) {
            if (substr($in_str, $i, 1) === '1') {
                $data[$i] |= $bit;
            }
        }
    }
}

$out_str = '';
foreach ($data as $i => $val) {
    $out_str .= sprintf("0x%02x, ", $val);
    if (($i & 0x0f) == 0x0f) {
        $out_str .= "\n";
    }
}


// ---------------- データ出力
$result = file_put_contents($filename_out_c, $out_str);
//echo($out_str);
if ($result == false) {
    $error->setFilename($filename_out_c);
    $error->error("file open error\n");
    exit(1);
}
