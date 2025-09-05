<?php

declare(strict_types = 1);
/**
 * z88dk map ファイルを EmuZ700 でも読めるシンボルファイルに変換
 *
 * 「l_bgm1Init_00103 = $1F4A ; addr, local, , bgm_c, code_compiler, game/src/game/bgm.c:330」
 *
 * を, アドレス順にソートして
 *
 * 「1F4A l_bgm1Init_00103」
 *
 * に直すだけ
 *
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2021.12.25
 */

// --------------------------------
// 引数チェック
if (count($argv) != 3)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in.map out.emuz.sym\n");
    exit(1);
}
$in_map_filename  = $argv[1];
$out_sym_filename = $argv[2];

if (file_exists($in_map_filename) === false) {
    fwrite(STDERR, "file not found[$in_map_filename]\n");
    exit(1);
}

// -------------------------------- マップ ファイルの入力 & 整形
$fhmap = fopen($in_map_filename, 'r');
$out_lines = [];
while ($line = fgets($fhmap)) {
    $line = preg_replace( '/ +/', ' ', $line);
    $words = explode(' ', $line);
    $addr = substr($words[2], 1, 4);
    $out_lines[] = $addr . ' ' . $words[0];
}
fclose($fhmap);

// -------------------------------- シンボル ファイルとしてソート & 出力
asort($out_lines);
file_put_contents($out_sym_filename, implode("\n", $out_lines));
