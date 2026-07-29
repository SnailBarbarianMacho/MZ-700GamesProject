<?php

declare(strict_types = 1);
require_once('nwk-classes/aal/aal80-parser.class.php');
require_once('nwk-classes/utils/error.class.php');

/**
 * Z80 代数アセンブリ言語 (Algebraic Assembly Language) フィルタ
 *
 * - 使い方は, Usage: 行を参照してください
 *   例: php aal80.php foo.aal.c foo.c
 *
 * @author Snail Barbarian Macho (NWK) 2024.12.06
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

$parser = new nwk\aal\Parser($error);

// ---------------- 修飾子 __aal または __aal_macro を含む関数
// MARK: 処理中心
$out_str = detectFunctionCallback_($in_str, function(
    $match, $match_pos, $ret_type, $funcname,  $args, $modifiers, $contents, $contents_pos) use (&$in_str, $parser, $error) {
        //echo("[$funcname($args)\n");
        //echo("[$ret_type $funcname($args) $modifiers {$contents}]\n");
        $has_naked          = str_contains($modifiers, '__naked');
        $has_z88dk_fastcall = str_contains($modifiers, '__z88dk_fastcall');
        $has_sdcccall       = str_contains($modifiers, '__sdcccall');

        if (str_contains($modifiers, '__aal_macro')) {
            $line_nr  = substr_count($in_str, "\n", 0, $contents_pos);
            if (!$has_naked) {
                $error->errorLine($line_nr, "マクロは, 呼び出し規約修飾子に '__naked' を追加してください", $funcname);
            }
            if ($has_z88dk_fastcall || $has_sdcccall) {
                $error->errorLine($line_nr, "マクロは, 呼び出し規約修飾子 '__z88dk_fastcall', '__sdcccall' は使えません", $funcname);
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

            return $ret_type . " $funcname($args)$modifiers" .
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

            if ($has_z88dk_fastcall && $has_sdcccall) {
                $error->errorLine($line_nr, "関数は, 呼び出し規約修飾子 '__z88dk_fastcall', '__sdcccall' 同時使用はできません", $funcname);
            }

            $line_nr = substr_count($in_str, "\n", 0, $contents_pos);

            $contents = $parser->parse(nwk\aal\Parser::MODE_FUNC, $has_naked, $contents, $line_nr, $funcname);

            $modifiers = str_replace('__aal', '', $modifiers);

            return $ret_type . " $funcname($args)$modifiers" .
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

$out_str = "/**** This file is made by $argv[0].  DO NOT MODIFY! ****/\n" . $out_str;
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
    $pattern = '/' .
        '([\w\*]*[ \t\w\*]*[\s\*]+)' .  // 戻値(最初は空白除く)
        '([a-zA-Z0-9_]+)\s*' .          // 関数名
        '\(([\s\w\*\,]*?)\)\s*' .       // 引数リスト ... () 内のすべての文字(改行や複数引数対応)
        '([__aal\s+|__aal_macro\s+|__z88dk_fastcall\s+|__sdcccall\(1\)\s+|__naked\s+]*)\s*'. // 呼び出し規約識別子(__aal か __aal_macro 必須, あとはオプションだが条件ややこいので後でチェック)
        '(\{(?:[^{}]++|(?5))*\})' .     // 関数本体 ... {} のネストに対応 (?5)
        '/s';                           // '.' は改行にもマッチ
    //$pattern = '/([\w]*[\s\w\*]*) ([\w]+) \\\\\(([\s\w\*\,]*)\\\\\) ([__aal\s+|__aal_macro\s+|__z88dk_fastcall\s+|__sdcccall\s+|__naked\s+]*) \\\\\{([\s\S]*)\\\\\}/Ux';旧バージョン
    $out_str = preg_replace_callback($pattern,
        function($matches) use ($callback) {
            $match     = $matches[0][0]; $match_pos = $matches[0][1];
            $ret_type  = $matches[1][0];
            $funcname  = $matches[2][0];
            $args      = $matches[3][0];
            $modifiers = $matches[4][0];
            $contents  = $matches[5][0]; $contents_pos = $matches[5][1];
            //echo("ret:[$ret_type]\nfunc:[$funcname]\nargs:[$args]\nmod:[$modifiers]\n"); // 検出度合いのデバッグ
            return $callback($match, $match_pos, $ret_type, $funcname, $args, $modifiers, $contents, $contents_pos);
        }, $str, -1, $dummy, PREG_OFFSET_CAPTURE);
    return ($out_str);
}
