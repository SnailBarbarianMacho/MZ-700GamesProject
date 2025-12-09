<?php

declare(strict_types = 1);
#require_once 'nwk-classes/utils/utils.class.php';
#require_once 'nwk-classes/utils/error.class.php';

/**
 * - --prog は ソース(.aal.c, .c)からオブジェクト(.o)を作る依存リストと,
 *   オブジェクトのリストと,
 *   clean 時の削除リストを作成
 *   例: game/src/ OBJS CLEAN_FILES
 *   game/system/a.c: game/system/a.aal.c game/system/sys.h
 *   game/system/b.c: game/system/b.aal.c game/system/sys.h game/cg/c.txt game/text/t.txt
 *   game/system/c.c: game/system/c.aal.c game/system/sys.h
 *   game/system/a.o:  game/system/a.c.
 *   game/system/b.o:  game/system/b.c
 *   game/system/c.o:  game/system/c.c
 *   OBJS := game/system/a.o game/system/b.o
 *   CLEAN_FILES := game/obj/a.o game/obj/b.o game/obj/c.o game/src/system/c.c
 *
 * - --cg は各 cgedit データ(.cgedit.txt, .cgedit.json) から複数のヘッダ(.h)を作る依存リストを作成
 *   例: -cgedit game/cg/ CG_FUNC
 *   game/cg/a.h game/cg/b.h: game/cg/a.cgedit.txt game/cg/a.cgedit.json
 *      $(call CG_FUNC)
 *   game/cg/c.h game/cg/d.h: game/cg/b.cgedit.txt game/cg/b.cgedit.json
 *      $(call CG_FUNC)
 *
 * - --music は各楽譜(.mid)からヘッダ(.h)を作る依存リストを作成
 *   例: -music game/music/
 *   game/music/test.h: game/music/test.mid
 *   game/music/foo.h: game/music/foo.mid
 *
 * - --text は各テキスト(.txt)からヘッダ(.h)を作る依存リストを作成
 *   例: -text game/text/
 *   game/text/branch-ahead.h: game/text/branch-ahead.txt
 *   game/text/game-over.h: game/text/game-over.txt
 *
 * - ファイル名では, アンダースコアはすべてハイフンになります(kebab-case)
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2021.07.05
 */

// --------------------------------
// 引数チェック
$out_file     = '';
$prog_dir     = '';
$objs_macro   = '';
$clean_files_macro = '';
$cg_dir       = '';
$cg_func      = '';
$music_dir    = '';
$text_dir     = '';
$storage_dir  = '';
$stags_macro  = '';

if (count($argv) <= 2) { usage_($argv); }
$out_file = $argv[1];
for ($i = 2; $i < count($argv); $i++) {
    switch ($argv[$i]) {
        case '--prog':
            if (count($argv) <= $i + 3) { usage_($argv); }
            $prog_dir          = $argv[$i + 1];
            $objs_macro        = $argv[$i + 2];
            $clean_files_macro = $argv[$i + 3];
            if (substr($prog_dir, -1) != '/') { $prog_dir .= '/'; }
            $i += 3;
            break;
        case '--cg':
            if (count($argv) <= $i + 2) { usage_($argv); }
            $cg_dir  = $argv[$i + 1];
            $cg_func = $argv[$i + 2];
            if (substr($cg_dir, -1) != '/') { $cg_dir .= '/'; }
            $i += 2;
            break;
        case '--music':
            if (count($argv) <= $i + 1) { usage_($argv); }
            $music_dir = $argv[$i + 1];
            if (substr($music_dir, -1) != '/') { $music_dir .= '/'; }
            $i += 1;
            break;
        case '--text':
            if (count($argv) <= $i + 1) { usage_($argv); }
            $text_dir = $argv[$i + 1];
            if (substr($text_dir, -1) != '/') { $text_dir .= '/'; }
            $i += 1;
            break;
        default: usage_($argv);
    }
}

$out_str = '# **** This file is made by '.$argv[0].'. DO NOT MODIFY ! ****'."\n";

// ---------------------------------------------------------------- prog
if ($prog_dir !== '') {
    $out_str .= "\n# prog\n";

    // ソース情報の収集
    // $prog_dir の中を再帰的にサーチして c ソースを探します
    $it = createRecursiveDirectoryIterator_($prog_dir);
    $c_sources = [];
    $aal_c_filenames = [];
    foreach ($it as $pathname => $info) {
        if (pathinfo($pathname, PATHINFO_EXTENSION) == 'c') {
            $pathname = adjustPathname_($pathname);
            $c_sources[] = $pathname;
            if (str_ends_with($pathname, '.aal.c')) {
                $aal_c_filenames[] = pathinfo($pathname, PATHINFO_FILENAME);
            }
        }
    }
    //var_export($aal_c_filenames);
    //var_export($c_sources);

    // ファイルを開いて, #include 文をサーチ
    // 見つかればリストに追加します.
    // そのファイルも再帰的に #include 文をサーチします
    // ファイルは絶対パスで管理し, 重複ファイルもチェックします
    // *.c と同名の *.aal.c があれば, 後者用の依存リストを追加します
    $o_pathnames = [];
    $o_clean_pathnames = [];
    foreach ($c_sources as $c_source) {

        if (str_ends_with($c_source, '.aal.c')) {
            $c_source2 = substr($c_source, 0, -6) . '.c';   // .aal を抜いたファイル
            $o_clean_pathnames[] = $c_source2;
            $pathname_list  = [];
            //echo("===>cソース:$c_source\n");
            check_('.', 0, $c_source, $pathname_list);
            $out_str .= $c_source2 . ': ' . implode(' ', $pathname_list) . "\n";

            $filename2 = pathinfo($c_source2, PATHINFO_FILENAME);
            $o_pathname  = $prog_dir . $filename2 . '.o';
            $o_pathnames[]       = $o_pathname;
            $o_clean_pathnames[] = $o_pathname;
            $out_str .= "$o_pathname: $c_source2\n";
            continue;
        }

        $filename = pathinfo($c_source, PATHINFO_FILENAME);
        if (in_array($filename . '.aal', $aal_c_filenames, true)) {
            continue;
        }
        $pathname_list  = [];
        $o_pathname  = $prog_dir . $filename . '.o';
        $o_pathnames[]       = $o_pathname;
        $o_clean_pathnames[] = $o_pathname;

        //echo("===>cソース:$c_source\n");
        check_('.', 0, $c_source, $pathname_list);

        $out_str .= $o_pathname . ': ' . implode(' ', $pathname_list) . "\n";
    }
    $out_str .= "$objs_macro := " . implode(' ', $o_pathnames) . "\n";
    $out_str .= "$clean_files_macro := " . implode(' ', $o_clean_pathnames) . "\n";
}

// ---------------------------------------------------------------- cgedit
if ($cg_dir !== '') {
    $out_str .= "\n# cg\n";
    //$cg_headers = [];

    // $cg_dir 以下を, 再帰的に .gedit.txt, .cgedit.json を探します
    $it = createRecursiveDirectoryIterator_($cg_dir);
    foreach ($it as $pathname => $info) {
        if (str_ends_with($pathname, '.cgedit.txt')) {
            $pathname = adjustPathname_($pathname);

            // 同名の cgedit.json があるかチェック
            $pathname_json = str_replace('.cgedit.txt', '.cgedit.json', $pathname);
            if (!file_exists($pathname_json)) {
                fwrite(STDERR, "cgedit JSON file is not exist[$pathname_json]\n");
                exit(1);
            }

            // あれば解析して記載された名前をヘッダとして出力
            $ext_tab = json_decode(file_get_contents($pathname_json), true);
            if ($ext_tab === null) {
                fwrite(STDERR, "JSON decode error[$pathname_json]\n");
                exit(1);
            }
            $names = array_keys($ext_tab);
            foreach ($names as &$name) {
                $name = str_replace('_', '-', $name);
                $name = "$cg_dir$name.h";
            }

            $out_str .= implode(' ', $names) . ': ' . $pathname . ' ' . $pathname_json. "\n";
            $out_str .= "\t$(call $cg_func)\n";
            //$cg_headers = array_merge($cg_headers, $names);
        }
    }
    //$out_str .= "$cgs_macro:= " . implode(' ', $cg_headers) . "\n";
}

// ---------------------------------------------------------------- music
if ($music_dir !== '') {
    $out_str .= "\n# music\n";

    // $music_dir 以下を, 再帰的に .mid を探します
    $it = createRecursiveDirectoryIterator_($music_dir);
    foreach ($it as $pathname => $info) {
        if (str_ends_with($pathname, '.mid')) {
            $pathname = adjustPathname_($pathname);
            $pathname_h = str_replace('.mid', '.h', $pathname);
            $out_str .= $pathname_h . ': ' . $pathname . "\n";
        }
    }
}

// ---------------------------------------------------------------- text
if ($text_dir !== '') {
    $out_str .= "\n# text\n";

    // $text_dir 以下を, 再帰的に .txt を探します
    $it = createRecursiveDirectoryIterator_($text_dir);
    foreach ($it as $pathname => $info) {
        if (str_ends_with($pathname, '.txt')) {
            $pathname = adjustPathname_($pathname);
            $pathname_h = str_replace('.txt', '.h', $pathname);
            $out_str .= $pathname_h . ': ' . $pathname . "\n";
        }
    }
}

// ---------------------------------------------------------------- storage
if ($storage_dir !== '') {
    $out_str .= "\n# storage\n";

    // $storage_dir 以下を, 再帰的に .c を探します
    $it = createRecursiveDirectoryIterator_($storage_dir);
    $zx0_files = [];
    foreach ($it as $pathname => $info) {
        if (str_ends_with($pathname, '.c')) {
            $pathname = adjustPathname_($pathname);

            $pathname_list = [];
            check_('.', 0, $pathname, $pathname_list);

            $zx0_file = str_replace('.c', '.zx0', $pathname);
            $zx0_files[] = $zx0_file;
            $out_str .= $zx0_file . ':';
            foreach ($pathname_list as $pathname) {
                $out_str .= ' ' . $pathname;
            }
            $out_str .= "\n";
        }
    }
    $out_str .= "$storage_macro := " . implode(' ', $zx0_files) . "\n";
}

// ---------------------------------------------------------------- 出力して終了
file_put_contents($out_file, $out_str);


// ---------------------------------------------------------------- subroutines
/**
 * ファイルを開いて
 * #include "..." 文があれば標準出力
 * @param $pathname            C または ヘッダ パス名('foo/bar/baz.c' など)
 * @param $pathname_list       [in][out]ヘッダ パス名のリスト.     重複防止用にも用います
 * @param $cg_pathname_list    [in][out]cg 用ヘッダ パス名のリスト. 重複防止用にも用います
 * @param $music_pathname_list [in][out]music 用ヘッダ パス名のリスト. 重複防止用にも用います
 * @param $text_pathname_list  [in][out]text 用ヘッダ パス名のリスト. 重複防止用にも用います
 */
function check_(
    string $org_pathname,
    int    $line_nr,
    string $pathname,
    array &$pathname_list): void
{
    $org_dir = pathinfo($org_pathname, PATHINFO_DIRNAME);
    $pathname = $org_dir . '/' . $pathname;
    $pathname = str_replace('\\', '/', $pathname); // '\'を'/' に変換
    //echo '開こうとするファイル['.$pathname."]\n";

    // pathname の最適化
    // hoge/../fuga を    fuga に変換
    // hoge/../../fuga を ../fuga に変換
    while (true) {
        $pathname2 = preg_replace('/[A-Za-z0-9_-]+\/\.\.\/(.+)$/', '$1', $pathname);
        if (strcmp($pathname, $pathname2) === 0) { break; }
        $pathname = $pathname2;
    }
    $pathname = preg_replace('/^\.\/(.+)$/', '$1', $pathname); // 先頭の "./" を削除
    //echo '開こうとするファイル(最適化後)['.$pathname."]\n";

    // 今までにないファイルならばリスト登録
    if (array_search($pathname, $pathname_list, true) === false) {
        $pathname_list[] = $pathname;
    }

    // ファイル存在チェック. 無くてもとりあえず出力(再帰しない)
    if (file_exists($pathname) === false)
    {
        fwrite(STDERR, "$org_pathname:$line_nr: Warning: $pathname: No such file\n");
        return;
    }

    //echo '['.$pathname."]\n";
    $file = fopen($pathname, "r");

    if (!$file) {
        fwrite(STDERR, "$org_pathname:$line_nr: Warning: $pathname: File open error\n");
        return;
    }

    $line_nr = 1;
    while ($line = fgets($file)) {
        // #include "～" を探す
        $matches = [];
        if (preg_match('/^#include\s*"([^"]+\.(h|txt))"/', $line, $matches) == 1) {
            check_($pathname, $line_nr, $matches[1], $pathname_list);
        }
        $line_nr++;
    }

}


/** 再帰ディレクトリ捜査イテレーターの生成 */
function createRecursiveDirectoryIterator_($dir): RecursiveIteratorIterator
{
    try {
        $it = new RecursiveIteratorIterator(
            new RecursiveDirectoryIterator(
                $dir,
                FilesystemIterator::SKIP_DOTS |
                FilesystemIterator::KEY_AS_PATHNAME |
                FilesystemIterator::CURRENT_AS_FILEINFO
            ), RecursiveIteratorIterator::LEAVES_ONLY);
        return $it;
    } catch (Exception $e) {
        fwrite(STDERR, "Directory not found[$dir]\n");
        exit(1);
    }
}


/** pathname を整える(ディレクトリ記号を統一するだけ) */
function adjustPathname_($pathname): string
{
    $pathname = str_replace("\\", '/', $pathname);
    $pathname = str_replace('//', '/', $pathname);
    return $pathname;
}


function usage_($argv): void
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " outfile [params...]\n" .
        "params are:\n" .
        "  --prog    prog_dir objs_macro_name clean_files_macro_name\n" .
        "  --cg      cg_dir cg_func\n" .
        "  --music   music_dir\n" .
        "  --text    text_dir\n");
    exit(1);
}
