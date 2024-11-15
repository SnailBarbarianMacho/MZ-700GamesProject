<?php

declare(strict_types = 1);
/**
 * 再帰的に .c, .h, .asm を探して, 行数を数えます
 *
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2023.17.05
 */

// --------------------------------
// 引数チェック
if (count($argv) != 2)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " dir");
    exit(1);
}
$dir = $argv[1];
if (!is_dir($dir)) {
    fwrite(STDERR, "[$dir] is not directory.\n");
    exit(1);
}


// --------------------------------
// 情報の収集
// src_dir の中を再帰的にサーチして c ソースを探します
$it = new RecursiveIteratorIterator(
    new RecursiveDirectoryIterator(
        $dir,
        FilesystemIterator::SKIP_DOTS |
        FilesystemIterator::KEY_AS_PATHNAME |
        FilesystemIterator::CURRENT_AS_FILEINFO
    ), RecursiveIteratorIterator::LEAVES_ONLY);
$line_ct = 0;
$file_ct = 0;
foreach ($it as $pathname => $info) {
    $ext = pathinfo($pathname, PATHINFO_EXTENSION);
    if ($ext == 'c' || $ext == 'h' || $ext == 'asm') {
        $pathname = str_replace('\\', '/', $pathname);
        $fp = fopen($pathname, 'r');
        $ct = 0;
        for (; fgets($fp); $ct++) {};
        echo("[$pathname] $ct lines\n");
        if ($ct) {
            $line_ct += $ct;
            $file_ct++;
        }
    }
}
echo("$file_ct files, $line_ct lines. average=" . ($line_ct / $file_ct) . "\n");
