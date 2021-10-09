<?php

declare(strict_types = 1);
/**
 * 三重和音の原理調査
 * @author Snail Barbarian Macho (NWK) 2021.07.06
 */


// --------------------------------
/**
 * wav バイナリを作成します
 *
 * @param string $data 波形データ バイナリ
 * @param int $nrChannels チャンネル数 1/2
 * @param int $sampleBits サンプリング ビット数 (8/16)
 * @param int $sampleRate サンプリングレート (44100 など)
 * @return string wav バイナリ
 */
function makeWaveData(string $data, int $nrChannels, int $sampleBits, int $sampleRate): string
{
    $blockSize = $nrChannels * ($sampleBits/8);
    $bytePerSecs = $blockSize * $sampleRate;
    $formatId = 1; // linear PCM
    $fmtChunk = 'WAVEfmt ';
    $fmtChunk .= pack("V", 16); // fmt chunk length
    $fmtChunk .= pack("v", $formatId);
    $fmtChunk .= pack("vVV", $nrChannels, $sampleRate, $bytePerSecs);
    $fmtChunk .= pack("vv", $blockSize, $sampleBits);

    $dataChunk = 'data'.pack('V', strlen($data)).$data;
    $riffLength = strlen($fmtChunk)+strlen($dataChunk);
    return 'RIFF'.pack("V", $riffLength).$fmtChunk.$dataChunk;
}


// --------------------------------
$sampleRate = 22100;
$nrChannels = 1;
$freqs = [
    523.251,
    659.225,
    783.991,
];
$counts    = array();
$durations = array();
$sampleBits = 8;    // u8
$period     = 1;    // seconds;
$data       = '';
$pulseWidth = 4;   // 44100Hz に対して 10 くらい

for ($i = 0; $i < count($freqs); $i++) {
    $counts[$i] = 0;
    $durations[$i] = $sampleRate / $freqs[$i];
   // echo('durations'.$i.':'.$durations[$i]. "\n");
}

for ($j = 0; $j < $sampleRate * $period; $j++) {
    $v = 0;
    for ($i = 0; $i < count($freqs); $i++) {
        if ($counts[$i] < $pulseWidth) { // パルス幅一定
            $v = 255;
        }
        $counts[$i] ++;
        if ($durations[$i] < $counts[$i]) {
            $counts[$i] = 0;
        }
    }
    $data .= pack('C', $v);
}

$wav = makeWaveData($data, $nrChannels, $sampleBits, $sampleRate);
file_put_contents('a.wav', $wav);
