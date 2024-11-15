<?php

declare(strict_types = 1);
/**
 * rm -f のように, 指定ファイルを削除します. ワイルドカード(*)対応
 * Windwos のコマンドプロンプトだとディレクトリ記号に "/" を使うとスイッチとみなしてしまうので, 止む追えず作る.
 *
 * @author Snail Barbarian Macho (NWK) 2023.12.06
 */

// 引数チェック
if (count($argv) < 2)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " file1 file2 ...\n");
    exit(1);
}

for ($i = 1; $i < count($argv); $i++) {
    $files = $argv[$i];
    foreach (glob($files) as $file) {
        //echo("$file ");
        unlink($file);
    }
}
