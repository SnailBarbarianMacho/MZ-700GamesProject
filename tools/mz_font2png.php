<?php

declare(strict_types = 1);
/**
 * MZ-700 形式フォント(4096bytes)を読み取って 2 枚の png 出力します`
 * 使い方は, Usage: 行を参照してください
 *
 * @author Snail Barbarian Macho (NWK) 2021.06.22
 */


// 寸法定数
define('PIXEL_W',       4);    // ピクセルの幅
define('PIXEL_SPACE_W', 1);    // ピクセル間の幅
define('CHARA_SPACE_W', 4);    // 文字と文字の間(両端合わせて)
define('CHARA_W',            PIXEL_W * 8 + PIXEL_SPACE_W * 9);  // 1 文字の幅(文字間なし)
define('CHARA_WITH_SPACE_W', CHARA_W + CHARA_SPACE_W);          // 1 文字の幅(文字間込)
define('CODE_W',        30);   // コード表示の幅(文字間なし)
define('IMAGE_W',            CODE_W + CHARA_SPACE_W + CHARA_WITH_SPACE_W * 16 + CHARA_SPACE_W); // イメージの幅


function createImage(array $fontData, int $index, string $filename): void
{
    // イメージ生成
    $image = imagecreatetruecolor(IMAGE_W, IMAGE_W);
    $color = imagecolorallocate($image, 0x80, 0x80, 0x80);
    $bgcolor = imagecolorallocate($image, 0x00, 0x00, 0x00);
    $fgcolor = imagecolorallocate($image, 0xff, 0xff, 0xff);
    $font = 4;

    for ($cx = 0; $cx < 16; $cx++)
    {
        $x = $cx * CHARA_WITH_SPACE_W + CHARA_SPACE_W + CODE_W + CHARA_SPACE_W;
        $y = CHARA_SPACE_W;
        imagefilledrectangle($image, $x, $y, $x + CHARA_W - 1, $y + CODE_W - 1, $color);
        drawValue($image, $font, $x + 5, $y + 1, $cx);
    }
    for ($cy = 0; $cy < 16; $cy++)
    {
        $cy_ = $cy * CHARA_WITH_SPACE_W + CHARA_SPACE_W + CODE_W + CHARA_SPACE_W;
        {
            $x = CHARA_SPACE_W;
            imagefilledrectangle($image, $x, $cy_, $x + CODE_W,  $cy_ + CHARA_W - 1, $color);
            drawValue($image, $font, $x, $cy_ + 7, $cy * 16);
        }
        for ($cx = 0; $cx < 16; $cx++)
        {
            $cx_ = $cx * CHARA_WITH_SPACE_W + CHARA_SPACE_W + CODE_W + CHARA_SPACE_W;
            imagefilledrectangle($image, $cx_, $cy_, $cx_ + CHARA_W - 1, $cy_ + CHARA_W - 1, $color);
            for ($py = 0; $py < 8; $py++)
            {
                $py_ = $py * (PIXEL_W + PIXEL_SPACE_W) + PIXEL_SPACE_W + $cy_;
                $data = intval($fontData[$index]);

                for ($px = 0; $px < 8; $px++)
                {
                    $px_ = $px * (PIXEL_W + PIXEL_SPACE_W) + PIXEL_SPACE_W + $cx_;
                    imagefilledrectangle($image, $px_, $py_, $px_ + PIXEL_W - 1, $py_ + PIXEL_W - 1, ($data & 0x80) ? $fgcolor : $bgcolor);
                    $data <<= 1;
                }
                $index++;
            }
        }
    }

    imagepng($image, $filename);
    imagedestroy($image);
}


// --------------------------------
function drawValue(GdImage $image, int $font, int $x, int $y, int $val): void
{
    $str = sprintf('%02x', $val);
    // 文字が小さいので 2 倍に拡大します
    $image2 = imagecreate(100, 100);
    $bgColor = imagecolorallocate($image2, 0x80, 0x80, 0x80);
    $fgColor = imagecolorallocate($image2, 0xff, 0xff, 0xff);
    imagefilledrectangle($image2, 0, 0, 100, 100, $bgColor);
    imagestring($image2, $font, 0, 0, $str, $fgColor);
    imagestring($image2, $font, 1, 0, $str, $fgColor);

    $image3 = imagescale($image2, 200, 200, IMG_NEAREST_NEIGHBOUR);
    imagecopy($image, $image3, $x, $y, 1, 2, 31, 26);
    imagedestroy($image2);
    imagedestroy($image3);
    return;
}
// --------------------------------

// 引数チェック
if (count($argv) !== 4)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in-font out-png1 out-png2\n");
    exit(1);
}
$fontFile = $argv[1];

// ファイル存在チェック
if (file_exists($fontFile) === false)
{
    fwrite(STDERR, "File not found[$fontFile]\n");
    exit(1);
}

// ファイル サイズチェック
{
    $fileSize = filesize($fontFile);
    if ($fileSize !== 4096)
    {
        fwrite (STDERR, "The font file size must be 4096 bytes[$fileSize]\n");
        exit(1);
    }
}

$fontData = unpack('C*', file_get_contents($argv[1]));
createImage($fontData,    1, $argv[2]);
createImage($fontData, 2049, $argv[3]);
