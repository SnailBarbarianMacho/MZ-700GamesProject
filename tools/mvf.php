<?php

declare(strict_types = 1);
/**
 * mv -f のように, 指定ファイル強制リネームします
 * Windwos のコマンドプロンプトだとディレクトリ記号に "/" を使うとスイッチとみなしてしまうので, 止む追えず作る.
 *
 * @author Snail Barbarian Macho (NWK) 2026.07.29
 */

// 引数チェック
if (count($argv) !== 3)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " file1 file2\n");
    exit(1);
}

rename($argv[1], $argv[2]);
