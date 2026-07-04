<?php

declare(strict_types = 1);
/**
 * cp のように, ファイルをコピーします. ワイルドカード使ったらダメ(無保証)
 * Windwos のコマンドプロンプトだとディレクトリ記号に "/" を使うとスイッチとみなしてしまうので, 止む追えず作る.
 *
 * @author Snail Barbarian Macho (NWK)
 */

// 引数チェック
if (count($argv) != 3)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " file-src file-dst\n");
    exit(1);
}

$file_src = $argv[1];
$file_dst = $argv[2];
copy($file_src, $file_dst);
