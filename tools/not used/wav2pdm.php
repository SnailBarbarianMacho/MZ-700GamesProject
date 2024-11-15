<?php

declare(strict_types = 1);
require_once('nwk-classes/sound/wav.class.php');
require_once('nwk-classes/utils/error.class.php');

/**
 * Wav ファイルのリニア PCM(パルス符号変調) を PDM (データ密度変調)用テキストに変換します
 * - pdm2c.php と併用して使います
 *
 * @author Snail Barbarian Macho (NWK) 2024.06.19
 */

// --------------------------------
$error = new nwk\utils\Error();
$b_verbose = false;
$samples   = 2048; // サンプル数上限 1～
$filter    = 3;    // 大きいほど高音がカットされます 0～

// ---- 引数チェック
if (count($argv) < 3)
{
    $error->error(-1, 'Usage: php ' . $argv[0] . " in.wav out.txt [options...]\n" .
        "  Options are:\n" .
        "  --verbose\n" .
        "  --samples=$samples\n" .
        "  --filter=$filter\n"
    );
    exit(1);
}

// ---- オプション解析
// 手抜きなので複数同じオプションがあったら知らん

for ($i = 3; $i < count($argv); $i++) {
    $arg = $argv[$i];
    if ($arg === '--verbose') {
        $b_verbose = true;
    } else if (preg_match('/^\-\-samples\=([0-9]+)$/', $arg, $matches)) {
        $samples = $matches[1];
    } else if (preg_match('/^\-\-filter\=([0-9]+)$/', $arg, $matches)) {
        $filter = $matches[1];
    } else {
        $error->error(-1, "Invalid options: $arg\n");
        exit(1);
    }
}

// ---- ファイル名チェック
$filename_in_wav  = $argv[1];
$filename_out_txt = $argv[2];
$error->setFilename($filename_in_wav);

if ($filename_in_wav == $filename_out_txt) {
    $error->error(-1, "Filenames are the same\n");
    exit(1);
}

// ---------------- ファイルを読み込む
$in_str = file_get_contents($filename_in_wav);
if ($in_str === false) {
    $error->error(-1, "file open error\n");
    exit(1);
}

$wav = new nwk\Wav\Wav($error);
$wav->import($in_str);
if ($error->getNrErrors()) {
    $error->error(-1, "aborted.\n");
    exit(1);
}

$data = $wav->getData();
$samp = count($data);
//echo("$filename_in_wav: samples: $samples\n");
if ($samples < $samp) {
    if ($b_verbose) {
        echo("$filename_in_wav: サンプル数が多いので $samples に切り詰めます:[$samp]\n");
    }
    $samp = $samples;
} else if ($samp < $samples) {
    if ($b_verbose) {
        echo("$filename_in_wav: サンプル数が $samples より少ないのでデータを 0 で詰めます:[$samp]\n");
    }
    for (; $samp < $samples; $samp++) {
        $data[] = 0;
    }
}
//echo(count($data));
//for ($i = 0; $i < 10; $i++) { echo("$data[$i]\n"); }

// ---------------- PDM データ作成
// パルス密度変換(ΣΔ変調)
$qe = 0.0;
$pdm = [];
$pdm2 = [];
for ($i = 0; $i < $samples; $i++) {
    $d   = $data[$i] / 32768.0;
    $val = ($qe <= $d) ? 1.0 : -1.0;
    $qe  = $val - $d + $qe;
    if ($val === -1.0) { $val = 0.0; }
    $pdm[] = $val;
}

// ローパスフィルタ(移動平均法)
for ($i = 0; $i < $samples; $i++) {
    $a = 0;
    for ($j = $i - $filter; $j <= $i + $filter; $j++) {
        if ($j < 0 || $samples <= $j ) {
            $a += 0.5;
        } else {
            $a += $pdm[$j];
        }
    }
    $pdm2[] = (int)($a / ($filter * 2 + 1) + 0.5);
}

if ($b_verbose) {
    foreach($pdm2 as $d) {
        echo($d);
    }
    echo("\n");
}


// ---------------- データ出力
$out_str = '';
foreach($pdm2 as $i => $p) {
    $out_str .= $p ? '1' : '0';
    if (($i & 0xff) == 0xff) { $out_str .= "\n"; }
}


// ---------------- 出力
$result = file_put_contents($filename_out_txt, $out_str);
if ($result == false) {
    $error->error(-1, "file open error[$filename_out_txt]\n");
    exit(1);
}
