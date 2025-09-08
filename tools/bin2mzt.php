<?php

declare(strict_types = 1);
require_once('nwk-classes/utils/mzt.class.php');

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
$bin_filename = $argv[1];
$name         = $argv[2];
$load_addr    = $argv[3];
$exec_addr    = $argv[4];
$mzt_filename = $argv[5];

// ファイル存在チェック
if (file_exists($bin_filename) === false)
{
    fwrite(STDERR, "File not found[$bin_filename]\n");
    exit(1);
}

// ファイル ロード
$bin_data_str = file_get_contents($bin_filename);
if (!$bin_data_str) {
    fwrite(STDERR, "File open error[$bin_filename]\n");
    exit(1);
}

// ロード, 実行アドレス. '0x' で始まるなら 16 進数
$load_addr = strtolower($load_addr);
$exec_addr = strtolower($exec_addr);
if (str_starts_with($load_addr, '0x')) {
    $load_addr = hexdec(substr($load_addr, 2));
}
if (str_starts_with($exec_addr, '0x')) {
    $exec_addr = hexdec(substr($exec_addr, 2));
}

try {
    $mzt = new \nwk\utils\MZT($bin_data_str, $name, $load_addr, $exec_addr);
} catch (\Exceprion $e) {
    fwrite(STDERR, "[$bin_filename] " . $e->getMessage());
    exit(1);
}
fwrite(STDOUT, "[$mzt_filename] " . $mzt->getInfoString() . "\n");
if (!$mzt->save($mzt_filename)) {
    fwrite(STDOUT, "[$mzt_filename] save failed\n");
}
