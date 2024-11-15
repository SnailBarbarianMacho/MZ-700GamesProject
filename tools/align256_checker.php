<?php

declare(strict_types = 1);
/**
 * 256 バイト境界チェッカー
 * - z88dk map ファイルを調べ,
 * - シンボル名 *_align256        ～ *_align256_end    が 256 バイト境界の中にいるかを調べます
 * - シンボル名 bss_compiler_head ～ bss_compiler_tail が 256 バイト境界の中にいるかを調べます
 * - はみ出してたらエラー
 * - 対応する _end シンボルが無くてもエラー
 *
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2024.02.24
 */

// --------------------------------
// 引数チェック
if (count($argv) != 2)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in.map\n");
    exit(1);
}
$in_map_filename  = $argv[1];

if (file_exists($in_map_filename) === false) {
    fwrite(STDERR, "file not found[$in_map_filename]\n");
    exit(1);
}

// -------------------------------- 入力&整形
$bss_label_start = '__bss_compiler_head';
$bss_label_end   = '__bss_compiler_tail';

$fhmap = fopen($in_map_filename, 'r');
$labels = [];
while ($line = fgets($fhmap)) {
    $line = preg_replace( '/ +/', ' ', $line);
    $words = explode(' ', $line);
    if ($words[0] === $bss_label_start ||
        $words[0] === $bss_label_end) {
        $addr = substr($words[2], 1, 4);
        $labels[$words[0]] = $addr;
    } else if (preg_match('/^[A-Za-z0-9_]+_align256(_end)?$/', $words[0], $matches) == 1) {
        $addr = substr($words[2], 1, 4);
        $labels[$matches[0]] = $addr;
    }
}
fclose($fhmap);

// -------------------------------- アドレスやラベルのチェック
$err = false;
foreach($labels as $label => $addr) {
    if (str_ends_with($label, '_align256')) {
        if (check($label, $label.'_end', $labels) === false) {
            $err = true;
        }
    }
}

if (array_key_exists($bss_label_start, $labels)) {
    if (check($bss_label_start, $bss_label_end, $labels) === false) {
        $err = true;
    }
}

if ($err) {
    fwrite(STDERR, "check failed.\n");
    exit(1);
}

printf("[$in_map_filename]: 256-byte alignment check ... OK!\n");


/**
 * $labels の中に, $label_end があるかチェックして,
 * あればアドレスチェック
 */
function check($label_start, $label_end, $labels)
{
    if (!array_key_exists($label_end, $labels)) {
        fwrite(STDERR, "label [$label_end] is not exist\n");
        return false;
    }

    $addr_start = $labels[$label_start];
    $addr_end   = $labels[$label_end];
    //print("$addr_start - $addr_end  $label_start\n");
    if (((hexdec($addr_end) - 1) & 0xff00) !== (hexdec($addr_start) & 0xff00)) {
        $err = true;
        fwrite(STDERR, "alignment error! labels [$label_start($addr_start)] and [$label_end($addr_end)] is not same 256-segment\n");
        return false;
    }
    return true;
}