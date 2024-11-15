<?php

declare(strict_types = 1);
require_once('nwk-classes/z80ana/z80ana.class.php');
require_once('nwk-classes/utils/error.class.php');

/**
 * Z80 代数表記(algebraic notation)アセンブリ言語 簡易フィルタ
 *
 * - z88dk ソースのインライン アセンブラを代数表記で書けます
 * - 利点:
 *   - 直感的に読みやすい
 *   - 1行に複文書けるのでソースが短くなる
 *   - VScodeのC/C++拡張の文法チェックを通る
 *   - z88dk の代替命令(add BD, DE等が勝手に展開される)を通さない
 * - ソースはこんな感じで書きます:
 *
 * #include "../src-common/z80ana-def.h"        ... C Intelli Sense を騙すためのコードが入ってます. 削除されます
 *                                              ... 関数は小文字, マクロは大文字, レジスタは大文字, フラグは小文字
 *
 * // マクロ定義
 * int BAZ(int val, int reg_x) __z80ana_macro __naked {
 *                                              ... 関数名は大文字にしてください. 呼び出し規約修飾子に __z80ana_macro, __naked が必要
 *                                              ... レジスタ名を引数にする場合は reg_[\w]* であること
 * #define M1(x, y) x+y                         ... C マクロはそのまま出力されます (複数行に渡るマクロもOK)
 *   Z80ANA_DECL_VARS()                         ... 最初におまじないで入れておいてください
 *   Z80ANA_IF(val == 1)                        ... マクロ if
 *     extern label1, label2;                   ... extern
 *     goto   label1;                           ... jp の代わりに掛けます
 *     A = 1; A += (2); A >>= 1;                ... 代入演算子
 *     HL = 12; regX = 12;
 *     HL += DE + c;                            ... 式の末端に '+c' があると, adc, sbc 命令になります
 *     A = mem[12*34]; mem[IX+1] = A;           ... メモリ アクセスには mem[] を使います
 *     A = port[M1(1, 0)]; port[1] = A;         ... ポート アクセスには port[] を使います
 *   Z80ANA_ELSE()
 *     ++A; A--; ~A; -A;                        ... 単項演算子
 *   Z80ANA_ENDIF()
 *   Z80ANA_REPT(2)                             ... リピート(リピート時はラベルがローカルになりません. 手抜き)
 *     nop();                                   ... 全命令は関数の形に書きます
 *     and(A, 1); daa(A); rld(A, mem[HL]);      ... 通常のニーモニックで省略されるレジスタ A は, 省略できません
 *     ldi(); otir();                           ... ブロック転送は, 引数なし(手抜き)
 *     push(HL, DE, BC);                        ... push, pop は引数は1以上可変長
 *     ld(A, mem[HL]); in(A, port[0x00]);       ... メモリやポートに対しては mem[], port[] が必要
 *   Z80ANA_ENDR()
 *     if (c) bit(2, A);                        ... if ～ else if ～ else も対応してます.
 *     if (c_r) { ldd(); } else { B = 1; }      ... if のフラグ名に '_r' を付けると相対ジャンプになります
 *     if (z) { if (c) { B = 1; } else { B = 2; }} else { B = 3; }
 *                                              ... if ～ else のネストは最適化されないので注意(手抜き) 左のコードは:
 *                                                       jp nz, L03
 *                                                       jp nc, L01
 *                                                       ld B, 1
 *                                                       jp L02     ... L04 にはジャンプしない
 *                                                  L01: ld B, 2
*                                                   L02: jp L04
*                                                   L03: ld B, 3
                                                    L04:
 *     { ret(); }                               ... 複文を, { ... } で囲むことができます. 囲んだ部分は出力コードでインデントが付きます
 * }
 *
 * // アセンブラ関数の定義
 * static int foo(int arg1, int*arg2) __z80ana __z88dk_fastcall {
 *                                              ... 呼び出し規約修飾子に __z80ana を付けます
 *   Z80ANA_DECL_VARS
 *   BAZ(1, HL);                                ... マクロの呼び出し
 *   ...
 * }
 *
 * - 使用できるレジスタ:  IXH IXL IYH IYL HL DE BC AF PC SP IX IY XH XL YH YL A B C D E F H L I R
 * - if 式で使えるフラグ: false nz po pe nv nc z p m v c nz_r nc_r z_r c_r (※ _r が付いたフラグは, 相対ジャンプ)
 * - 代入演算子: =(ld, in, out), +=(add, adc), -=(sub, sbc), &=(and), |=(or), ^=(xor), <<=(srln), >>=(slan)
 * - 単項演算子: ++(inc), --(dec), ~(cpl), -(neg)
 * - 疑似命令と引数一覧:
 *   Z80ANA_DECL_VARS
 *   Z80ANA_LOCAL(expr, ...)
 *   Z80ANA_DB(expr, ...) , Z80ANA_DW(expr, ...)
 *   Z80ANA_IF(expr), Z80ANA_ELIF(expr), Z80ANA_ELSE, Z80ANA_ENDIF
 *   Z80ANA_REPT(expr), Z80ANA_ENDR
 * - 命令(関数)と引数一覧:
 *   ld(a, b), ldi(), ldir(), lddr()
 *   ex(a, b), exx()
 *   push(a, ...), pop(a, ...)
 *   and(a, b), or(a, b), xor(a, b), cpl(), not()
 *   cp(), cpi(), cpd(), cpir(), cpdr()
 *   add(a, b), adc(a, b), sub(a, b), sbc(a, b), inc(a), dec(a), neg(a), daa(a)
 *   bit(a, b), set(a, b), res(a, b),
 *   bit_3(a, b, c), set_3(a, b, c), res_3(a, b, c)
 *
 *   sla(r), sla_n(r, n), sla_2(i, r) ※ローテート/シフト命令の n は回数. 即値が入る場合は4迄
 *   sll(r), sll_n(r, n), sll_2(i, r)
 *   sra(a), sra_n(r, n), sla_2(i, r)
 *   srl(a), srl_n(r, n), srl_2(i, r)
 *   rl(r),  rl_n(r, n),  rl_2(i, r),  rla(A),  rla_n(A, n)
 *   rr(r),  rr_n(r, n),  rr_2(i, r),  rra(A),  rra_n(A, n)
 *   rlc(r), rlc_n(r, n), rlc_2(i, r), rlca(A), rlca_n(A, n)
 *   rrc(r), rrc_n(r, n), rrc_2(i, r), rrca(A), rrca_n(A, n)
 *   rld(A, a), rrd(A, a)
 *
 *   jp(a), jp_z(a), jp_nz(a), jp_c(a), jp_nc(a), jp_p(a), jp_m(a), jp_v(a), jp_nv(a), jp_pe(a), jp_po(a)
 *   jr(a), jr_z(a), jr_nz(a), jr_c(a), jr_nc(a), djnz(B, a)
 *   call(a), call_z(a), call_nz(a), call_c(a), call_nc(a), call_p(a), call_m(a), call_v(a), call_nv(a), call_pe(a), call_po(a)
 *   ret(),   ret_z(),   ret_nz(),   ret_c(),   ret_nc(),   ret_p(),   ret_m(),   ret_v(),   ret_nv(),   ret_pe(),   ret_po()
 *   rst(n), reti(), retn()
 *   nop(), halt(), di(), ei(), im0(), im1(), im2(), scr(), ccf()
 *   in(a, b), out(a, b), ini(), ind(), inir(), indr(), outi(), outd(), otir(), otdr()
 * - 小文字を含む関数は上記以外はエラーです.
 *   大文字の関数はマクロ呼び出しに展開されます
 * - 命令のレジスタの組み合わせは z88dk 任せ
 * - コメントは無視します (手抜き:文字列内のコメント記号やエスケープされたコメント記号の判定はしてません)
 * - 使い方は, Usage: 行を参照してください
 *   例: php z80ana.php foo.z80ana.c foo.c
 *
 * @author Snail Barbarian Macho (NWK) 2024.06.06
 */

// --------------------------------
$error = new nwk\utils\Error();

// 引数チェック
if (count($argv) != 3)
{
    $error->error('Usage: php ' . $argv[0] . " in.c out.c\n");
    exit(1);
}
$filename_in_c  = $argv[1];
$filename_out_c = $argv[2];
$error->setFilename($filename_in_c);

if ($filename_in_c == $filename_out_c) {
    $error->error("Filenames are the same\n");
    exit(1);
}

// ---------------- ファイルを読み込む
$in_str = file_get_contents($filename_in_c);
if ($in_str === false) {
    $error->error("file open error\n");
    exit(1);
}

// ---------------- 前処理

// チェック
if (preg_match('/\\\\ \(|\\\\ \)|\\\\ \{|\\\\ \}/x', $in_str) !== 0) {
    $error->error('ソースに, \'\(\' \'\)\' \'\{\' \'\}\'' . "を含まないでください\n");
    exit(1);
}

// 空白文字を統一
$in_str = str_replace(array("\r\n", "\r", "\n"), "\n", $in_str);
$in_str = str_replace(array(" ", "\t"), " ", $in_str);

// /* ... */ コメントを削除(改行を保ったまま)
$in_str = preg_replace_callback('/\/\* [\s\S]*? \*\//x', function($matches) {
    return preg_replace("/[^\n]/", "", $matches[0]);// 改行を除いて削除
}, $in_str);

// // ... コメントを削除
$in_str = preg_replace_callback('/\/\/ [\s\S]*?\n/x', function($matches) {
    return preg_replace("/[^\n]/", "", $matches[0]);// 改行を除いて削除
}, $in_str);

// z80ana-def.h を含む行を削除
$in_str = preg_replace('/\#include \ + \"[\ \S]*z80ana\-def\.h\"/x', "", $in_str);

// エスケープ処理
$in_str = escape($in_str);

$parser = new nwk\z80ana\Parser($error);

// ---------------- 修飾子 __z80ana または __z80ana_macro を含む関数
$out_str = preg_replace_callback('/([\w]*[\s\w\*]*) ([\w]+) \\\\\(([\s\w\*\,]*)\\\\\) ([\s\w]*) \\\\\{([\s\S]*)\\\\\}/Ux',
    function($matches) use (&$in_str, $parser, $error) {
        $ret_type = $matches[1][0];
        $funcname = $matches[2][0];
        $args     = $matches[3][0];
        $modifier = $matches[4][0];// 呼び出し規約修飾子
        $contents = $matches[5][0];
        //echo("[$args]\n");
        if (str_contains($modifier, '__z80ana_macro')) {
            $line_nr  = substr_count($in_str, "\n", 0, $matches[5][1]);

            if (!str_contains($modifier, '__naked')) {
                $error->errorLine($line_nr, "マクロの場合は, 呼び出し規約修飾子に '__naked' を追加してください", $matches[0][0]);
            }

            $contents = $parser->parse('macro', $contents, $line_nr, $funcname);

            $labels   = $parser->getLabels();
            $labels_str = '';
            if (count($labels) !== 0) {
                $labels_str = 'local ' . implode(', ', $labels) . "\n";
            }

            $arg_arr  = explode(',', $args);
            $macro_args = '';
            foreach($arg_arr as $i => $arg) {
                $arg = trim(str_replace("\n", ' ', $arg));
                //echo("$i [$arg]\n");
                $terms = explode(' ', $arg);
                if ($i !== 0) { $macro_args .= ', ';}
                $macro_args .= end($terms);
            }
            $modifier = str_replace('__z80ana_macro', '', $modifier);

            return $ret_type . "$funcname($args)$modifier" .
                "{\n" .
                "__asm\n" .
                "$funcname macro $macro_args\n" .
                $labels_str .
                $contents .
                "endm\n" .
                "__endasm;\n" .
                "}\n";
        } else if (str_contains($modifier, '__z80ana')) {
            $line_nr = substr_count($in_str, "\n", 0, $matches[5][1]);

            $contents = $parser->parse('func', $contents, $line_nr, $funcname);

            $modifier = str_replace('__z80ana', '', $modifier);

            return $ret_type . "$funcname($args)$modifier" .
                "{\n" .
                "__asm\n" .
                $contents .
                "__endasm;\n" .
                "}\n";
        }
        // __z80ana や z80ana_macro を含まないならそのまま出力
        $line_nr = substr_count($in_str, "\n", 0, $matches[0][1]);
        return "// line " . $line_nr + 1 . "\n" . $matches[0][0];
    }, $in_str, -1, $dummy, PREG_OFFSET_CAPTURE);

// 空行の削除
//$out_str = preg_replace('/\n\n/', '', $out_str);

// ---------------- 出力
$error->report();

$out_str = "/**** This file is made by $argv[0].  DO NOT MODIFY! ****/\n" . unescape($out_str);
$result = file_put_contents($filename_out_c, $out_str);
if ($result == false) {
    $error->error("file open error[$filename_out_c]\n");
    exit(1);
}
exit(0);


// -------------------------------- エスケープ処理
/**
 * 正規表現で関数定義を検出したいので, 小細工を入れます.
 * - 一番外側の { ... } なので, それをエスケープする \{ ... \}
 * - 関数の引数は { ... } 外の一番外側の ( ... ) なので, それをエスケープする \( ... \) ※マクロはどうする?
 * - 処理後に元に戻す
 */
function escape(string $str): string
{
    $paren_lv   = 0;
    $bracket_lv = 0;
    $chars = str_split($str);

    foreach ($chars as &$r_char) {
        switch ($r_char) {
            case '(':
                if ($bracket_lv === 0) {
                    if ($paren_lv === 0) {
                        $r_char = '\(';
                    }
                    $paren_lv ++;
                }
                break;

            case ')':
                if ($bracket_lv === 0) {
                    $paren_lv --;
                    if ($paren_lv === 0) {
                        $r_char = '\)';
                    }
                }
                break;

            case '{':
                if ($bracket_lv === 0) {
                    $r_char = '\{';
                }
                $bracket_lv ++;
                break;
            case '}':
                $bracket_lv --;
                if ($bracket_lv === 0) {
                    $r_char = '\}';
                }
                break;
        }
    }
    return implode('', $chars);
}


function unescape(string $str): string
{
    $chars = str_split($str);
    $b_escape = false;

    foreach ($chars as $i => &$r_char) {
        switch ($r_char) {
            case '\\':
                $b_escape = true;
                break;
            case '(':
            case ')':
            case '{':
            case '}':
                if ($b_escape) {
                    $chars[$i - 1] = '';
                    $b_escape = false;
                }
                break;
            default:
                $b_escape = false;
                break;
        }
    }
    return implode('', $chars);
}
