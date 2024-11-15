# メモリ汚しツール / Memory Dirtinizer

* MZ-700 の 64K RAM の大半を適当な値で埋めるツールです
* BSS セクションを初期化しないで使う場合のトラブルの事前チェックになります
* 0x76 (HALT命令) で埋めるのでプログラム暴走時のチェックにもなるかもしれません
* ビルドは build.bat で行います