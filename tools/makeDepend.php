<?php

declare(strict_types = 1);
/**
 * 手抜き依存関係リスト作成
 * - gcc -MM xx.c でもいいんですが,
 *   それだけの為に, いちいち gcc を入れるのが面倒なので, 独自の機能を追加して作りました
 * - cg    対応 (cg/ 以下のヘッダは, cgedit ファイルと依存関係を作成する)
 * - music 対応 (music/ 以下のヘッダは, MIDI ファイルと依存関係を作成する)
 * - text  対応 (text/ 以下のヘッダは, テキスト ファイルと依存関係を作成する)
 *
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2021.07.05
 */

// --------------------------------
// 引数チェック
if (count($argv) != 10)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " src_dir cg_dir music_dir text_dir obj_dir cgedit.txt cgedit.json obj_macro_name out_depends.mk\n");
    exit(1);
}
$srcDir     = $argv[1];
$cgDir      = $argv[2];
$musicDir   = $argv[3];
$textDir    = $argv[4];
$objDir     = $argv[5];
$cgEditText = $argv[6];
$cgEditJson = $argv[7];
$objMacroName = $argv[8];
$outFile    = $argv[9];
$outStr     = '# **** This file is made by '.$argv[0].'. DO NOT MODIFY ! ****'."\n";

if (substr($cgDir,    -1) != '/') { $cgDir    .= '/'; }
if (substr($musicDir, -1) != '/') { $musicDir .= '/'; }
if (substr($textDir,  -1) != '/') { $textDir  .= '/'; }
//echo("$cgDir $musicDir $textDir\n");

if (file_exists($cgEditText) === false) {
    fwrite(STDERR, "File not found[$cgEditText]\n");
    exit(1);
}
if (file_exists($cgEditJson) === false) {
    fwrite(STDERR, "File not found[$cgEditJson]\n");
    exit(1);
}


// --------------------------------
// 情報の収集
// src_dir の中を再帰的にサーチして c ソースを探します
$it = new RecursiveIteratorIterator(
    new RecursiveDirectoryIterator(
        $srcDir,
        FilesystemIterator::SKIP_DOTS |
        FilesystemIterator::KEY_AS_PATHNAME |
        FilesystemIterator::CURRENT_AS_FILEINFO
    ), RecursiveIteratorIterator::LEAVES_ONLY);
$cSources = array();
foreach ($it as $pathname => $info) {
    if (pathinfo($pathname, PATHINFO_EXTENSION) == 'c') {
        $pathname = str_replace('\\', '/', $pathname);
        $cSources[] = $pathname;
    }
}
//var_export($cSources);


// ファイルを開いて, #include 文をサーチ
// 見つかればリストにあげます.
// そのファイルも再帰的に #include 文をサーチします
// 重複ファイルもチェックします
// フォルダに /music/ が含まれる場合は
$oPathnames = '';
$cgPathnameList    = [];
$musicPathnameList = [];
$textPathnameList  = [];
foreach ($cSources as $cSource) {
    $pathnameList  = [];

    $oPathname  = $objDir . '/' . pathinfo($cSource, PATHINFO_FILENAME).'.o';
    $oPathnames = $oPathnames.' '.$oPathname;
    //echo("===>cソース:$cSource\n");
    check('.', $cSource, $pathnameList, $cgDir, $musicDir, $textDir, $cgPathnameList, $musicPathnameList, $textPathnameList);

    $outStr .= $oPathname.': ';
    foreach ($pathnameList as $pathname) {
        $outStr .= ' ' . $pathname;
    }
    $outStr .= "\n";
    //$outStr .= "\t@echo Compiling...[$<]\n";
    //$outStr .= "\t@$(CC) $(CFLAGS) -o $@ -c $<\n";
    //$outStr .= "\t@$(CC) $(CFLAGS) -o $@ -c ".$cSource."\n";
}

// --------------------------------
// cgEdit
$outStr .= "\n# cgedit\n";
{
    // 抽出テーブルに記載された名前が $cgPathnameList にあるかをチェック
    $extTab = json_decode(file_get_contents($cgEditJson), true);
    if ($extTab === null) {
        fwrite(STDERR, "JSON decode error[$cgEditJson]\n");
        exit(1);
    }
    $names = array_keys($extTab);
    foreach ($cgPathnameList as $cgPathname) {
        $matches = [];
        $name = pathinfo($cgPathname,  PATHINFO_FILENAME);
        if (!in_array($name, $names)) {
            fwrite(STDERR, "WARN: [$name] is not in [$cgEditJson]\n");
        }
    }

    $targets = '';
    foreach ($cgPathnameList as $pathname) {
        $targets .= $pathname . ' ';
    }
    $outStr .= $targets . ': ' . "$cgEditText $cgEditJson\n";
    $outStr .= "\t@echo Generating CG data...\n";
    $outStr .= "\t@$(PHP) ../tools/cgEdit2c.php $^\n";
}


// --------------------------------
// music
$outStr .= "\n# music\n";
foreach ($musicPathnameList as $pathname) {
    $dirname  = dirname($pathname);
    $basename = basename($pathname, '.h');
    $outStr .= $pathname . ': ' . $dirname . '/' . $basename . ".mid\n";
#    $outStr .= "\t@echo Generating music data...[$@]\n";
#    $outStr .= "\t@$(PHP) ../tools/midi2sd3mml.php $^ $@\n";
}

// --------------------------------
// text
$outStr .= "\n# text\n";
foreach ($textPathnameList as $pathname) {
    $dirname  = dirname($pathname);
    $basename = basename($pathname, '.h');
    $outStr .= $pathname . ': ' . $dirname . '/' . $basename . ".txt\n";
#    $outStr .= "\t@echo Generating text data...[$@]\n";
#    $outStr .= "\t@$(PHP) ../tools/text2c.php $^ $@\n";
}

// --------------------------------
// objs
$outStr .= "\n";
$outStr .= "$objMacroName := $oPathnames\n";

// --------------------------------

file_put_contents($outFile, $outStr);

// --------------------------------
/**
 * ファイルを開いて
 * #include "..." 文があれば標準出力
 * @param $pathname C または ヘッダ パス名('foo/bar/baz.c' など)
 * @param $pathnameList      [in][out]ヘッダ パス名のリスト.     重複防止用にも用います
 * @param $cgPathnameList    [in][out]cg 用ヘッダ パス名のリスト. 重複防止用にも用います
 * @param $musicPathnameList [in][out]music 用ヘッダ パス名のリスト. 重複防止用にも用います
 * @param $textPathnameList  [in][out]text 用ヘッダ パス名のリスト. 重複防止用にも用います*
 */
function check(
    string $orgPathname,
    string $pathname,
    array &$pathnameList,
    string $cgDir,
    string $musicDir,
    string $textDir,
    array &$cgPathnameList,
    array &$musicPathnameList,
    array &$textPathnameList): void
{
    $bRecursive = true;

    $orgDir = pathinfo($orgPathname, PATHINFO_DIRNAME);
    $pathname = $orgDir . '/' . $pathname;
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

    if (strpos($pathname, $cgDir) === 0) {
        // cg/ パスが入ってるなら,それを登録(再帰はしない)
        if (array_search($pathname, $cgPathnameList, true) === false) {
            $cgPathnameList[] = $pathname;
        }
        $bRecursive = false;
    } else if (strpos($pathname, $musicDir) === 0) {
        // music/ パスが入ってるなら,それを登録(再帰はしない)
        if (array_search($pathname, $musicPathnameList, true) === false) {
            $musicPathnameList[] = $pathname;
        }
        $bRecursive = false;
    } else if (strpos($pathname, $textDir) === 0) {
        // text/ パスが入ってるなら,それを登録(再帰はしない)
        if (array_search($pathname, $textPathnameList, true) === false) {
            $textPathnameList[] = $pathname;
        }
        $bRecursive = false;
    }

    // 今までにないファイルならばリスト登録
    if (array_search($pathname, $pathnameList, true) === false) {
        $pathnameList[] = $pathname;
    }

    // ファイル存在チェック. 無くてもとりあえず出力(再帰しない)
    if (file_exists($pathname) === false)
    {
        fwrite(STDERR, "WARN: File not found[$pathname] in [$orgPathname]\n");
        return;
    }

    //echo '['.$pathname."]\n";
    $file = fopen($pathname, "r");

    if (!$file) {
        fwrite(STDERR, "WARN: File open error[$pathname] in [$orgPathname]\n");
        return;
    }

    if ($bRecursive === false) {    // 再帰しない
        return;
    }

    while ($line = fgets($file)) {
        // #include "～" を探す
        $matches = [];
        if (preg_match('/^#include\s*"([^"]+\.h)"/', $line, $matches) == 1) {
            check($pathname, $matches[1], $pathnameList, $cgDir, $musicDir, $textDir, $cgPathnameList, $musicPathnameList, $textPathnameList);
        }
    }

}
