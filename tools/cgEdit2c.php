<?php

declare(strict_types = 1);
/**
 * 「きゃらぐらえでぃた CGEDIT」 形式のデータを C ソースで使いやすいように整形します
 * 使い方は, Usage: 行を参照してください
 *
 * - CGEDIT 形式の解析
 *   CSV 形式で, 1行 3000 個の項目です.
 *   最初の 1000 個は VRAM
 *   次の 1000 個は ATB の文字の色
 *   次の 1000 個は ATB の背景の色
 * - extraction-table.json の内容
 * ｛
 *    "sTitle_S": {                                "comment_" で始まる文字列のデータは無視
 *       "mode": "Transparent",                    mode は省略すると通常モード
 *       "x": 0, "y": 16, "w": 6, "h": 9
 *    },
 *   :
 *  }
 *
 * @author Snail Barbarian Macho (NWK) 2021.07.06
 */


// --------------------------------

// 引数チェック
if (count($argv) !== 3)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " cgedit.txt extraction-table.json\n");
    exit(1);
}

// ファイル存在チェック
if (file_exists($argv[1]) === false)
{
    fwrite(STDERR, 'File not found:' . $argv[1] . "\n");
    exit(1);
}
if (file_exists($argv[2]) === false)
{
    fwrite(STDERR, 'File not found:'.$argv[2] . "\n");
    exit(1);
}

// cgedit ファイル チェック
$data = explode(',', file_get_contents($argv[1]));
if (count($data) != 40 * 25 * 3) {
    fwrite(STDERR, 'ファイルの異常です:'.$argv[1] . "\n");
    exit(1);
}

// ATB の合成
for ($i = 0; $i < 40 * 25; $i++) {
    $data[$i + 40 * 25] = ($data[$i + 40 * 25] << 4) | $data[$i + 40 * 25 * 2];
}

// 抽出テーブル json ロード
$extTab = json_decode(file_get_contents($argv[2]), true);
if ($extTab === null)
{
    fwrite(STDERR, 'JSON decode error[' . $argv[2] . "\n");
    exit(1);
}

//var_export($extTab);
$pathInfo = pathinfo($argv[2]);

// 抽出テーブルに沿ってデータを出力
foreach ($extTab as $name => $value) {
    if (str_starts_with($name, 'comment_')) {
        continue;
    }
    $mode = $value['mode'] ?? '';
    $x = $value['x'];
    $y = $value['y'];
    $w = $value['w'];
    $h = $value['h'];
    $out  = sprintf("/* This file is made by **** $argv[0] ****.  DO NOT MODIFY! */\n\n");
    $out .= sprintf("// mode:[$mode] pos#size:($x, $y)#($w, $h)\n");
    $nameUpper = strtoupper($name);
    $out .= sprintf("#define CG_{$nameUpper}_WIDTH {$w}\n");
    $out .= sprintf("#define CG_{$nameUpper}_HEIGHT {$h}\n");
    $out .= sprintf("static const u8 s{$name}[] = {\n");
    switch ($mode)
    {
        default:
            break;
        case '':
            $out .= printTable($data, $x, $y, $w, $h);
            break;
        case 'Transparent':
            $out .= printTableTransparent($data, $x, $y, $w, $h);
            break;
    }
    $out .= sprintf("};\n");
    file_put_contents($pathInfo['dirname'].'/'.$name.'.h', $out);
}

exit;

// -------------------------------- 通常出力
function printTable(array $data, int $x, int $y, int $w, int $h): string
{
    // ---------------- TEXT
    $ret = sprintf("    // TEXT\n");
    for ($iy = 0; $iy < $h; $iy++) {
        $ret .= sprintf('    ');
        for ($ix = 0; $ix < $w; $ix++) {
            $c = (int)$data[($iy + $y) * 40 + ($ix + $x)];
            $ret .= sprintf('0x%02x, ', $c & 0xff);
            if (($ix % 10) == 9) {
                $ret .= sprintf(' ');
            }
        }
        $ret .= sprintf("\n");
        if (($iy % 5) == 4) {
            $ret .= sprintf("\n");
        }
    }

    // ---------------- ATB
    $ret .= sprintf("    // ATB\n");
    for ($iy = 0; $iy < $h; $iy++) {
        $ret .= sprintf('    ');
        for ($ix = 0; $ix < $w; $ix++) {
            $a = (int)$data[($iy + $y) * 40 + ($ix + $x) + 1000];
            if (0x100 <= (int)$data[($iy + $y) * 40 + ($ix + $x)]) { $a |= 0x80; }
            $ret .= sprintf('0x%02x, ', $a);
            if (($ix % 10) == 9) {
                $ret .= sprintf(' ');
            }
        }
        $ret .= sprintf("\n");
        if (($iy % 5) == 4) {
            $ret .= sprintf("\n");
        }
    }

    return $ret;
}


// -------------------------------- 透過出力
function printTableTransparent(array $data, int $x, int $y, int $w, int $h): string
{
    // ---------------- TEXT + ATB interleaved
    $ret = sprintf("    // TEXT + ATB transparent + interleaved\n");
    for ($iy = 0; $iy < $h; $iy++) {
        $ret .= sprintf('    ');
        for ($ix = 0; $ix < $w; $ix++) {
            $c = (int)$data[($iy + $y) * 40 + ($ix + $x)];
            $a = (int)$data[($iy + $y) * 40 + ($ix + $x) + 1000];
            if (0x100 <= $c) { $a |= 0x80; }
            if ($c != 0x00) {
                $ret .= sprintf('0x%02x, 0x%02x, ', $c & 0xff, $a);
            } else {
                $ret .= sprintf('0x00,       ');
            }
            if (($ix % 10) == 9) {
                $ret .= sprintf(' ');
            }
        }
        $ret .= sprintf("\n");
        if (($iy % 5) == 4) {
            $ret .= sprintf("\n");
        }
    }
    return $ret;
}
