<?php

declare(strict_types = 1);
/**
 * 「きゃらぐらえでぃた CGEDIT」 形式のデータを C ソースで使いやすいように整形します
 * 使い方は, Usage: 行を参照してください
 *
 * - CGEDIT 形式の解析
 *   3000 個の項目からなる 1 行の CSV 形式です. 拡張子 .txt
 *   最初の 1000 個は TEXT(ディスプレイ コード)[0, 511]
 *   次の 1000 個は ATB の文字の色[0, 7]
 *   次の 1000 個は ATB の背景の色[0, 7]
 * - extraction-table.json の内容
 * ｛
 *    "hoge": {                    名前. "comment_" で始まる文字列のデータは無視
 *       "mode": "Transparent",    モード:
 *                                      "Normal" (または省略) 通常
 *                                      "Transparent"         透過
 *                                      "TextOnly"            テキストのみ
 *                                      "Asm"                 アセンブリ コード出力
 *                                      "AsmTextOnly"         アセンブリ コード出力(テキストのみ)
 *                                      "AsmAtbOnly"          アセンブリ コード出力(属性のみ)
 *        "x": 0, "y": 16, "w": 4, "h": 4  CGEDIT でのキャラクタの位置と寸法
 *        "transparent": [          透過となる座標(Asm モードのみ)
 *           [0, 0],                    x, y
 *        ]
 *     },
 *   :
 *  }
 *
 * - 出力
 *   - 通常モードの場合, C 言語形式で, TEXT と ATB が別々に出力されます:
 *     #define CG_HOGE_WIDTH 4
 *     #define CG_HOGE_HEIGHT 4
 *     static u8 const cg_hoge[] = {
 *       // TEXT
 *       0xfa, 0xf7, 0xf3, 0xff,
 *       0x00, 0x00, 0xfe, 0xf1,
 *       0x00, 0xfa, 0xf5, 0x00,
 *       0x00, 0xf2, 0xf1, 0x00,
 *       // ATB
 *       0x70, 0x70, 0x70, 0x70,
 *       0x70, 0x70, 0x70, 0x70,
 *       0x70, 0x70, 0x70, 0x00,
 *       0x70, 0x70, 0x70, 0x00,
 *     };
 *
 *   - Transparent モードの場合, TEXT と ATB が交互に出力されます. テキスト == 0x00 の場合は ATB は出力しません:
 *     #define CG_HOGE_WIDTH 4
 *     #define CG_HOGE_HEIGHT 4
 *     static u8 const cg_hoge[] = {
 *       // TEXT + ATB transparent + interleaved
 *       0xfa, 0x70, 0xf7, 0x70, 0xf3, 0x70, 0xff, 0x70,
 *       0x00,       0x00,       0xfe, 0x70, 0xf1, 0x70,
 *       0x00,       0xfa, 0x70, 0xf5, 0x70, 0x00,
 *       0x00,       0xf2, 0x70, 0xf1, 0x70, 0x00,
 *     };
 *
 *  - TextOnly モードの場合, テキストのみが出力されます
 *     #define CG_HOGE_WIDTH 4
 *     #define CG_HOGE_HEIGHT 4
 *     static u8 const cg_hoge[] = {
 *       // TEXT
 *       0xfa, 0xf7, 0xf3, 0xff,
 *       0x00, 0x00, 0xfe, 0xf1,
 *       0x00, 0xfa, 0xf5, 0x00,
 *       0x00, 0xf2, 0xf1, 0x00,
 *     };
 *
 *   - AsmText, AsmAtb モードの場合, 直接描画するC インライン形式でアセンブラコードを出します.
 *     1キャラ転送の速度は速くなりますが, 汎用性が無くなります
 *
 *        通常モードでの描画          Asm モードでの転送
 *       (2+1bytes, 14T-states)       (2～3bytes, 8～11T-states)
 *      -----------------------------------------------------
 *        ldi                         ld  (HL), data
 *                                    inc L
 *
 *    draw_cmds (描画コマンド)を使って特定の場所の描画方法を変更できます
 *    "hoge": {                        ... 名前
 *       "mode": "AsmText",            ... モード. AsmText, AsmAtb のいずれか.
 *       "draw_cmds": [                ... 描画コマンド(省略可). 下記アセンブラ マクロ参照
 *           ["draw", 2, 2],           ... 指定位置の文字やATBはユーザーが描画します.
 *           ["drawOpt", 2, 2],        ... draw とほぼ同じですが, 最適化の影響を受けます.
 *           ["composeAtbFgBg", 2, 3], ... 指定位置のFG色と背景のBG色を合成します
 *           ["composeAtbBgBg", 2, 7], ... 指定位置のBG色と背景のBG色を合成します
 *       ],
 *       :
 *    },
 *
 *    - テキスト == 0x00 で, 描画コマンドも指定されてない場合, 属性の値によらず透明扱いになり, 描画されません
 *    - 出力されるコードは, 1 行目右方向へ描画, 2行目左方向へ描画, ... のように行ったり来たりスキャンになります
 *    - 定義されす C マクロ
 *      CG_名前_MASK
 *          "draw", "drawOpt" を指定した場合に定義されます. 引数 mask と同じ値(0x07 またはレジスタ名)です
 *    - 定義が必要なアセンブラ マクロ:
 *      CG_名前_SET_VVRAM_ADDR dx, dy, H, L, HL
 *          描画開始時に1回だけ展開されます.
 *          矩形左上端から(dx, dy)だけ移動した描画先テキスト アドレスor属性アドレスを, HL にセットしてください
 *          全レジスタ破壊 OK
 *          例: テキスト VRAM の絶対移動
 *            macro CG_HOGE_SET_VVRAM_ADDR x, y, H, L, HL
 *              VVRAM_TEXT_ADDR(x, y)
 *            endm
 *      CG_名前_MOVE_VVRAM_ADDR dx, dy, x, y, H, L, HL
 *          HL を, (dx, dy) だけ移動します. または, HL を, (x, y)に移動します.
 *          dx は負の場合もあります. dy は 0 か 1.
 *            例1: 改行のみならば, inc H                  (4 T-states)
 *            例2: 絶対移動ならば, ld  HL, 0 + X + Y * w  (10 T-states)
 *          DE, BC 破壊不可
 *          例: テキスト VRAM の絶対移動
 *            macro CG_HOGE_MOVE_VVRAM_ADDR dx, dy, x, y, H, L, HL
 *              if  dy == 1
 *                if dx == -1
 *                    dec     L
 *                    inc     H
 *                elif dx == 0
 *                    inc     H
 *                elif dx == 1
 *                    inc     L
 *                    inc     H
 *                else
 *                    ld      HL,  0 + VVRAM_TEXT_ADDR(x, y)
 *                endif
 *              else                                                // dy = 0
 *                if dx == -1
 *                    dec     L
 *                elif dx == 0
 *                elif dx == 1
 *                    inc     L
 *                else
 *                    ld      L,  0 + x
 *                endif
 *              endif
 *            endm
 *      CG_名前_DRAW_座標X_座標Y HL, val
 *          "draw", "drawOpt" を指定した部分に展開されます.
 *          (HL) に描画してください.
 *          元あった値は引数 val に入ってます. "drawOpt" の場合, レジスタになることがあります
 *          HL, DE, BC 破壊不可. 裏レジスタを使って A を算出して描画するとよいでしょう.
 *          例: ATB合成コード: 新しい属性の下位ニブルと,既存の属性の下位ニブルの合成. mask は使わない
 *
 * @author Snail Barbarian Macho (NWK)
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
    fwrite(STDERR, 'File not found:[' . $argv[1] . "]\n");
    exit(1);
}
if (file_exists($argv[2]) === false)
{
    fwrite(STDERR, 'File not found:['. $argv[2] . "]\n");
    exit(1);
}

// cgedit ファイル チェック
$data = explode(',', file_get_contents($argv[1]));
if (count($data) != 40 * 25 * 3) {
    fwrite(STDERR, 'Invalid file:[' . $argv[1] . "]\n");
    exit(1);
}

// ATB の合成
for ($i = 0; $i < 40 * 25; $i++) {
    $c  = $data[$i + 40 * 25 * 0];
    $fg = $data[$i + 40 * 25 * 1];
    $bg = $data[$i + 40 * 25 * 2];
    $data[$i + 40 * 25] = ($fg << 4) | $bg | (($c >= 256) ? 0x80 : 0x00);
}

// $data の正規化(int 0～255, 大きさ2000にする)
$data = array_slice($data, 0, 40 * 25 * 2, true);
foreach ($data as &$r_val) {
    $r_val = ((int)$r_val) & 0xff;
}

// 抽出テーブル json ロード
$extTab = json_decode(file_get_contents($argv[2]), true);
if ($extTab === null)
{
    fwrite(STDERR, 'JSON decode error. file:[' . $argv[2] . "]\n");
    exit(1);
}

//var_export($extTab);
$pathInfo = pathinfo($argv[2]);

// 抽出テーブルに沿ってデータを出力
foreach ($extTab as $name => $value) {
    if (str_starts_with($name, 'comment_')) {   // コメント扱い
        continue;
    }
    $mode      = 'Normal';  // 省略可
    $x         = -1;        // 省略不可
    $y         = -1;        // 省略不可
    $w         = -1;        // 省略不可
    $h         = -1;        // 省略不可
    $draw_cmds = [];        // 省略可
    foreach ($value as $subkey => $subvalue) {
        switch ($subkey) {
        default: fwrite(STDERR, 'WARN: Unknown sub-key[' . $subkey . "]. name:[$name].\n"); break;
        case 'mode':      $mode      = $subvalue; break;
        case 'x':         $x         = $subvalue; break;
        case 'y':         $y         = $subvalue; break;
        case 'w':         $w         = $subvalue; break;
        case 'h':         $h         = $subvalue; break;
        case 'draw_cmds': $draw_cmds = $subvalue; break;
        }
    }
    if ($x == -1 || $y == -1 || $w == -1 || $h == -1) {
        fwrite(STDERR, "ERROR: Required sub-key(x, y, w, h) is not found. name:[$name]\n");
        break;
    }
    if (1000 <= ($y + $h - 1) * 40 + $x + $w - 1) {
        fprintf(STDERR, "Invalid rectangle size(x:$x, y:$y)#(w:$w, h:$h) name:[$name]\n");
        exit(1);
    }

    $out  = sprintf("/* This file is made by **** $argv[0] ****.  DO NOT MODIFY! */\n\n");
    $out .= sprintf("// name:[$name] pos#size:($x, $y)#($w, $h)\n\n");
    $name_upper = strtoupper($name);
    switch ($mode)
    {
        default:
            fwrite(STDERR, "WARN: Invalid mode[$mode]. name:[$name]. Ignored\n");
            break;
        case '':
        case 'Normal':
            $out .= printCSourceBegin_($name_upper, $name, $w, $h);
            $out .= printTable_($data, $x, $y, $w, $h);
            printCSourceEnd_();
            break;
        case 'Transparent':
            $out .= printCSourceBegin_($name_upper, $name, $w, $h);
            $out .= printTableTransparent_($data, $x, $y, $w, $h);
            printCSourceEnd_();
            break;
        case 'TextOnly':
            $out .= printCSourceBegin_($name_upper, $name, $w, $h);
            $out .= printTableTextOnly_($data, $x, $y, $w, $h);
            printCSourceEnd_();
            break;
        case 'AsmText':
            $out .= printTableAsm_($name, $name_upper, $data, $x, $y, $w, $h, $draw_cmds, true, false);
            break;
        case 'AsmAtb':
            $out .= printTableAsm_($name, $name_upper, $data, $x, $y, $w, $h, $draw_cmds, false, true);
            break;
    }
    $name = str_replace('_', '-', $name);
    file_put_contents($pathInfo['dirname'].'/'.$name.'.h', $out);
}

exit;

// -------------------------------- Cソースの場合の頭とお尻の出力
function printCSourceBegin_($name_upper, $name, $w, $h): string
{
    $ret  = sprintf("#define CG_{$name_upper}_WIDTH {$w}\n");
    $ret .= sprintf("#define CG_{$name_upper}_HEIGHT {$h}\n");
    $ret .= sprintf("static u8 const cg_{$name}[] = {\n");
    return $ret;
}

function printCSourceEnd_(): string
{
    return sprintf("};\n");
}


// -------------------------------- 通常出力
function printTable_(array $data, int $x, int $y, int $w, int $h): string
{
    // ---------------- TEXT
    $ret = sprintf("    // TEXT\n");
    for ($iy = 0; $iy < $h; $iy++) {
        $ret .= sprintf('    ');
        for ($ix = 0; $ix < $w; $ix++) {
            $c = $data[($iy + $y) * 40 + ($ix + $x)];
            $ret .= toByteHexString_($c) . ',';
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
            $idx = ($iy + $y) * 40 + ($ix + $x);
            $a = $data[$idx + 1000];
            if (0x100 <= $data[$idx]) { $a |= 0x80; }
            $ret .= toByteHexString_($a) . ',';
            if (($ix % 10) == 9) {
                $ret .= sprintf(' ');
            }
        }
        $ret .= sprintf("\n");
        if (($iy % 5) == 4) {
            $ret .= sprintf("\n");
        }
    }

    $ret .= sprintf("};\n");
    return $ret;
}


// -------------------------------- 透過出力
function printTableTransparent_(array $data, int $x, int $y, int $w, int $h): string
{
    // ---------------- TEXT + ATB interleaved
    $ret = sprintf("    // TEXT + ATB transparent + interleaved\n");
    for ($iy = 0; $iy < $h; $iy++) {
        $ret .= sprintf('    ');
        for ($ix = 0; $ix < $w; $ix++) {
            $idx = ($iy + $y) * 40 + ($ix + $x);
            $c = $data[$idx       ];
            $a = $data[$idx + 1000];
            if (0x100 <= $c) { $a |= 0x80; }
            if ($c != 0x00) {
                $ret .= toByteHexString_($c) . ',' . toByteHexString_($a) . ', ';
            } else {
                $ret .= sprintf('0x00,      ');
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

    $ret .= sprintf("};\n");
    return $ret;
}


// -------------------------------- テキストのみ出力
function printTableTextOnly_(array $data, int $x, int $y, int $w, int $h): string
{
    // ---------------- TEXT
    $ret = sprintf("    // TEXT\n");
    for ($iy = 0; $iy < $h; $iy++) {
        $ret .= sprintf('    ');
        for ($ix = 0; $ix < $w; $ix++) {
            $c = $data[($iy + $y) * 40 + ($ix + $x)];
            $ret .= toByteHexString_($c) . ',';
            if (($ix % 10) == 9) {
                $ret .= sprintf(' ');
            }
        }
        $ret .= sprintf("\n");
        if (($iy % 5) == 4) {
            $ret .= sprintf("\n");
        }
    }

    $ret .= sprintf("};\n");
    return $ret;
}


// -------------------------------- 直接アセンブラ出力
/**
 */
function printTableAsm_(
    string $name, string $name_upper, array $data,
    int $x, int $y, int $w, int $h, array $draw_cmds,
    bool $b_text): string
{
    $VATB_BG_MASK = 0x07;   // VATB(0, 7, 0)
    $DATA_TERM_MARK = -99;  // $data に刻む右端末端マーク. -1～-98 は左端末端スキップ数になります

    // -------- 矩形領域のコピー
    $data_text = [];
    $data_atb  = [];
    $data_cmd  = [];
    for ($iy = 0; $iy < $h; $iy++) {
        for ($ix = 0; $ix < $w; $ix++) {
            $idx  = ($iy + $y) * 40 + ($ix + $x);
            $data_text[] = $data[$idx];
            $data_atb[] = $data[$idx + 1000];
            $data_cmd[] = '';
        }
    }
    /*$data_text = [
        0, 0, 0, 0,  0, 1, 0, 1,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1, 0, 0, 0,  0, 0, 0, 0,
        0, 0, 0, 0,  0, 1, 1, 1,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1, 1, 1, 1,  1, 1, 0, 0,
        0, 0, 0, 0,  0, 1, 1, 1,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1, 1, 1, 1,  1, 1, 0, 0,
    ];
    $data_atb = [
        0, 0, 0, 0,  1, 1, 0, 1,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1, 0, 0, 0,  0, 0, 0, 0,
        0, 0, 0, 0,  1, 1, 1, 1,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1, 0, 0, 0,  0, 0, 0, 0,
        0, 0, 0, 0,  1, 1, 1, 1,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1, 0, 0, 0,  0, 0, 0, 0,
    ]; /*TEST*/

    // ---------------- draw_cmds の埋め込み
//    $draw_text_cmds = [];   // key:位置 value:描画コマンドの種類
//    $draw_atb_cmds  = [];   // key:位置 value:描画コマンドの種類
    $nr_masks = 0;
    foreach ($draw_cmds as $item) {
        if (!is_array($item) ||
            count($item) != 3 ||
            !is_string($item[0]) ||
            !is_numeric($item[1]) ||
            !is_numeric($item[2]) ) {
                fwrite(STDERR, "ERROR: Invalid draw params. name:[$name].\n");
                exit(1);
        }
        $cmd = $item[0];
        $px  = $item[1];
        $py  = $item[2];
        if ($px < 0 || $w <= $px || $py < 0 || $h <= $py) {
            fwrite(STDERR, "ERROR: Invalid draw coordinate($px, $py). cmd:[$cmd] name:[$name].\n");
            exit(1);
        }
        $pos = $py * $w + $px;
        switch ($cmd) {
            default:
            fwrite(STDERR, "ERROR: Invalid draw command[$cmd](name:[$name], coord:($px, $py)\n");
                exit(1);
            case 'draw':
            case 'drawOpt':
                if ($data_cmd[$pos] != '') {     // 重複
                    fwrite(STDERR, "ERROR: Draw command[$cmd](name:[$name], coord:($px, $py)) is already set to [". $data_cmd[$pos] . "].\n");
                    exit(1);
                }
                $data_cmd[$pos] = $cmd;
                break;
            case 'composeAtbFgBg':
                $nr_masks ++;                   // fall through
            case 'composeAtbBgBg':
                if ($b_text) {                  // Atb のみ
                    fwrite(STDERR, "ERROR: This draw command[$cmd](name:[$name], coord:($px, $py)) is only allowed in Atb mode.\n");
                    exit(1);
                }
                if ($data_cmd[$pos] != '') {     // 重複
                    fwrite(STDERR, "ERROR: Draw command[$cmd](name:[$name], coord:($px, $py)) is already set to [". $data_cmd[$pos] . "].\n");
                    exit(1);
                }
                $data_cmd[$pos] = $cmd;
                break;
        }
    }

    //print_r($data_text);
    //print_r($data_atb);

    // -------- 透明領域の最適化の為に, 矩形の左右端の検出
    //
    //     .A..
    //     B.C.
    //
    // ↑このような矩形(A-Cは表示, .は非表示) を最適化で出力するために, 両端をマーキングします
    //
    //   最適化無し             最適化
    //  ---------------------------------------
    //   ld  HL, 1行目位置      ld  HL, 1行目位置+1
    //   inc L
    //   Aを描画                Aを描画
    //   inc L                  inc L
    //   inc L
    //   inc H                  inc H
    //   dec L
    //   Cを描画                Cを描画
    //   dec L
    //   Bを描画                Bを描画
    //
    // -------- 行左右末端の透明領域に, -スキップ数をマーキングする
    //  0, 0, 0, 0, C, 0, C, 0, 0, 0
    //            ↓
    // -4,-4,-4,-4, C, 0, C,-3,-3,-3
    for ($iy = 0; $iy < $h; $iy++) {
        // 左端の処理
        for ($ix = 0, $skip = -1; $ix < $w; $ix++, --$skip) {
            $idx = $iy * $w + $ix;
            $text = $data_text[$idx];
            $atb  = $data_atb[$idx];
            $cmd  = $data_cmd[$idx];
            if ($text != 0 || $cmd != '') {
                for (--$ix, $skip++ ; 0 <= $ix; --$ix) {
                    $idx = $iy * $w + $ix;
                    $data_text[$idx] = $skip;
                }
                break;
            }
        }
        // 右端の処理
        for ($ix = $w - 1, $skip = -1; $ix >= 0; --$ix, --$skip) {
            $idx = $iy * $w + $ix;
            $text = $data_text[$idx];
            $atb  = $data_atb[$idx];
            $cmd  = $data_cmd[$idx];
            if ($text != 0 || $cmd != '') {
                for ($ix++, $skip++ ; $ix < $w; $ix++) {
                    $idx = $iy * $w + $ix;
                    $data_text[$idx] = $skip;
                }
                break;
            }
        }
    }
    //print_r($data_text);
    //print_r($data_atb);

    // -------- 最適化のために, 最も使用される値をレジスタに置き換えます.
    // テキストでは, B, C, D, E レジスタに割り当てます
    // 属性では, B, C, D レジスタに割り当てます(Eは属性値が入ってる)

    // ---- 使用値のヒストグラム作成
    $hist = array_fill(0, 256, 0);
    $hist[$VATB_BG_MASK] = $nr_masks;
    foreach ($data_text as $idx => $text) {
        if ($text < 0) { continue; }                // 左末端or右末端なのでヒストグラムに入れない
        $atb = $data_atb[$idx];
        $cmd = $data_cmd[$idx];
        if ($cmd == 'draw') { continue; }           // ヒストグラムに入れない描画コマンド
        if ($text == 0 && $cmd == '') { continue; } // 表示しないのでヒストグラムに入れない
        if ($b_text) {
            $hist[$text] ++;
        } else {
            $hist[$atb] ++;
        }
    }
    arsort($hist);                      // 頻度の高い順にソート
    //print_r($hist);
    //print(array_sum($hist));

    // ---- トップ順位を作成
    $top_chars = [];    // トップの使用値
    foreach($hist as $char => &$r_nr) {
        if ($r_nr <= 2) { break; }       // 2個以下の場合は無視
        $top_chars[] = $char;
        $top_nrs[]   = $r_nr;

        if (count($top_chars) == 4) { break; } // トップ 4 まで
    }
    //print_r($top_chars);

    // ---- トップをレジスタに割り当てるコードを作成
    $top_reg = [];  // key: 値, value: その値に割り当てるレジスタ
    $ret = '';
    $top_char0 = toByteHexString_($top_chars[0]);
    $top_char1 = toByteHexString_($top_chars[1]);
    $top_char2 = toByteHexString_($top_chars[2]);
    $top_char3 = toByteHexString_($top_chars[3]);
    switch (count($top_chars)) {
        case 1: // C に割り当て
            $ret .= "ld      C,  0 + ($top_char0)\n";
            $top_reg[$top_chars[0]] = 'C';
            break;
        case 2: // B, C に割り当て
            $ret .= "ld      BC, 0 + ($top_char0 << 8) | ($top_char1)\n";
            $top_reg[$top_chars[0]] = 'B';
            $top_reg[$top_chars[1]] = 'C';
            break;
        case 3: // B, C, E に割り当て
            $ret .= "ld      BC, 0 + ($top_char0 << 8) | ($top_char1)\n";
            $ret .= "ld      E,  0 + ($top_char2)\n";
            $top_reg[$top_chars[0]] = 'B';
            $top_reg[$top_chars[1]] = 'C';
            $top_reg[$top_chars[2]] = 'E';
            break;
        case 4:// B, C, D, E に割り当て
            $ret .= "ld      BC, 0 + ($top_char0 << 8) | ($top_char1)\n";
            $ret .= "ld      DE, 0 + ($top_char2 << 8) | ($top_char3)\n";
            $top_reg[$top_chars[0]] = 'B';
            $top_reg[$top_chars[1]] = 'C';
            $top_reg[$top_chars[2]] = 'D';
            $top_reg[$top_chars[3]] = 'E';
    }
    //print_r($top_chars);
    //print_r($top_reg);

    // ---- ヒストグラム情報を出力
    $ret .= sprintf("// Value histogram:\n");
    foreach ($hist as $char => &$r_nr) {
        if ($r_nr == 0) { break; }
        $ret .= sprintf("//   %s x %d%s\n",  toByteHexString_($char), $r_nr,
            isset($top_reg[$char]) ? (' ... Assigned to reg ' . $top_reg[$char]) : '');
    }
    if ($nr_masks != 0) {
        $ret .= sprintf("#define CG_{$name_upper}_MASK " .
            (isset($top_reg[$VATB_BG_MASK]) ? $top_reg[$VATB_BG_MASK] : $VATB_BG_MASK) .
            "\n");
    }
    $ret .= sprintf("\n");

    // -------- 描画コードを出力
    // 偶数行は右へ奇数行は左へ行ったり来たりスキャンで描画します
    $x_end = 0;
    for ($iy = 0; $iy < $h; $iy++) {
        $ret .= sprintf("// TEXT line $iy\n");

        // 最初の x 位置と方向
        if ($iy & 1) {
            $text = $data_text[$iy * $w + $w - 1];
            $ix = (0 <= $text) ? ($w - 1) : ($w - 1 + $text); // 負の値ならスキップ
            $x_step = -1;
        } else {
            $text = $data_text[$iy * $w];
            $ix = (0 <= $text) ? 0 : -$text;    // 負の値ならスキップ
            $x_step = 1;
        }

        // 最初の行なら SET, 改行なら MOVE マクロを設定します
        if ($iy == 0) {
            $ret .= sprintf("CG_{$name_upper}_SET_VVRAM_ADDR $ix, 0/*x,y*/, H, L, HL\n");
        } else {
            $ret .= sprintf("CG_{$name_upper}_MOVE_VVRAM_ADDR " . ($ix - $x_end - $x_step) . ", 1/*dx,dy*/, $ix, $iy/*x,y*/, H, L, HL\n");
        }
        //echo("x move: $x_end -> $ix\n");

        // 最後の x 位置
        if ($iy & 1) {
            $x_end  = -1;
        } else {
            $x_end  = $w;
        }

        $x_skip  = 0;
        $b_first = true;
        for (; $ix != $x_end; $ix += $x_step) {
            $idx = $iy * $w + $ix;
            $text = $data_text[$idx];
            $atb  = $data_atb[$idx];
            $cmd  = $data_cmd[$idx];

            if ($text < 0) {               // 末端部
                $x_end = $ix - $x_step;
                break;
            }

            if ($text == 0 && $cmd == '') {  // 透明スキップ
                $x_skip += $x_step;
                continue;
            }

            if (!$b_first) {   // 最初は増減しない
                $ret .= sprintf("CG_{$name_upper}_MOVE_VVRAM_ADDR $x_skip, 0/*dx,dy*/, $ix, $iy/*x,y*/, H, L, HL\n");
            }
            $b_first = false;
            $x_skip = $x_step;

            if ($b_text) {
                $char     = toByteHexString_($text);
                $char_opt = isset($top_reg[$text]) ? $top_reg[$text] : $char;
            } else {
                $char     = toByteHexString_($atb);
                $char_opt = isset($top_reg[$atb]) ? $top_reg[$atb] : $char;
            }
            $mask_opt = isset($top_reg[$VATB_BG_MASK]) ? $top_reg[$VATB_BG_MASK] : toByteHexString_($VATB_BG_MASK);

            switch ($cmd) {
                case '':
                    $ret .= sprintf("ld      (HL), $char_opt // ($ix, $iy)=$char\n");
                    break;
                case 'draw':
                    $ret .= sprintf("CG_{$name_upper}_DRAW_{$ix}_{$iy} HL, $char/*char*/ // ($ix, $iy)=$char\n");
                    break;
                case 'drawOpt':
                    $ret .= sprintf("CG_{$name_upper}_DRAW_{$ix}_{$iy} HL, $char_opt/*char*/ // ($ix, $iy)=$char\n");
                    break;
                case 'composeAtbFgBg':
                    $ret .=
                    "ld      A,  (HL)\n" .
                        "and     A,  $mask_opt // mask=". toByteHexString_($VATB_BG_MASK) . "\n" .
                        "or      A,  $char_opt // atb=" . toByteHexString_($atb) . "\n" .
                        "ld      (HL),  A // ($ix, $iy)\n";
                    break;
                case 'composeAtbBgBg':
                    $ret .=
                        "ld      A,  $char_opt // atb=" . toByteHexString_($atb) . "\n" .
                        "rld     // ($ix, $iy)\n";
                    break;
            }
        }
    }
    return $ret;
}

function toByteHexString_(int $val): string {
    return sprintf('0x%02x', $val & 0xff);
}