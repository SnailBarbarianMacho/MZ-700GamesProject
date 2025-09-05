<?php

declare(strict_types = 1);
require_once('nwk-classes/sound/wav.class.php');
require_once('nwk-classes/utils/utils.class.php');
require_once('nwk-classes/utils/mzt.class.php');

/**
 * MZT ファイルから WAV データを生成します
 * @author Snail Barbarian Macho (NWK) 2025.06.19
 */

// -------------------------------- 引数チェック
$args = [];
$errs = '';

$options = \nwk\utils\Utils::getOpt($argv,
    array('h', 'v:', 'd', 'f'),
    array('help', 'volume:', 'dataonly', 'firstonly'),
    $args, $errs);

if ($errs) {
    echo("Invalid option(s):$errs\n"); exit(1);
}
if (isset($options['help']) || isset($options['h'])) {
    usage($argv);
}

$vol = 100;
if (isset($options['volume']) || isset($options['v'])) {
    $vol = isset($options['volume']) ? $options['volume'] : $options['v'];
}
if (!is_numeric($vol) || $vol != (int)$vol || $vol < 0 || 100 < $vol) {
    fwrite(STDERR, "Volume range from [0, 100] (default: 100)\n");
    exit(1);
}
$vol = (int)$vol;

$b_dataonly = false;
if (isset($options['d']) || isset($options['dataonly'])) {
    $b_dataonly = true;
}

$b_firstonly = false;
if (isset($options['f']) || isset($options['firstonly'])) {
    $b_firstonly = true;
}

if (count($args) !== 2) {
    usage($argv);
}
$in_filename = $args[0];
$out_filename = $args[1];
if (!file_exists($in_filename)) {
    fwrite(STDERR, "File open error [$in_filename]");
    exit(1);
}

$data_str = file_get_contents($in_filename);
if ($data_str === false) {
    fwrite(STDERR, "File read error [$in_filename]");
    exit(1);
}

try {
    $mzt = new \nwk\utils\MZT($data_str);
} catch (\Exceprion $e) {
    fwrite(STDERR, "[$in_filename]:" . $e->getMessage());
    exit(1);
}


const SAMPLE_FREQ = 48000;
const NR_CHANNELS = 1;
const SAMPLE_BITS = 8;
//const SAMPLE_BITS = 16;

//$t = microtime(true);
//echo "$b_firstonly \n";
$wav_str = $mzt->genWavData($out_filename, SAMPLE_FREQ, SAMPLE_BITS, $b_dataonly, $b_firstonly);
unset($mzt);

try {
    $wav = new nwk\sound\Wav(NR_CHANNELS, SAMPLE_FREQ, SAMPLE_BITS, $wav_str);
} catch (\Exception $e) {
    fwrite(STDERR, "[$in_filename] " . $e->getMessage());
    exit(1);
}
$wav->adjustVolume($vol / 100.0);

fwrite(STDOUT, "[$out_filename] " . $wav->getInfoString() . "\n");
if (!$wav->save($out_filename)) {
    fwrite(STDERR, "[$out_filename] Save failed\n");
    exit(1);
}
//echo(microtime(true) - $t . "\n");


// -------------------------------- usage
function usage($argv)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " infile.mzt outfile.wav [options]\n");
    fwrite(STDERR, "Options are:\n");
    fwrite(STDERR, "  -h, --help\n");
    fwrite(STDERR, "     Show this message\n");
    fwrite(STDERR, "  -lxxx, --level=xxx\n");
    fwrite(STDERR, "     Level range from[0, 1]\n");
    fwrite(STDERR, "  -d, --dataonly\n");
    fwrite(STDERR, "     Data block only\n");
    fwrite(STDERR, "  -f, --fastonly\n");
    fwrite(STDERR, "     No duplication\n");
    exit(1);
}
