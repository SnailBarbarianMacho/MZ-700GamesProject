<?php

declare(strict_types = 1);
require_once 'nwk-classes/utils/utils.class.php';

/**
 * map ファイルを参考に, バイナリファイルから bss セクション相当を削除します
 * - z88dk の「クラシック ライブラリ」使用時は bss セクションの削除機能が無いので作りました
 * - __BSS_head から末端を削除します
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2024.10.25
 */

// オプション解析
$args = [];
$errs = '';
$options = \nwk\utils\Utils::getOpt($argv, array('e::', 'h'), array('help'), $args, $errs);
if ($errs) {
    fwrite(STDERR, "Invalid option(s):$errs\n");
    exit(1);
}

// 引数チェック
if (count($args) != 3 || isset($options['h']) || isset($options['help'])) {
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in-bin-file map-file out-bin-file [options]\n" .
        "  Options are one of:\n" .
        "  -e<n> Error if BSS section size less than or equal n[0,] bytes\n" .
        "  -h, --help\n");
    exit(1);
}

$error_bss_size = -1;
if (isset($options['e'])) {
    $error_bss_size = $options['e'];
    if (!is_numeric($error_bss_size)) {
        fwrite(STDERR, "Invalid -e option value:$error_bss_size\n");
        exit(1);
    }
}

// ファイル読み込み
$bin_filename = $args[0];
$map_filename = $args[1];
$out_filename = $args[2];

$bin_data = file_get_contents($bin_filename);
if ($bin_data === false) {
    fwrite(STDERR, "$bin_filename: File open error\n");
    exit(1);
}

$map_data = file_get_contents($map_filename);
if ($map_data === false) {
    fwrite(STDERR, "$map_filename: File open error\n");
    exit(1);
}

$map_data = str_replace(array("\r\n", "\r", "\n"), "\n", $map_data);
$map_arr  = explode("\n", $map_data);
$head = -1;
$tail = -1;
$bss_head = -1;
$bss_end_tail = -1;
$ct = 0;
foreach ($map_arr as $line) {
    if (strpos($line, '__head ') === 0) {
        $head = $line;
        $ct++;
    }
    if (strpos($line, '__tail ') === 0) {
        $tail = $line;
        $ct++;
    }
    else if (strpos($line, '__BSS_head ') === 0) {
        $bss_head = $line;
        $ct++;
    }
    else if (strpos($line, '__BSS_END_tail ') === 0) {
        $bss_end_tail = $line;
        $ct++;
    }
    if ($ct === 4) { break; }
}

if ($head === -1) {
    fwrite(STDERR, "$map_filename: Symbol '__head' is not found\n");
    exit(1);
}
if ($tail === -1) {
    fwrite(STDERR, "$map_filename: Symbol '__tail' is not found\n");
    exit(1);
}
if ($bss_head === -1) {
    fwrite(STDERR, "$map_filename: Symbol '__BSS_head' is not found\n");
    exit(1);
}
if ($bss_end_tail === -1) {
    fwrite(STDERR, "$map_filename: Symbol '__BSS_END_tail' is not found\n");
    exit(1);
}


/** シンボルファイルの1行からアドレスを抽出. 見つからないなら -1 */
function getAddress_(string $line): int
{
    preg_match('/ \$[0-9A-Fa-f]{4} /', $line, $matches);
    if (count($matches) !== 1) {
        return -1;
    }
    return hexdec(substr($matches[0], 1));
}


$head_addr = getAddress_($head);
$tail_addr = getAddress_($tail);
$bss_head_addr = getAddress_($bss_head);
$bss_end_tail_addr = getAddress_($bss_end_tail);
if ($head_addr === -1) {
    fwrite(STDERR, "$map_filename: Symbol '__head' address is not found\n");
    exit(1);
}
if ($tail_addr === -1) {
    fwrite(STDERR, "$map_filename: Symbol '__tail' address is not found\n");
    exit(1);
}
if ($bss_head_addr === -1) {
    fwrite(STDERR, "$map_filename: Symbol '__BSS_head' address is not found\n");
    exit(1);
}
if ($bss_end_tail_addr === -1) {
    fwrite(STDERR, "$map_filename: Symbol '__BSS_END_tail' address is not found\n");
    exit(1);
}

if ($head_addr > $tail_addr) {
    fwrite(STDERR, sprinf("$map_filename: Invalid __head, __tail address: 0x%04x > 0x%04x\n", $head_addr, $tail_addr));
    exit(1);
}
if ($bss_head_addr > $bss_end_tail_addr) {
    fwrite(STDERR, sprinf("$map_filename: Invalid __BSS_head, __BSS_END_tail address: 0x%04x > 0x%04x\n", $bss_head_addr, $bss_end_tail_addr));
    exit(1);
}
if ($tail_addr !== $bss_end_tail_addr) {
    fwrite(STDERR, sprintf("$map_filename: The BSS section must be last: 0x%04x != 0x%04x\n", $tail_addr, $bss_end_tail_addr));
    exit(1);
}
if ($tail_addr !== $head_addr + strlen($bin_data)) {
    fwrite(STDERR, sprintf("$map_filename: Invalid __tail address: 0x%04x != 0x%04x\n", $tail_addr, $head_addr + strlen($bin_data)));
    exit(1);
}
if ($bss_head_addr < $head_addr || $tail_addr < $bss_head_addr) {
    fwrite(STDERR, sprintf("$map_filename: The __BSS_head address(0x%04x) is out of range:[0x%04x, 0x%04x]\n", $bss_head_addr, $head_addr, $tail_addr));
    exit(1);
}
$bss_size = $bss_end_tail_addr - $bss_head_addr;
if ($bss_size <= $error_bss_size) {
    fwrite(STDERR, sprintf("$map_filename: The BSS section size is too small! 0x%04x <= 0x%04x\n", $bss_size, $error_bss_size));
    exit(1);
}

// bss セクションの削除と出力
$out = substr($bin_data, 0, $bss_head_addr - $head_addr);
printf("$bin_filename:  Load#Size:0x%04x#0x%04x  BSS:0x%04x (0x%04x bytes removed)\n",
    $head_addr, $tail_addr - $head_addr, $bss_head_addr, $tail_addr - $bss_head_addr);
file_put_contents($out_filename, $out);
