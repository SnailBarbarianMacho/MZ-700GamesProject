<?php

declare(strict_types = 1);
/**
 * z88dk map ファイルを EmuZ700 でも読めるシンボルファイルに変換
 *
 * l_bgm1Init_00103 = $1F4A ; addr, local, , bgm_c, code_compiler, game/src/game/bgm.c:330
 * を
 * $1F4A l_bgm1Init_00103
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
$inMapFilename = $argv[1];
$outSymFilename = $argv[2];

if (file_exists($inMapFilename) === false) {
    fwrite(STDERR, "file not found[$inMapFilename]\n");
    exit(1);
}

// -------------------------------- 入力
$fhmap = fopen($inMapFilename, 'r');
$outText = '';
while ($line = fgets($fhmap)) {
    $line = preg_replace( '/ +/', ' ', $line);
    $words = explode(' ', $line);
    $addr = substr($words[2], 1, 4);
    $outText .= $addr . ' ' . $words[0] . "\n";
}
fclose($fhmap);

// -------------------------------- 出力
file_put_contents($outSymFilename, $outText);
