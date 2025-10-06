<?php

declare(strict_types = 1);
require_once('nwk-classes/sound/wav.class.php');
require_once('nwk-classes/utils/error.class.php');
require_once('nwk-classes/utils/utils.class.php');

/**
 * WAV, BMP 波形データ, テキスト波形データ のいずれかを読み込み,
 *   6bit, 7.85Hz にリサンプリングし (WAVファイルのみ),
 *   offset#256 サンプルを採用して (WAVファイルのみ),
 * - WAV, BMP 波形データ, テキスト波形データ, C ソース で出力(同時出力もOK)
 * - 例:
 *   - WAV → C ソース
 *     php $argv[1] drum3.wav drum3.h
 *   - WAV → テキスト波形データを出力
 *     php $argv[1] drum3.wav drum3.txt
 *   - テキスト波形データ → C ソース
 *     php $argv[1] drum3.txt drum3.h
 *   - WAV → C ソース + WAV, スケール1.5倍, verbose モード
 *     php $argv[1] drum3.wav drum3.h drum3-out.wav --vol_scale=1.5 -v
 *
 * @author Snail Barbarian Macho (NWK) 2024.09.06
 */

// --------------------------------
$error = new nwk\utils\Error();

const OPT_HELP    = 'help';
const OPT_VERBOSE = 'verbose';
const OPT_OFFSET  = 'offset';
const OPT_VOL_SCALE   = 'vol_scale';
const DEFAULT_OFFSET  = 0;
const DEFAULT_VOL_SCALE = 1.0;
const SAMPLEING_FREQ = 7850;    // 15700 / 2
const SAMPLE_BITS = 16;
const CHANNELS = 1;
const PRECISION = 63;           // 変換後の精度
$samples   = 256;
$b_verbose = false;
$offset    = DEFAULT_OFFSET;    // サンプルオフセット 0～
$vol_scale = DEFAULT_VOL_SCALE; // 音量スケール

// ---- 引数チェック,  オプション解析
$args = [];
$errs = '';
$options = nwk\utils\Utils::getOpt($argv,
    array('h',      'v'),
    array(OPT_HELP, OPT_VERBOSE, OPT_OFFSET . ':', OPT_VOL_SCALE . ':'),
    $args, $errs);
if ($errs) {
    $error->error("Invalid options: $errs\n");
    exit(1);
}

foreach($argv as $i => $arg) {
    if (str_starts_with($arg, '-')) {
        unset($argv[$i]);
    }
}

if (count($argv) < 3) {
    $error->error("Invalid number of arguments:" . count($argv) . "\n");
    usage($argv, $error);
}

if (isset($options[OPT_HELP])    || isset($options['h'])) { usage($argv, $error); }
if (isset($options[OPT_VERBOSE]) || isset($options['v'])) { $b_verbose = true; }
if (isset($options[OPT_OFFSET])) {
    $offset = $options[OPT_OFFSET];
    if (!is_numeric($offset) || $offset < 0) {
        $error->error("Invalid offset: $offset\n");
        exit(1);
    }
    $offset = (int)$offset;
}
if (isset($options[OPT_VOL_SCALE])) {
    $vol_scale = (float)$options[OPT_VOL_SCALE];
    if (!is_numeric($vol_scale) || $vol_scale < 0.0) {
        $error->error("Invalid volume scale: $vol_scale\n");
        exit(1);
    }
}

// ---- ファイル名チェック
$filename_in = $argv[1];
$error->setFilename($filename_in);
if (!str_ends_with($filename_in, '.wav') &&
    !str_ends_with($filename_in, '.bmp') &&
    !str_ends_with($filename_in, '.txt')) {
    $error->error("Invalid input file: $filename_in\n");
    exit(1);
}

unset($argv[0]);
unset($argv[1]);
$filenames_out = array_unique($argv);
foreach($filenames_out as $filename_out) {
    if (!str_ends_with($filename_out, '.wav') &&
        !str_ends_with($filename_out, '.bmp') &&
        !str_ends_with($filename_out, '.txt') &&
        !str_ends_with($filename_out, '.h')) {
            $error->error("Invalid output file: $filename_out\n");
            exit(1);
        }
    if ($filename_in == $filename_out) {
        $error->error("Same filename exists\n");
        exit(1);
    }
}
//echo(print_r($filenames_out, true) . "\n");

// ---------------- WAV ファイルを読み込んで加工
$wav = new nwk\sound\Wav();
if (str_ends_with($filename_in, '.wav')) {
    if ($wav->load($filename_in) === false) {
        $error->error($wav->getLastErrorMsg() . "\naborted.\n");
        exit(1);
    }
    // -------- リサンプリング
    $wav = $wav->resample(0, SAMPLEING_FREQ);
    $data = $wav->getData();
    $nr_samp2  = count($data);
    $samp2_end = $offset + $samples;
    if ($b_verbose) {
        echo("  $filename_in: サンプル数:" . $wav->getNrSamples() . " リサンプリング後:$nr_samp2\n");
        echo("  リサンプリング後に採用するサンプル位置#サンプル数:$offset#$samples\n");
        echo("  scale:$vol_scale\n");
    }

    // -------- 抽出
    if ($nr_samp2 <= $samp2_end) {
        if ($b_verbose) {
            echo("  $filename_in: サンプルが足りないので, 不足分を 0 で詰めます\n");
        }
        for (; $nr_samp2 < $samp2_end; $nr_samp2++) {
            $data[] = 0;
        }
    }

    // -------- 音量調整
    $data = array_slice($data, $offset, $samples);
    $wav = $wav->setData($data)->autoMaximize($vol_scale);

    // -------- 精度を落とす
    $data = $wav->getData();
    foreach($data as &$r_val) {
        $r_val = round(((float)$r_val + 32768) / ((float)(1 << SAMPLE_BITS) / PRECISION));
    }
} else if (str_ends_with($filename_in, '.bmp')) {
    $img = imagecreatefrombmp($filename_in);
    if ($img === false) {
        $error->error("File read error\n");
        exit(1);
    }
    $w = imagesx($img);
    $h = imagesy($img);
    if ($samples < $w) {
        $error->error("Invalid image width:$w\n");
        exit(0);
    }
    if (PRECISION !== imagesy($img)) {
        $error->error("Invalid image height:$h\n");
        exit(0);
    }
    if (imageistruecolor($img)) {
        $error->error("Only index256 color is supported\n");
        exit(0);
    }
    if ($b_verbose) {
        echo("  $filename_in: $w x $h indexed\n");
    }

    $data = [];
    for ($x = 0; $x < $w; $x++) {
        $val = $h;
        for ($y = $h - 1; $y >= 0; $y--) {
            $col = imagecolorat($img, $x, $y);
            //if ($x == 20)echo("[$x $y]=$col ");
            if ($col !== 0) {
                $val = $h - $y - 1;
                break;
            }

        }
        $data[] = $val;
    }
    //print_r($data);
} else if (str_ends_with($filename_in, '.txt')) {
    $contents = file_get_contents($filename_in);
    if ($contents === false) {
        $error->error("File read error\n");
        exit(1);
    }
    $contents = preg_replace('/[\t 　]/', '', $contents); // 空白文字の削除
    $contents = preg_replace('/\r\n?/', "\n", $contents); // 改行統一
    $data = explode("\n", $contents);

    foreach($data as $i => $val) {
        $data[$i] = min(PRECISION, strlen($val));
    }
    if (count($data) < 256) {
        if ($b_verbose) {
            echo("  $filename_in: データが" . 256-count($data) . "個足りないので, 不足分を 0 で詰めます\n");
        }
        for ($i = count($data); $i < 256; $i++) {
            $data[$i] = 0;
        }
    } else if (256 < count($data)) {
        if ($b_verbose) {
            echo("  $filename_in: データが多すぎるので, 末端の" . count($data)-256 . "個は削除します\n");
        }
        $data = array_slice($data, 0, 256);
    }
}
//echo(print_r($data, true));

// ---------------- 出力
foreach($filenames_out as $filename_out) {
    if (str_ends_with($filename_out, '.wav')) {
        $data2 = str_repeat('  ', count($data));
        foreach($data as $i => $val) {
            $val = (int)((float)$val * (1 << SAMPLE_BITS) / PRECISION) - 32768;
            $val = min($val, 32767);
            $val = max($val, -32768);
            $val_l = $val & 0xff;
            $val_h = ($val >> 8) & 0xff;
            $data2[$i * 2    ] = pack('C', $val_l);
            $data2[$i * 2 + 1] = pack('C', $val_h);
        }
        $wav->setData($data2);
        $wav->setSamplingFrequency(SAMPLEING_FREQ);
        $wav->setSampleBits(SAMPLE_BITS);
        $wav->setChannels(CHANNELS);
        if ($wav->save($filename_out) === false) {
            $error->error("file write error[$filename_out]\n");
            exit(1);
        }
    } else if (str_ends_with($filename_out, '.txt')) {
        $out_str = '';
        foreach($data as $i => $val) {
            $str = '';
            for ($j = 0; $j < $val; $j++) {
                $str .= (($j % 8) == 0) ? '_' : '#';
            }
            $out_str .= $str;
            if ($i !== count($data) - 1) { $out_str .= "\n"; }
        }
        if (file_put_contents($filename_out, $out_str) === false) {
            $error->error("file write error[$filename_out]\n");
            exit(1);
        }
    } else if (str_ends_with($filename_out, '.bmp')) {
        // カラー0 はパレット0に, それ以外はパレット1に
        $w = count($data);
        $h = PRECISION;
        $img = imagecreatetruecolor($w, $h);
        if ($img === false) {
            $error->error("image creation failed[$filename_out]\n");
            exit(1);
        }
        // パレット1を抽出する為に, 1pixelだけ打つ
        $col = imagecolorallocate($img, 255, 255, 255);
        imagesetpixel($img, 0, 1, $col);
        imagetruecolortopalette($img, false, 256);
        imagecolorset($img,  0,   0,   0, 255);
        imagecolorset($img,  1, 255, 255, 255);

        $ymax = PRECISION;
        foreach ($data as $x => $val) {
            for ($y = 0, $yy = PRECISION; $y <= $val; $y++, $yy--) {
                imagesetpixel($img, $x, $yy, 0);
            }
            for (; $yy >= 0; $yy--) {
                imagesetpixel($img, $x, $yy, 1);
            }
        }/**/
        if (imagebmp($img, $filename_out, false) === false) {
            $error->error("file write error[$filename_out]\n");
            exit(1);
        }
    } else if (str_ends_with($filename_out, '.h')) {
        // 最初の256フレームは, 0, 0, 2, 2, 4, 4, ... 番目を鳴らす
        // 次の256フレームは,   1, 1, 3, 3, 5, 5, ... 番目を鳴らす
        $data2 = array_fill(0, count($data), 0);
        $data_half = count($data) / 2;
        foreach($data as $i => $val) {
            if ($i < $data_half) {
                $data2[$i * 2] = $val;
            } else {
                $idx = ($i - $data_half) * 2 + 1;
                //echo("$i => $val $idx\n");
                $data2[($i - $data_half) * 2 + 1] = $val;
            }
        }
        $data2 = array_values($data2);

        $out_str = nwk\utils\Utils::byteDumpDec($data2);
        if (file_put_contents($filename_out, $out_str) === false) {
            $error->error("file write error[$filename_out]\n");
            exit(1);
        }
    }
    if ($b_verbose) {
        echo("  データ書き込みました:$filename_out\n");
    }
}

//echo($out_str);


function usage(array $argv, nwk\utils\Error $error): void {
    $error->error('Usage: php ' . $argv[0] . " in out1 out2 ... [options...]\n" .
        "  Infiles is: *.wav, *.bmp, *.txt\n" .
        "  Outfiles are: *.wav, *.bmp, *.txt, *.h\n" .
        "  Options are:\n" .
        "  -h, --" . OPT_HELP . "\n" .
        "  -v, --" . OPT_VERBOSE . "\n" .
        "  --" . OPT_OFFSET . "=[offset]   サンプル オフセット(整数)(default:" . DEFAULT_OFFSET . ")\n" .
        "  --" . OPT_VOL_SCALE . "=[scale] 音量スケーリング(小数)(default:" . DEFAULT_VOL_SCALE . ")\n"
    );
    exit(1);
}
