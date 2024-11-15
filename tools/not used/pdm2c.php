<?php

declare(strict_types = 1);
require_once('nwk-classes/sound/wav.class.php');
require_once('nwk-classes/utils/error.class.php');

/**
 * PDM (データ密度変調)データを c 配列または 視聴用 wav ファイルに変換します
 * - wav2pdm.php と併用して使います
 *
 * @author Snail Barbarian Macho (NWK) 2024.06.19
 */

// --------------------------------
$error = new nwk\utils\Error();

// ---- 引数チェック
if (count($argv) < 3)
{
    $error->error(-1, 'Usage: php ' . $argv[0] . " in.pdm.txt out.c [options...]\n" .
        "  Options are:\n" .
        "  --wav\n"
    );
    exit(1);
}

// ---- オプション解析
// 手抜きなので複数同じオプションがあったら知らん
$out_wav   = false;

for ($i = 3; $i < count($argv); $i++) {
    $arg = $argv[$i];
    if ($arg === '--wav') {
        $out_wav   = true;
    } else {
        $error->error(-1, "Invalid options: $arg\n");
        exit(1);
    }
}

// ---- ファイル名チェック
$filename_in_txt = $argv[1];
$filename_out_c  = $argv[2];
$error->setFilename($filename_in_txt);

if ($filename_in_txt == $filename_out_c) {
    $error->error(-1, "Filenames are the same\n");
    exit(1);
}

// ---------------- ファイルを読み込む
$in_str = file_get_contents($filename_in_txt);
if ($in_str === false) {
    $error->error(-1, "file open error\n");
    exit(1);
}

$in_str = preg_replace('/\s*/', '', $in_str);
$len = strlen($in_str);
$pdm = [];
// '1' 以外は 0 扱い
for ($i = 0; $i < $len; $i++) {
    $pdm[] = substr($in_str, $i, 1) === '1' ? 1 : 0;
}


// ---------------- 視聴用 wav 作成
if ($out_wav) {
    $wav = new nwk\Wav\Wav($error);

    $data = [];
    foreach($pdm as $p) {
        $data[] = $p ? -30000 : 30000;
    }

    $wav->setData($data);
    $out_wav = $wav->export();
    $pathinfo = pathinfo($filename_in_txt);
    $filename_out_wav = $pathinfo['dirname'] . '/' . $pathinfo['filename'] . '.wav';

    $result = file_put_contents($filename_out_wav, $out_wav);
    if ($result == false) {
        $error->error(-1, "file open error[$filename_out_wav]\n");
        exit(1);
    }
}


// ---------------- データ作成
$out = array_fill(0, 256, 0x00);
foreach ($pdm as $i => $d) {
    $bit = 7 - ($i >> 8);
    $idx = $i & 0xff;
    if ($d != 0) { $out[$idx] |= (1 << $bit); }
    //echo("$bit");
}

$out_str = "// $filename_in_txt\n";
foreach ($out as $i => $d) {
    $out_str .= sprintf("0x%02x, ", $d);
    if (($i & 0x0f) === 0x0f) { $out_str .= "\n"; }
}


// ---------------- データ出力
$result = file_put_contents($filename_out_c, $out_str);
if ($result == false) {
    $error->error(-1, "file open error[$filename_out_c]\n");
    exit(1);
}
