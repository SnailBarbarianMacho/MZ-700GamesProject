<?php

declare(strict_types = 1);
/**
 * png ファイルとフォント ファイルを読んで,
 * 疑似グラフィック用のデータ(cソース)と複数の予想イメージを作成します
 * 目視して最適なデータを選択してください
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2021.07.08
 */



// --------------------------------
/** データを作って出力します
 * @param $bitImgArr      ビット イメージの配列(8bit = 1単位)
 * @param $width          ビット イメージの幅(pixel)
 * @param $height         ビット イメージの高さ(pixel)
 * @param $fontData       フォントデータ配列
 * @param $algo           アルゴリズム. 'dist'(ユーグリット距離), 'cos0'(コサイン類似度 0), 'cos1'(コサイン類似度 1)
 * @param $pngBaseName    'hoge.png' ならば 'hoge' のみ. 'hoge-dist-inv0-atb0-score23.png' のような名前が付きます
 */
function createAnDiffAndOutData(
    array $bitImgArr,
    int $width,
    int $height,
    array $fontData,
    string $algo,
    string $pngBaseName
): void
{
    // 反転と ATB の組み合わせで 4 種類のデータを作る
    list($outDataArr[0], $scoreArr[0]) = createData($algo, $bitImgArr, $width, $height, 0x00, $fontData, 1);
    list($outDataArr[1], $scoreArr[1]) = createData($algo, $bitImgArr, $width, $height, 0xff, $fontData, 1);
    list($outDataArr[2], $scoreArr[2]) = createData($algo, $bitImgArr, $width, $height, 0x00, $fontData, 2049);
    list($outDataArr[3], $scoreArr[3]) = createData($algo, $bitImgArr, $width, $height, 0xff, $fontData, 2049);

    // 最もスコアの低いデータを探します
    $score = PHP_INT_MAX;
    $idx   = 0;
    for ($i = 0; $i < 4; $i++) {
        if ($scoreArr[$i] < $score) {
            $score = $scoreArr[$i];
            $idx   = $i;
        }
    }

    $bBitImgInv = $idx & 1;
    $bAtb       = ($idx >> 1) & 1;

    // 最もスコアの低いデータのみ出力します
    writeData($outDataArr[$idx], $width, $height, $algo, $bBitImgInv, $bAtb, $scoreArr[$idx]);
    saveImage($outDataArr[$idx], $width, $height, $algo, $bBitImgInv, $bAtb, $scoreArr[$idx], $fontData, $pngBaseName);
}

/** データ作成
 * @param $algo           アルゴリズム. 'dist'(ユーグリット距離), 'cos0'(コサイン類似度 0), 'cos1'(コサイン類似度 1)
 * @param $bitImgArr      ビット イメージの配列(8bit = 1単位)
 * @param $width          ビット イメージの幅(pixel)
 * @param $height         ビット イメージの高さ(pixel)
 * @param $bitImgInv      ビット イメージ反転 0x00 か 0xff
 * @param $fontData       フォントデータ配列
 * @param $fontDataOffset フォントデータ配列のオフセット. 1 か 2049
 * @return 「データ配列, 異なるピクセル数」を返します
 */
function createData(
    string $algo,
    array $bitImgArr,
    int $width,
    int $height,
    int $bitImgInv,
    array $fontData,
    int $fontDataOffset
): array
{
    $i     = 0;
    $score = 0;
    $outData = array_fill(0, 0x00, $width * $height / 8);
    for ($y = 0; $y < $height; $y++) {
        $line = $y & 7;
        for ($x = 0; $x < $width; $x += 8) {
            list($char, $sc) = searchChara($algo, $bitImgArr[$i] ^ $bitImgInv, $fontData, $fontDataOffset, $line);
            $score += $sc;
            $outData[$i] = $char;
            $i++;
        }
    }
    return [$outData, $score];
}

/**
 * この 8bit のビットイメージに最も近い フォントデータのキャラ番号を返します
 *
 * - 近似の計算方法
 *   1. ユークリッド距離が一番近い(=ドットの違いが一番小さい)文字を選びます
 *      8 bit なので 8 次元空間で計算します
 *   2. 同じ距離ならば, コサイン類似度が一番小さい文字を採用します
 *
 *      コサイン近似度は「2つのベクトルの角度」で示します
 *      角度が小さいほど, つまり cosθ が大きいほど近い文字.
 *      cosθ はベクトルの内積の公式をつかって
 *           cosθ = (Ax*Bx + Ay*By + ...) / |A||B|
 *
 *      8 bit なので 8 次元空間で計算します
 *      8 bit 値 A, B では,
 *      |A| は「ビット '1' の数の平方根」
 *      Ax*Bx + Ay*By ... は, AB両方が '1' のビット数
 *
 *      ベクトルの大きさが 0 だと計算できないので注意
 *
 * @param $bitImg   ビット イメージ
 * @param $fontData フォントデータ配列
 * @param $fontDataOffset フォントデータ配列のオフセット. 1 か 2049
 * @param $line     ライン番号
 * @return キャラコードとスコアの配列です.
 *    スコアはドットの違いの数です. 小さいほど近いことを意味します
 */
function searchChara(
    string $algo,
    int $bitImg,
    array $fontData,
    int $fontDataOffset,
    int $line): array
{
    $minScore = 8;  // ユークリッド距離
    $minCos   = -1; // コサイン類似度
    $chara    = 0;
    for ($c = 0; $c < 256; $c++) {
        // 8bit なので, 8 次元空間のユーグリット距離を求めて最も小さい距離の文字(スコア)を採用します
        // ... スコアは, ドットの不一致数と同じになります
        $fontBitImg = $fontData[$c * 8 + $line + $fontDataOffset];
        $score = 0;
        for ($b = 0; $b < 8; $b++) {
            $tmp  = (($bitImg >> $b) & 1) - (($fontBitImg >> $b) & 1);
            $score += $tmp * $tmp;  // ユーグリッド距離
        }

        // 一致したら, その文字を返します
        if ($score == 0) {
            return array($c, 0);
        }

        if ($score < $minScore) {
            // よりユークリッド距離が短い文字が見つかれば, コサイン類似度はリセット
            $minScore = $score;
            $chara    = $c;
            $minCos   = -1.0;
            $denominator = sqrt(bitCount($bitImg)) * sqrt(bitCount($fontBitImg));
            if ($denominator) {
                $minCos = bitCount($bitImg & $fontBitImg) / $denominator;  // 両方が 1 のビット数
            }
        } else if ($score == $minScore) {
            switch ($algo) {
                default:
                    break;
                case 'cos0':
                    // 同じユークリッド距離ならば, コサイン類似度が近い文字を選びます
                    $denominator = sqrt(bit0Count($bitImg)) * sqrt(bit0Count($fontBitImg));
                    if ($denominator) {
                        $tmp = bit0Count($bitImg | $fontBitImg) / $denominator;  // 両方が 0 のビット数
                        if ($minCos < $tmp) {
                            $minCos = $tmp;
                            //printf("%f\n", $minCos);
                            $chara  = $c;
                        }
                    }
                    break;
                case 'cos1':
                    // 同じユークリッド距離ならば, コサイン類似度が近い文字を選びます
                    $denominator = sqrt(bitCount($bitImg)) * sqrt(bitCount($fontBitImg));
                    if ($denominator) {
                        $tmp = bitCount($bitImg & $fontBitImg) / $denominator;  // 両方が 1 のビット数
                        if ($minCos < $tmp) {
                            $minCos = $tmp;
                            //printf("%f\n", $minCos);
                            $chara  = $c;
                        }
                    }
                    break;
            }
        }
    }

    return array($chara, $minScore);
}


/**
 * データを C 形式で出力します
 * パラメータは createAndOutData() 参照
 */
function writeData($outData, $width, $height, $algo, $bBitImgInv, $bAtb, $score): void
{
    $i = 0;
    echo ('    // '.createName($algo, $bBitImgInv, $bAtb, $score)."\n");
    for ($y = 0; $y < $height; $y++) {
        echo("    ");
        for ($x = 0; $x < $width; $x += 8) {
            printf('0x%02x, ', $outData[$i]);
            $i++;
        }
        echo("\n");
        if (($y & 7) == 7) {
            echo("\n");
        }
    }
}

/**
 * データを png 出力
 * パラメータは createAndOutData() 参照
 */
function saveImage($outData, $width, $height, $algo, $bBitImgInv, $bAtb, $score, $fontData, $pngBaseName): void
{
    // データを元にビットマップを作成
    $image = imagecreate($width, $height);
    $bgColor = imagecolorallocate($image, 0x00, 0x00, 0x00);
    $fgColor = imagecolorallocate($image, 0xff, 0xff, 0xff);
    if ($bBitImgInv) {
        list($bgColor, $fgColor) = array($fgColor, $bgColor);
    }
    $fontDataOffset = $bAtb ? 2049 : 1;

    $i = 0;
    for ($y = 0; $y < $height; $y++) {
        $line = $y & 7;
        for ($x = 0; $x < $width; $x += 8) {
            $charBitmap = $fontData[$outData[$i] * 8 + $line + $fontDataOffset];
            $i++;
            for ($b = 0; $b < 8; $b++) {
                imagesetpixel($image, $x + $b, $y, ($charBitmap & (0x80>>$b)) ? $fgColor : $bgColor);
            }
        }
    }
    imagepng($image, $pngBaseName.'-'.createName($algo, $bBitImgInv, $bAtb, $score).'.png');
    imagedestroy($image);
}

function createName($algo, $bBitImgInv, $bAtb, $score): string
{
    return $algo.'-inv'.$bBitImgInv.'-atb'.$bAtb.'-score'.$score;
}

function bitCount($val)
{
    $c = $val - (($val >> 1) & 0x55555555);
    $c = (($c >> 2) & 0x33333333) + ($c & 0x33333333);
    $c = (($c >> 4) + $c) & 0x0F0F0F0F;
    $c = (($c >> 8) + $c) & 0x00FF00FF;
    $c = (($c >> 16) + $c) & 0x0000FFFF;
    return $c;
}

function bit0Count($val)
{
    return 8 - bitCount($val);
}


// --------------------------------
// 引数チェック
if (count($argv) !== 3)
{
    fwrite(STDERR, 'Usage: php '.$argv[0]." in-png in-font\n");
    fwrite(STDERR, "  The out-png file name variations are created\n");
    fwrite(STDERR, "  (eg. test.png -> test_dist_inv0_atb0_score22.png etc.)\n");
    exit(1);
}

// png ファイル存在チェック
if (file_exists($argv[1]) === false)
{
    fwrite(STDERR, 'File not found:' . $argv[1] . "\n");
    exit(1);
}
// font ファイル存在チェック
if (file_exists($argv[2]) === false)
{
    fwrite(STDERR, 'File not found:' . $argv[2] . "\n");
    exit(1);
}

// png ファイルを読んで寸法チェック
// 縦は 8 の倍数で, 64 pixel まで
// 高さは 8 の倍数で, 240 pixel まで
$image = imagecreatefrompng($argv[1]);
$width = imagesx($image);
$height = imagesy($image);
if (($width < 8) || (88 < $width) ||
    ($height < 8) || (240 < $height) ||
    ($width % 8 != 0) || ($height % 8 != 0)) {
    fwrite(STDERR, 'Invalid image size:' . $argv[2] . ' (' . $width . 'x' . $height . ")\n");
    exit(1);
}

// イメージは 8 bit 単位で配列に格納
$bitImgArr = array_fill(0, $width * $height / 8, 0);
$i = 0;
for ($y = 0; $y < $height; $y++) {
    $bit = 0;
    for ($x = 0; $x < $width; $x++) {
        $rgb = imagecolorat($image, $x, $y);
        $r = ($rgb >> 16) & 0xff;
        $bitImgArr[$i] <<= 1;
        $bitImgArr[$i] |= ($r > 128) ? 1 : 0;
        //$imageArr[$i] |= ($r < 128) ? 1 : 0;
        $bit++;
        if ($bit == 8) {
            $bit = 0;
            $i++;
        }
    }
}
echo('// image size:('.$width.'x'.$height.') '.count($bitImgArr)."bytes\n");
imagedestroy($image);

// font ファイル サイズチェック
{
    $fileSize = filesize($argv[2]);
    if ($fileSize !== 4096)
    {
        fwrite (STDERR, 'The font file size must be 4096 bytes:' . $fileSize . "\n");
        exit(1);
    }
}

// フォントは配列に
$fontData = unpack('C*', file_get_contents($argv[2]));

// データ作成と選択
// スコアと画像を生成するので, ユーザー判断で選択してもらいます
$pathInfo = pathinfo($argv[1]);
$pngBaseName = $pathInfo['dirname'].'/'.$pathInfo['filename'];
createAnDiffAndOutData($bitImgArr, $width, $height, $fontData, 'dist', $pngBaseName);
createAnDiffAndOutData($bitImgArr, $width, $height, $fontData, 'cos0', $pngBaseName);
createAnDiffAndOutData($bitImgArr, $width, $height, $fontData, 'cos1', $pngBaseName);
