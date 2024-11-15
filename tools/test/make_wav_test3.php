<?php

declare(strict_types = 1);
require_once(__DIR__ . '/../nwk-classes/sound/wav.class.php');
require_once(__DIR__ . '/../nwk-classes/utils/error.class.php');

/**
 * PWM 方式用のサンプリングデータ聴覚試験
 * 元WAVデータを加工して, 「元データ, 16000Hz-6bit, 8000Hz-6bit, 8000Hz-4bit」のデータを作る
 */

$filename_in_wav  = $argv[1];
$filename_out_wav = $argv[2];
//echo("$filename_in_wav $filename_out_wav\n");

$error = new \nwk\utils\Error();
$wav = new \nwk\sound\Wav();
if ($wav->load($filename_in_wav) === false) {
    $error->error($wav->getLastErrorMsg() . "aborted.\n");
    exit(1);
}
echo("$filename_in_wav: " . $wav->getInfoString() . "\n");

//$wav_16k_6b = $wav->degrade(15700, 'point', true, 6, true)->fill(-1, -20000);
//$wav__8k_6b = $wav->degrade(7950,  'point', true, 6, true)->fill(-1, -20000);
//$wav_16k_4b = $wav->degrade(15700, 'point', true, 4, true)->fill(-1, -20000);
//$wav__8k_4b = $wav->degrade(7950,  'point', true, 4, true)->fill(-1, -20000);
$wav = $wav->resample(0, 7950, 0)->autoMaximize(1.5);
$data = $wav->getData();
foreach($data as &$r_val) {
    $r_val &= ~0x03ff;
}
$wav->setData($data);

//$wav = $wav->append($wav_16k_6b);
//$wav = $wav->append($wav__8k_4b);
//$wav = $wav->append($wav)->append($wav);
echo("wav: " . $wav->getInfoString() . "\n");
$wav->save($filename_out_wav);
