<?php

declare(strict_types = 1);
require_once('nwk-classes/sound/wav.class.php');
require_once('nwk-classes/utils/utils.class.php');

/**
 * 複数の Wav ファイルを連結します
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2025.06.xx
 */


 // -------------------------------- 引数チェック
$args = [];
$errs = '';

$options = \nwk\utils\Utils::getOpt($argv,
    array('h', 'g:'),
    array('help', 'gap:'),
    $args, $errs);

if ($errs) {
    echo("Invalid option(s):$errs\n"); exit(1);
}
if (isset($options['help']) || isset($options['h'])) {
    usage($argv);
}

$gap = 0;
if (isset($options['gap']) || isset($options['g'])) {
    $gap = isset($options['gap']) ? $options['gap'] : $options['g'];
    //echo($gap."\n");
}
if (!is_numeric($gap) || $gap < 0 || 100 < $gap) {
    fwrite(STDERR, "Gap range from [0, 10] sec (default: 0)\n");
    exit(1);
}

// -------------------------------- ファイル名の整理
$filenames = [];
for ($i = 1; $i < count($argv); $i ++) {
    $filename = $argv[$i];
    if ($filename[0] !== '-') {
        $filenames[] = $filename;
    }
}
if (count($filenames) <= 1) {
    fwrite(STDERR, "Too few filenames\n");
    exit(1);
}

// -------------------------------- WAV データのマージ
$gap_samples = 0;
$wav_out = null;
for ($i = 0; $i < count($filenames) - 1; $i ++) {
    $filename = $filenames[$i];
    try {
        $wav = new nwk\sound\Wav();
    } catch (\Exception $e) {
        fwrite(STDERR, "[$filename]] " . $e->getMessage());
        exit(1);
    }
    if (!$wav->load($filename)) {
        fwrite(STDERR, "[$filename] " . $wav->getLastErrorMsg());
        exit(1);
    }
    $gap_samples = $gap * $wav->getSamplingFrequency();

    if ($i === 0) {
        $wav_out = $wav;
    } else {
        $wav_out = $wav_out->append($wav, $gap_samples);
        if (!$wav_out) {
            fwrite(STDERR, "[$filename] " . $wav->getLastErrorMsg());
            exit(1);
        }
    }
}

$out_filename = end($filenames);
fwrite(STDOUT, "[$out_filename] " . $wav_out->getInfoString() . "\n");
$wav_out->save($out_filename);


// -------------------------------- usage
function usage($argv)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " 1.wav 2.wav ... outfile.wav [options]\n");
    fwrite(STDERR, "Options are:\n");
    fwrite(STDERR, "  -h, --help\n");
    fwrite(STDERR, "     Show this message\n");
    fwrite(STDERR, "  -gxxx, --gap=xxx\n");
    fwrite(STDERR, "     Insert gap(second)[0, 10]\n");
    exit(1);
}
