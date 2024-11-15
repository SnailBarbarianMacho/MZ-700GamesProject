<?php

declare(strict_types = 1);
/**
 * バイナリから MZT ファイルを生成します
 * 使い方は, Usage: 行を参照してください
 *
 * MZT ファイル フォーマット
 * 0x00     : ファイル モード (0x01 = バイナリ)
 * 0x01-0x11: ファイル名. 大文字のみ. スペースOK. 0x0d 末端. 残りは 0x0d でパディング
 * 0x12-0x13: バイナリのサイズ
 * 0x14-0x15: ロード アドレス
 * 0x16-0x17: 実行開始アドレス
 * 0x18-0x7f: とりあえず 0x00 で埋める
 *
 * @author Snail Barbarian Macho (NWK) 2021.07.06
 */

// --------------------------------

// 引数チェック
if (count($argv) !== 6)
{
    fwrite(STDERR, 'Usage: php '.$argv[0]." bin-file progename load-addr exec-addr outfile.mzt\n");
    fwrite(STDERR, "  progname can use uppercase space, up to 16 characters\n");
    fwrite(STDERR, "  The addresses can be decimal or hexadecimal(0x)\n");
    exit(1);
}
$binFilename = $argv[1];
$progName    = $argv[2];
$loadAddr    = $argv[3];
$execAddr    = $argv[4];
$outMzt      = $argv[5];

// ファイル存在チェック
if (file_exists($binFilename) === false)
{
    fwrite(STDERR, "File not found[$binFilename]\n");
    exit(1);
}

// プログラム名. 小文字は大文字に, 16文字に揃えて末端に 0x0d で埋めて 17 バイトにします
$progName = substr(strtoupper($progName), 0, 16);
for ($i = strlen($progName); $i < 17; $i++) {
    $progName .= pack("C", 0x0d);
}

// ロード, 実行アドレス. '0x' で始まるなら 16 進数
$loadAddr = strtolower($loadAddr);
$execAddr = strtolower($execAddr);
if (str_starts_with($loadAddr, '0x')) {
    $loadAddr = hexdec(substr($loadAddr, 2));
}
if (str_starts_with($execAddr, '0x')) {
    $execAddr = hexdec(substr($execAddr, 2));
}

// ファイル ロード
$binData = file_get_contents($binFilename);
if (!$binData) {
    fwrite(STDERR, "File open error[$binFilename]\n");
    exit(1);
}
$binSize =  strlen($binData);

printf("$outMzt:  Load#Size:0x%04x#0x%04x  Exec:0x%04x\n", $loadAddr, $binSize, $execAddr);


$data = pack("C", 0x01);        // ファイル モード
$data .= $progName;             // プログラム名
$data .= pack("v", $binSize);   // バイナリ サイズ
$data .= pack("v", $loadAddr);  // ロード アドレス
$data .= pack("v", $execAddr);  // 実行開始アドレス
for ($i = 0; $i < 0x68; $i++)
{
    $data .= pack("C", 0x00);  // ファイル名
}
$data .=  $binData;

file_put_contents($outMzt, $data);
