<?php

declare(strict_types = 1);
/**
 * 複数のバイナリ データを, アドレスを指定してマージします.
 * データ間の隙間は 0 でパディングします
 * データが重なるとエラーです
 *
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2021.09.xx
 */


// --------------------------------

// 引数チェック
$nrArgs = count($argv) - 1;
if ($nrArgs < 3 || ($nrArgs & 1) == 0) {
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " org1 in-file1 org2 in-file2 ... ... out.bin\n");
    exit(1);
}

// 引数取り込み
$orgs      = [];
$filenames = [];
for ($i = 0; $i < $nrArgs - 1; $i += 2) {
    $orgs[]      = $argv[$i + 1];
    $filenames[] = $argv[$i + 2];
}
$outFilename = $argv[$nrArgs];

// アドレスを整数化

foreach ($orgs as &$org) {
    if (is_numeric($org)) {                                     // 10 進数
        $org = (int)($org);
    } else if (preg_match('/^0x[0-9A-Za-z]+$/', $org) == 1) {   // 16 進数
        $org = hexdec(substr($org, 2));
    } else {
        fwrite(STDERR, "Invalid org[$org]\n");
        exit(1);
    }
}

//print_r($orgs);
//print_r($filenames);
//print_r($outFilename);
//echo("\n");

// ファイル存在チェック
foreach ($filenames as $filename) {
    if (file_exists($filename) === false) {
        fwrite(STDERR, "File not found[$filename]\n");
        exit(1);
    }
}

// ファイルのロード
$datas = [];
foreach ($filenames as $filename) {
    $data = file_get_contents($filename);
    if ($data === false) {
        fwrite(STDERR, "File open error[$filename]\n");
        exit(1);
    }
    $datas[] = $data;
}

// アドレス計算
$addr = 0;
$out = '';
//fwrite(STDERR, sprintf("==== marging [$outFilename]\n"));
foreach ($datas as $i => $data) {
    $out .= $data;
    $end_addr = $addr + strlen($data);
    fwrite(STDERR, sprintf("　addr abs[0x%04x-0x%04x] rel[0x%04x-0x%04x] filename[%s]\n",
        $addr + $orgs[0], $end_addr + $orgs[0], $addr, $end_addr, $filenames[$i]));
    if ($i != count($datas) - 1) {
        $next_addr = $orgs[$i + 1] - $orgs[0];
        if ($next_addr < $end_addr) {
            fwrite(STDERR, sprintf("Invalid addr. file[%s] must be located at org >= [0x%04x]\n",
                $filenames[$i + 1], $end_addr));
            exit(1);
        }
        for ($j = $end_addr; $j < $next_addr; $j++) { // pad
            $out .= pack('C', 0x00);
        }
    }
    $addr = $next_addr;
}

// 出力
file_put_contents($outFilename, $out);
