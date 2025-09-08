<?php

declare(strict_types = 1);
/**
 * チェックサムの計算
 * 使い方は, Usage: 行を参照してください
 */

// --------------------------------
// 引数チェック
if (count($argv) != 4) {
    fwrite(STDERR, "invalid argument\n");    
    usage();
    exit(1);
}

$in_filename  = $argv[1];
$out_filename = $argv[2];
$option       = $argv[3];

if ($option !== '-a' && $option !== '-b' && $option !== '-c') {
    fwrite(STDERR, "invalid option[$option]\n");
    usage();
    exit(1);
}

if (file_exists($in_filename) === false) {
    fwrite(STDERR, "file not found[$in_filename]\n");
    exit(1);
}

$in_str = file_get_contents($in_filename);
$out_str = '';
$size = strlen($in_str);
$sum = 0;

for ($i = 0; $i < $size; $i++) {
    $sum += unpack('C', substr($in_str, $i, 1))[1];

}
$sum &= 0xffff;

switch($option) {
    case '-a':
        $out_str = sprintf("db      0x%02x, 0x%02x,", sum & 0xff, sum >> 8);
        break;
    case '-b':
        $out_str = pack('v', $sum);
        break;
    case '-c':
        $out_str = sprintf("    0x%02x, 0x%02x,", sum & 0xff, sum >> 8);        
        break;
}

file_put_contents($out_filename, $out_str);


function usage()
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in.bin outfile option\n" . 
        "  option is one of:\n" .
        "    -a output is assembler 'db' pseudo-instruction style\n" .
        "    -b output is binary\n" . 
        "    -c output is c-array style\n");
}
