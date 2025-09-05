<?php

declare(strict_types = 1);
/**
 * バイナリデータをオブジェクト(code_compiler セクション) に変換します
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2025.07.25
 */


// --------------------------------

// 引数チェック
if (count($argv) !== 4)
{
    // シンボルは '_' を頭につけてください
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in.bin out.o _symbol\n");
    exit(1);
}

$bin_filename = $argv[1];
$obj_filename = $argv[2];
$symbol = $argv[3];

// ファイル存在チェック
if (file_exists($bin_filename) === false)
{
    fwrite(STDERR, 'File not found:' . $bin_filename . "\n");
    exit(1);
}

// ロード
$data = file_get_contents($bin_filename);
if ($data === false) {
    fwrite(STDERR, 'File load error:' . $bin_filename . "\n");
    exit(1);
}
$data_len = strlen($data);
$data_padded = str_pad($data, (($data_len + 3) & 0xfffffffc), pack('C', 0x00));
//printf("%04x -> %04x %d", $data_len, strlen($data_padded), (4-$data_len)&3);

// モジュール名セクション
$module_name = preg_replace('/[^A-Za-z0-9_]/', '_', basename($bin_filename));

$module_names_section =
    pack('V', 4);                           // モジュール名(strid)
    //pack('V', 0);                           // 終了マーカー

// 定義済シンボル セクション
$defined_symbols_section =
    pack('VVVVVVV', 2, 2, 1, 0, 2, 3, 1) .  // public, 再配置可能アドレス, セクション名(strid), アドレス, シンボル名(strid), ファイル名(strid), ソース行番号
    pack('V', 0);                           // 終了マーカー

// セクション
$sections =
    pack('VVVV', 0, 0, -1, 1) .             // 長さ, セクション名(strid), アドレス, 境界
    pack('VVVV', 0, 5, -1, 1) .             // 長さ, セクション名(strid), アドレス, 境界
    pack('VVVV', 0, 6, -1, 1) .             // 長さ, セクション名(strid), アドレス, 境界
    pack('VVVV', 0, 7, -1, 1) .             // 長さ, セクション名(strid), アドレス, 境界
    pack('VVVV', $data_len, 1, -1, 1) .     // 長さ, セクション名(strid), アドレス, 境界
    $data_padded .                          // データ本体(pad込み)
    pack('V', -1);                          // 終了マーカー

// 文字列テーブル
$bin_filename = str_replace('\\', '/', $bin_filename);
$strs = [
    '',
    'rodata_compiler',
    $symbol,
    $bin_filename,
    $module_name,
    'bss_compiler',
    'IGNORE',
    'code_crt_init'
];
$str_len = 0;
foreach($strs as $str) { $str_len += strlen($str) + 1; }
$str_len = ($str_len + 3) & 0xfffffffc;
$str_table = pack('VV', count($strs), $str_len); // 文字列数, 文字データ合計(padded)
$str_len = 0;
foreach($strs as $str) { $str_table .= pack('V', $str_len); $str_len += strlen($str) + 1; }
foreach($strs as $str) { $str_table .= $str . pack('C', 0x00); }
$str_table = str_pad($str_table, ((strlen($str_table) + 3) & 0xfffffffc), pack('C', 0x00));

// ヘッダ
$cpu_id = 1;                // CPU ID
$use_ixiy = 0;              // IXIY を使う
$sz_header = 0x28;
$defined_symbols_addr = $sz_header;  // 定義済シンボル アドレス
$module_names_addr    = $defined_symbols_addr + strlen($defined_symbols_section);   // モジュール名セクション アドレス
$sections_addr        = $module_names_addr    + strlen($module_names_section);      // セクション アドレス
$str_table_addr       = $sections_addr        + strlen($sections);                  // 文字列テーブル アドレス
$expresions_addr = -1;      // 式セクション アドレス
$external_symbols_addr = -1;// 外部シンボル アドレス
$header = 'Z80RMF18' . pack('VVVVVVVV', $cpu_id, $use_ixiy,
    $module_names_addr,
    $expresions_addr,
    $defined_symbols_addr,
    $external_symbols_addr,
    $sections_addr,
    $str_table_addr
);

// 出力
$obj = $header . $defined_symbols_section . $module_names_section . $sections . $str_table;
if (file_put_contents($obj_filename, $obj) === false) {
    fwrite(STDERR, 'File save error:' . $obj_filename . "\n");
    exit(1);
}
