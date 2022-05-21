<?php

declare(strict_types = 1);
/**
 * 三重和音の原理調査
 * @author Snail Barbarian Macho (NWK) 2021.07.06
 * or 合成法
 *          +---+             +---+
 *     A ---+   +-------------+   +----------
 *
 *          +---+       +---+       +---+
 *     B ---+   +-------+   +-------+   +----
 *
 *          +---+    +---+    +---+    +---+
 *     C ---+   +----+   +----+   +----+   +-
 *
 *          +---+    +------+ +---+ +------+
 * A|B|C ---+   +----+      +-+   +-+      +-
 *
 */

// --------------------------------
/**
 * wav バイナリを作成します
 *
 * @param string $data 波形データ バイナリ
 * @param int $nr_channels チャンネル数 1/2
 * @param int $sample_bits サンプリング ビット数 (8/16)
 * @param int $sample_rate サンプリングレート (44100 など)
 * @return string wav バイナリ
 */
function makeWaveData(string $data, int $nr_channels, int $sample_bits, int $sample_rate): string
{
    $block_size    = $nr_channels * ($sample_bits/8);
    $byte_per_secs = $block_size * $sample_rate;
    $format_id = 1; // linear PCM
    $fmt_chunk = 'WAVEfmt ';
    $fmt_chunk .= pack("V", 16); // fmt chunk length
    $fmt_chunk .= pack("v", $format_id);
    $fmt_chunk .= pack("vVV", $nr_channels, $sample_rate, $byte_per_secs);
    $fmt_chunk .= pack("vv", $block_size, $sample_bits);

    $data_chunk = 'data'.pack('V', strlen($data)).$data;
    $riff_len   = strlen($fmt_chunk)+strlen($data_chunk);
    return 'RIFF'.pack("V", $riff_len).$fmt_chunk.$data_chunk;
}

// --------------------------------
//const SAMPLE_RATE = 11025;
const SAMPLE_RATE = 22100;
const NR_CHANNELS = 1;
const SAMPLE_BITS = 8;    // u8
$freqs = [
    523.251,  // C5
    659.225,  // E5
    783.991,  // G5
];
$counts      = [];
$durations   = [];
$period      = 2;    // seconds;
$data        = '';
const PULSE_WIDTH = 12 / 44100 * SAMPLE_RATE;

for ($i = 0; $i < count($freqs); $i++) {
    $counts[$i] = 0;
    $durations[$i] = SAMPLE_RATE / $freqs[$i];
   // echo('durations'.$i.':'.$durations[$i]. "\n");
}

for ($j = 0; $j < SAMPLE_RATE * $period; $j++) {
    $mul = 1.0 - $j / (SAMPLE_RATE * $period); // 減衰
    $pulse_width = PULSE_WIDTH * $mul;
    $v = 0;
    for ($i = 0; $i < count($freqs); $i++) {
        if ($counts[$i] < $pulse_width) { // パルス幅一定
            $v = 255;
        }
        $counts[$i] ++;
        if ($durations[$i] < $counts[$i]) {
            $counts[$i] = 0;
        }
    }
    $data .= pack('C', $v);
}

$wav = makeWaveData($data, NR_CHANNELS, SAMPLE_BITS, SAMPLE_RATE);
file_put_contents('test1.wav', $wav);
