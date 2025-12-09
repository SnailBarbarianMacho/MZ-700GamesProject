<?php

declare(strict_types = 1);
require_once('nwk-classes/aal/aal80-parser.class.php');
require_once('nwk-classes/utils/error.class.php');

/**
 * Z80 代数アセンブリ言語 (Algebraic Assembly Language) フィルタ
 *
 * - z88dk ソースのインライン アセンブラを代数表記で書けます
 * - 利点:
 *   - 直感的に読みやすい
 *   - 1行に複文書けるのでソースが短くなる
 *   - VScodeのC/C++拡張の文法チェックを通る
 *   - z88dk の代替命令(add BD, DE等が勝手に展開される)を通さない
 * - ソースはこんな感じで書きます:
 *
 * #include "../src-common/aal80.h"        ... C Intelli Sense を騙すためのコードが入ってます. 削除されます
 *                                              ... 関数は小文字, マクロは大文字, レジスタは大文字, フラグは小文字
 *
 * // マクロ定義
 * int BAZ(int val, int reg_x) __aal_macro __naked {
 *                                              ... 関数名は大文字にしてください. 型(int)はダミー. 呼び出し規約修飾子に __aal_macro, __naked が必要
 *                                              ... レジスタ名を引数にする場合は reg_[\w]* であること
 * #define M1(x, y) x+y                         ... C マクロはそのまま出力されます (複数行に渡るマクロもOK)
 *   AAL_DEF_VARS;                           ... 最初におまじないで入れておいてください
 *   AAL_IF(val == 1)                        ... マクロ if
 *     extern label1, label2;                   ... extern
 *     goto   label1;                           ... jp の代わりに掛けます
 *     A = 1; A += (2); A >>= 1;                ... 代入演算子
 *     HL = 12; regX = 12;
 *     HL += DE + c;                            ... 式の末端に '+c' があると, adc, sbc 命令になります
 *     A = mem[12*34]; mem[IX+1] = A;           ... メモリ アクセスには mem[] を使います
 *     A = port[M1(1, 0)]; port[1] = A;         ... ポート アクセスには port[] を使います
 *   AAL_ELSE()
 *     ++A; A--; ~A; -A;                        ... 単項演算子
 *   AAL_ENDIF()
 *   AAL_REPT(2)                             ... リピート(リピート時はラベルがローカルになりません. 手抜き)
 *     nop();                                   ... 全命令は関数の形に書きます
 *     and(A, 1); daa(A); rld(A, mem[HL]);      ... 通常のニーモニックで省略されるレジスタ A は, 省略できません
 *     ldi(); otir();                           ... ブロック転送は, 引数なし(手抜き)
 *     push(HL, DE, BC);                        ... push, pop は引数は1以上可変長
 *     ld(A, mem[HL]); in(A, port[0x00]);       ... メモリやポートに対しては mem[], port[] が必要
 *   AAL_ENDR()
 *     if (c) A = 1;                            ... if ～ else if ～ else も対応してます.
 *     if (c_jr) A = 1; else A = 2;             ... if のフラグに '_jr' を付けると相対ジャンプになります
 *     if (c_jr_else_jr) A = 1; else A = 2;     ... if のフラグに '_else_jr' を付けると, else 先のジャンプも相対ジャンプになります
 *                                                  if - else がネストする場合もジャンプ命令は最適化されてます
 *     if (z) jp foo; 　　　　     　　　       ... jp z, foo
 *                                                  if 節が goto, jp(), jr() 単文ならば, 最適化します.
 *                                                  else 節のジャンプ命令は無くなります
 *     if (z_jr) jp foo;                        ... jr z, foo    (式) が相対ジャンプなら, jp は jr になります
 *     if (z) jr foo;                           ... jr z, foo
 *     if (m) jr foo;      // bad!              ... aal は通りますがコンパイル時にエラー
 *     if (z) call foo;                         ... call/return も同じように最適化
 *     if (z_jr) ret();    // bad!              ... ただし, (式) には相対ジャンプは使えません
 *     do A++; while (c_jr);                    ... do - while. カッコ内は, コンディションコードの他, true, true_jr, false, B-- にも対応してます
 *     do { if (c) break; continue; } while(c); ... break, continue に対応してます
 *     while(c) A++;                            ... while. カッコ内は, コンディションコードの他, true にも対応してます
 *                                                  ループ末端は jp 命令でループ先頭に戻ります (jr 命令はありません)
 *     {  }                                     ... 複文を, { ... } で囲むことができます. 囲んだ部分は出力コードでインデントが付きます
 * }
 *
 * // アセンブラ関数の定義
 * static int foo(int arg1, int*arg2) __aal __z88dk_fastcall __naked
 * {
 *                                              ... 呼び出し規約修飾子に __aal を付けます. __z88dk_fastcall や __naked はオプション
 *   AAL_DEF_VARS
 *   BAZ(1, HL);                                ... マクロの呼び出し.
 *                                                  ※引数がカッコから始まると z88dk のマクロアセンブラが引数の数がおかしいと言います
 *   ...
 *   AAL_NO_RETURN;                          ... __naked 関数の末端には, これか AAL_FALL_THROUGH を書いてください (ダミー ディレクティブですが, コード可読性を上げます) *
 * }
 *
 * - 使用できるレジスタ: A B C D E H L I R IXH IXL IYH IYL AF BC DE HL PC SP IX IY XH XL YH YL
 *                       A_ B_ C_ D_ E_ H_ L_ AF_ BC_ DE_ HL_ (裏レジスタ)
 * - if () 式で使えるフラグ: ※4
 *   - if も else も絶対ジャンプ: z,    eq,    nz,    ne,    c,    lt,    nc,    ge,   p, m, v, nv, pe, po
 *   - if は相対, else は絶対:    z_jr, eq_jr, nz_jr, ne_jr, c_jr, lt_jr, nc_jr, ge_jr
 *   - if は絶対, else は相対:    z_else_jr, eq_else_jr, nz_else_jr, ne_else_jr, c_else_jr, lt_else_jr, nc_else_jr, ge_else_jr, p_else_jr, m_else_jr, v_else_jr, nv_else_jr, pe_else_jr, po_else_jr
 *   - if も else も相対ジャンプ: z_jr_else_jr, eq_jr_else_jr, nz_jr_else_jr, ne_jr_else_jr, c_jr_else_jr, lt_jr_else_jr, nc_jr_else_jr, ge_jr_else_jr
 * - 代入演算子: =(ld, in, out), +=(add, adc), -=(sub, sbc), &=(and), |=(or), ^=(xor), <<=(srl_n), >>=(sla_n)
 * - 単項演算子: ++(inc), --(dec)
 * - 疑似命令と引数一覧:
 *   AAL_DECL_VARS
 *   AAL_LOCAL(expr, ...)
 *   AAL_DB(expr, ...) , AAL_DW(expr, ...)
 *   AAL_IF(expr), AAL_ELIF(expr), AAL_ELSE, AAL_ENDIF
 *   AAL_REPT(expr), AAL_REPTI(var, expr), AAL_ENDR
 *   AAL_GLOBAL(...)
 * - 命令(関数)と引数一覧:
 *   ld(a, b)※1, ldi(), ldir(), lddr()
 *   ex(a, b), exx()
 *   push(a, ...), pop(a, ...)
 *   and(a, b), or(a, b), xor(a, b), cpl(a), not(a)
 *   cp(), cpi(), cpd(), cpir(), cpdr()
 *   add(a, b), adc(a, b), sub(a, b), sbc(a, b), inc(a), dec(a), neg(a), daa(a)
 *   bit(a, b), set(a, b), res(a, b),
 *   bit_3(a, b, c)※4, set_3(a, b, c)※4, res_3(a, b, c)※4
 *
 *   sla(r)※1, sla_n(r, n)※2, sla_2(i, r)※3
 *   sll(r)※1, sll_n(r, n)※2, sll_2(i, r)※3
 *   sra(a)※1, sra_n(r, n)※2, sla_2(i, r)※3
 *   srl(a)※1, srl_n(r, n)※2, srl_2(i, r)※3
 *   rl(r)※1,  rl_n(r, n)※2,  rl_2(i, r)※3,  rla(A),  rla_n(A, n)※2
 *   rr(r)※1,  rr_n(r, n)※2,  rr_2(i, r)※3,  rra(A),  rra_n(A, n)※2
 *   rlc(r), rlc_n(r, n)※2, rlc_2(i, r)※3, rlca(A), rlca_n(A, n)※2
 *   rrc(r), rrc_n(r, n)※2, rrc_2(i, r),※3 rrca(A), rrca_n(A, n)※2
 *   rld(A, a), rrd(A, a)
 *
 *   jp(a), jp_z(a), jp_eq(a)※4, jp_nz(a), jp_ne(a)※4, jp_c(a), jp_lt(a)※4, jp_nc(a), jp_ge(a)※4,
 *   jp_p(a), jp_m(a), jp_v(a), jp_nv(a), jp_pe(a), jp_po(a)
 *   jr(a), jr_z(a), jr_eq(a)※4, jr_nz(a), jr_ne(a)※4, jr_c(a), jr_lt(a)※4, jr_nc(a), jr_ge(a)※4, djnz(B, a)
 *
 *   call(a), call_z(a), call_eq(a)※4, call_nz(a), call_ne(a)※4, call_c(a), call_lt(a)※4, call_nc(a), call_ge(a)※4
 *   call_p(a), call_m(a), call_v(a), call_nv(a), call_pe(a), call_po(a)
 *   ret(), ret_z(), ret_eq()※4, ret_nz(), ret_ne()※4, ret_c(), ret_lt(a), ret_nc(), ret_ge()※4,
 *   ret_p(), ret_m(), ret_v(), ret_nv(), ret_pe(), ret_po()
 *   return ※引数無しの場合のみ
 *   rst(n), reti(), retn()
 *   nop(), halt(), di(), ei(), im0(), im1(), im2(), scr(), ccf()
 *   in(a, b), out(a, b), ini(), ind(), inir(), indr(), outi(), outd(), otir(), otdr()
 *
 * ※1 引数に 16bit レジスタが指定できます. 例:HL = BC; srl(HL);
 * ※2 n 回繰り返します(nが数値の場合は4迄)
 * ※3 未定義命令の op b, (IX+d), r
 * ※4 符号なし整数でのシフト/比較. eq(==)は z, ne(!=)は nz, lt(<)は c, ge(>=)は nc と同じ
 *
 * - 小文字を含む関数は上記以外はエラーです.
 *   大文字の関数はマクロ呼び出しに展開されます
 * - 命令のレジスタの組み合わせは z88dk 任せ
 * - コメントは無視します (手抜き:文字列内のコメント記号やエスケープされたコメント記号の判定はしてません)
 * - 既知のバグ:
 *   - 関数検出が投げやりなので, 関数定義の前後には空白行を入れてください
 * - 使い方は, Usage: 行を参照してください
 *   例: php aal80.php foo.aal.c foo.c
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

// aal80.h を含む行を削除
$in_str = preg_replace('/\#include \ + \"[\ \S]*aal80\.h\"/x', "", $in_str);

// エスケープ処理
$in_str = escape_($in_str);

$parser = new nwk\aal\Parser($error);

// ---------------- 修飾子 __aal または __aal_macro を含む関数
// MARK: 処理中心
$out_str = detectFunctionCallback_($in_str, function(
    $match, $match_pos, $ret_type, $funcname,  $args, $modifiers, $contents, $contents_pos) use (&$in_str, $parser, $error) {
        //echo("[$funcname($args)\n");
        //echo("[$ret_type $funcname($args) $modifiers {$contents}]\n");
        $has_naked = str_contains($modifiers, '__naked');

        if (str_contains($modifiers, '__aal_macro')) {
            $line_nr  = substr_count($in_str, "\n", 0, $contents_pos);
            if (!$has_naked) {
                $error->errorLine($line_nr, "マクロの場合は, 呼び出し規約修飾子に '__naked' を追加してください", $funcname);
            }

            $contents = $parser->parse(nwk\aal\Parser::MODE_MACRO, $has_naked, $contents, $line_nr, $funcname);

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
            $modifiers = str_replace('__aal_macro', '', $modifiers);

            return $ret_type . "$funcname($args)$modifiers" .
                "{\n" .
                "__asm\n" .
                "$funcname macro $macro_args\n" .
                $labels_str .
                $contents .
                "endm\n" .
                "__endasm;\n" .
                "}\n";
            $line_nr = substr_count($in_str, "\n", 0, $match_pos);
            return "// line " . $line_nr + 1 . "\n" . $match;
        } else if (str_contains($modifiers, '__aal')) {
            $line_nr = substr_count($in_str, "\n", 0, $contents_pos);

            $contents = $parser->parse(nwk\aal\Parser::MODE_FUNC, $has_naked, $contents, $line_nr, $funcname);

            $modifiers = str_replace('__aal', '', $modifiers);

            return $ret_type . "$funcname($args)$modifiers" .
                "{\n" .
                "__asm\n" .
                $contents .
                "__endasm;\n" .
                "}\n";
            $line_nr = substr_count($in_str, "\n", 0, $match_pos);
            return "// line " . $line_nr + 1 . "\n" . $match;
        } else {
            // __aal や aal_macro を含まないなら, 行番号も含まずそのまま出力
            return $match;
        }
    });

// 空行の削除
//$out_str = preg_replace('/\n\n/', '', $out_str);

// ---------------- 出力
$error->report();

$out_str = "/**** This file is made by $argv[0].  DO NOT MODIFY! ****/\n" . unescape_($out_str);
$result = file_put_contents($filename_out_c, $out_str);
if ($result == false) {
    $error->error("file open error[$filename_out_c]\n");
    exit(1);
}
exit(0);

// -------------------------------- 関数の抽出とコールバック
// MARK: detectFunctionCallback_()
/**
 * コールバック関数 $callback の引数は,
 *  "int xyz = 0;\nvoid foo(int xxx, int yyy) __z88dk_fastcall __naked { return 10; }" ならば,
 *  $match          マッチ全文          "int foo(int xxx, int yyy) __z88dk_fastcall __naked { return; }"
 *  $match_pos      その位置            13
 *  $ret_type       戻り型              "void"
 *  $funcname       関数名              "foo"
 *  $args           引数                "int xxx, int yyy"
 *  $modifiers      呼び出し規約修飾子  "__z88dk_fastcall __naked"
 *  $contents       関数の中身          "return;"
 *  $contents_pos   その位置            67
 */
function detectFunctionCallback_(string $str, callable $callback): string {
    $out_str = preg_replace_callback('/([\w]*[\s\w\*]*) ([\w]+) \\\\\(([\s\w\*\,\(\)]*)\\\\\) ([\s\w]*) \\\\\{([\s\S]*)\\\\\}/Ux',
        function($matches) use ($callback) {
            $match     = $matches[0][0]; $match_pos = $matches[0][1];
            $ret_type  = $matches[1][0];
            $funcname  = $matches[2][0];
            $args      = $matches[3][0];
            $modifiers = $matches[4][0];
            $contents  = $matches[5][0]; $contents_pos = $matches[5][1];
            return $callback($match, $match_pos, $ret_type, $funcname, $args, $modifiers, $contents, $contents_pos);
        }, $str, -1, $dummy, PREG_OFFSET_CAPTURE);
    return ($out_str);
}


// -------------------------------- エスケープ処理
// MARK: escape_()
/**
 * 正規表現で関数定義を検出したいので, 小細工を入れます.
 * - 一番外側の { ... } なので, それをエスケープする \{ ... \}
 * - 関数の引数は { ... } 外の一番外側の ( ... ) なので, それをエスケープする \( ... \) ※マクロはどうする?
 * - 処理後に元に戻す
 */
function escape_(string $str): string
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


// MARK: unescape_()
function unescape_(string $str): string
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
