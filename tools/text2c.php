<?php

declare(strict_types = 1);
/**
 * テキスト文字列をディスプレイ コードの配列に変換し
 *
 * 使えるタグは,
 *   {sp}{lf}{lf2}
 *   {small-}
 *   {1dot}
 *   {↓}{↑}{→}{←}
 *   {man↓}{man↑}{man→}{man←}
 *   {ufo}{snake}
 *   {nicochan1}{nicochan0}
 *   {col=fgColor,bgColor}
 *   {col=fgColor}          ※ bgColor=0 の場合
 *   {moveRight=n}
 *   {moveDown=n}
 *
 * 使い方は, Usage: 行を参照してください
 *
 */

// --------------------------------
// 引数チェック
if (count($argv) != 3)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in.txt out.h\n");
    exit(1);
}
$inTextFilename = $argv[1];
$outFilename    = $argv[2];

if (file_exists($inTextFilename) === false) {
    fwrite(STDERR, "file not found[$inTextFilename]\n");
    exit(1);
}

const TAB = [
    ' ' => 'DC_SP',
    '　' => 'DC_SP',
    '{sp}' => 'DC_SP',
    '{lf}' => 'DC_LF',
    '{lf2}' => 'DC_LF2',

    'A' => 'DC_A', 'B' => 'DC_B', 'C' => 'DC_C', 'D' => 'DC_D', 'E' => 'DC_E',
    'F' => 'DC_F', 'G' => 'DC_G', 'H' => 'DC_H', 'I' => 'DC_I', 'J' => 'DC_J',
    'K' => 'DC_K', 'L' => 'DC_L', 'M' => 'DC_M', 'N' => 'DC_N', 'O' => 'DC_O',
    'P' => 'DC_P', 'Q' => 'DC_Q', 'R' => 'DC_R', 'S' => 'DC_S', 'T' => 'DC_T',
    'U' => 'DC_U', 'V' => 'DC_V', 'W' => 'DC_W', 'X' => 'DC_X', 'Y' => 'DC_Y',
    'Z' => 'DC_Z',
    'a' => 'DC_A', 'b' => 'DC_B', 'c' => 'DC_C', 'd' => 'DC_D', 'e' => 'DC_E',
    'f' => 'DC_F', 'g' => 'DC_G', 'h' => 'DC_H', 'i' => 'DC_I', 'j' => 'DC_J',
    'k' => 'DC_K', 'l' => 'DC_L', 'm' => 'DC_M', 'n' => 'DC_N', 'o' => 'DC_O',
    'p' => 'DC_P', 'q' => 'DC_Q', 'r' => 'DC_R', 's' => 'DC_S', 't' => 'DC_T',
    'u' => 'DC_U', 'v' => 'DC_V', 'w' => 'DC_W', 'x' => 'DC_X', 'y' => 'DC_Y',
    'z' => 'DC_Z',
    '0' => 'DC_0', '1' => 'DC_1', '2' => 'DC_2', '3' => 'DC_3', '4' => 'DC_4',
    '5' => 'DC_5', '6' => 'DC_6', '7' => 'DC_7', '8' => 'DC_8', '9' => 'DC_9',
    '+' => 'DC_PLUS',   '-' => 'DC_MINUS', '*' => 'DC_STAR', '/' => 'DC_SLASH',
    '#' => 'DC_NUMBER', '$' => 'DC_DOLLER', '%' => 'DC_PERCENT',  '&' => 'DC_AMP',
    '=' => 'DC_EQUAL',         '@' => 'DC_AT',
    ':' => 'DC_COLON',         ';' => 'DC_SEMICOLON',
    '.' => 'DC_PERIOD',        ',' => 'DC_COMMA',
    '?' => 'DC_QUESTION',      '？' => 'DC_QUESTION',
    '!' => 'DC_EXCLAM',        '！' => 'DC_EXCLAM',
    '"' => 'DC_QUOT',          "'" => 'DC_APOS',
    '(' => 'DC_L_BLACKET',     ')' => 'DC_R_BLACKET',
    '[' => 'DC_L_SQ_BLACKET',  ']' => 'DC_R_SQ_BLACKET',
    '<' => 'DC_LT',            '>' => 'DC_GT',
    'π' => 'DC_PI',            '￥' => 'DC_YEN',      '\\' => 'DC_BACK_SLASH',
    '←' => 'DC_R_ARROW',       '↑' => 'DC_U_ARROW',

    '■' => 'DC_FULL_BLOCK',
    '▀' => 'DC_UPPER_HALF_BLOCK',
    '▄' => 'DC_LOWER_HALF_BLOCK',
    '▌' => 'DC_LEFT_HALF_BLOCK',
    '▐' => 'DC_RIGHT_HALF_BLOCK',
    '◤' => 'DC_UPPER_LEFT_TRIANGLE',
    '◥' => 'DC_UPPER_RIGHT_TRIANGLE',
    '◣' => 'DC_LOWER_LEFT_TRIANGLE',
    '◢' => 'DC_LOWER_RIGHT_TRIANGLE',
    '♠' => 'DC_SPADE', '♦' => 'DC_DIAMOND', '♣' => 'DC_CLUB', '♥' => 'DC_HEART',

    'ア' => 'DC_KANA_A',  'イ' => 'DC_KANA_I',  'ウ' => 'DC_KANA_U',  'エ' => 'DC_KANA_E',  'オ' => 'DC_KANA_O',
    'ァ' => 'DC_KANA_XA', 'ィ' => 'DC_KANA_XI', 'ゥ' => 'DC_KANA_XU', 'ェ' => 'DC_KANA_XE', 'ォ' => 'DC_KANA_XO',
    'ヴ' => 'DC_KANA_VU',
    'カ' => 'DC_KANA_KA', 'キ' => 'DC_KANA_KI',  'ク' => 'DC_KANA_KU',  'ケ' => 'DC_KANA_KE', 'コ' => 'DC_KANA_KO',
    'ガ' => 'DC_KANA_GA', 'ギ' => 'DC_KANA_GI',  'グ' => 'DC_KANA_GU',  'ゲ' => 'DC_KANA_GE', 'ゴ' => 'DC_KANA_GO',
    'サ' => 'DC_KANA_SA', 'シ' => 'DC_KANA_SHI', 'ス' => 'DC_KANA_SU',  'セ' => 'DC_KANA_SE', 'ソ' => 'DC_KANA_SO',
    'ザ' => 'DC_KANA_ZA', 'ジ' => 'DC_KANA_ZI',  'ズ' => 'DC_KANA_ZU',  'ゼ' => 'DC_KANA_ZE', 'ゾ' => 'DC_KANA_ZO',
    'タ' => 'DC_KANA_TA', 'チ' => 'DC_KANA_CHI', 'ツ' => 'DC_KANA_TSU', 'テ' => 'DC_KANA_TE', 'ト' => 'DC_KANA_TO',
    'ダ' => 'DC_KANA_DA', 'ヂ' => 'DC_KANA_DI',  'ヅ' => 'DC_KANA_DU',  'デ' => 'DC_KANA_DE', 'ド' => 'DC_KANA_DO',
    'ッ' => 'DC_KANA_XTSU',
    'ナ' => 'DC_KANA_NA', 'ニ' => 'DC_KANA_NI', 'ヌ' => 'DC_KANA_NU', 'ネ' => 'DC_KANA_NE', 'ノ' => 'DC_KANA_NO',
    'ハ' => 'DC_KANA_HA', 'ヒ' => 'DC_KANA_HI', 'フ' => 'DC_KANA_FU', 'ヘ' => 'DC_KANA_HE', 'ホ' => 'DC_KANA_HO',
    'バ' => 'DC_KANA_BA', 'ビ' => 'DC_KANA_BI', 'ブ' => 'DC_KANA_BU', 'ベ' => 'DC_KANA_BE', 'ボ' => 'DC_KANA_BO',
    'パ' => 'DC_KANA_PA', 'ピ' => 'DC_KANA_PI', 'プ' => 'DC_KANA_PU', 'ペ' => 'DC_KANA_PE', 'ポ' => 'DC_KANA_PO',
    'マ' => 'DC_KANA_MA', 'ミ' => 'DC_KANA_MI', 'ム' => 'DC_KANA_MU', 'メ' => 'DC_KANA_ME', 'モ' => 'DC_KANA_MO',
    'ヤ' => 'DC_KANA_YA', 'ユ' => 'DC_KANA_YU', 'ヨ' => 'DC_KANA_YO',
    'ャ' => 'DC_KANA_XYA', 'ュ' => 'DC_KANA_XYU', 'ョ' => 'DC_KANA_XYO',
    'ラ' => 'DC_KANA_RA', 'リ' => 'DC_KANA_RI', 'ル' => 'DC_KANA_RU', 'レ' => 'DC_KANA_RE', 'ロ' => 'DC_KANA_RO',
    'ワ' => 'DC_KANA_WA', 'ヲ' => 'DC_KANA_WO', 'ン' => 'DC_KANA_N',

    'あ' => 'DC_KANA_A',  'い' => 'DC_KANA_I',   'う' => 'DC_KANA_U',   'え' => 'DC_KANA_E',  'お' => 'DC_KANA_O',
    'ぁ' => 'DC_KANA_XA', 'ぃ' => 'DC_KANA_XI',  'ぅ' => 'DC_KANA_XU',  'ぇ' => 'DC_KANA_XE', 'ぉ' => 'DC_KANA_XO',
    'か' => 'DC_KANA_KA', 'き' => 'DC_KANA_KI',  'く' => 'DC_KANA_KU',  'け' => 'DC_KANA_KE', 'こ' => 'DC_KANA_KO',
    'が' => 'DC_KANA_GA', 'ぎ' => 'DC_KANA_GI',  'ぐ' => 'DC_KANA_GU',  'げ' => 'DC_KANA_GE', 'ご' => 'DC_KANA_GO',
    'さ' => 'DC_KANA_SA', 'し' => 'DC_KANA_SHI', 'す' => 'DC_KANA_SU',  'せ' => 'DC_KANA_SE', 'そ' => 'DC_KANA_SO',
    'ざ' => 'DC_KANA_ZA', 'じ' => 'DC_KANA_ZI',  'ず' => 'DC_KANA_ZU',  'ぜ' => 'DC_KANA_ZE', 'ぞ' => 'DC_KANA_ZO',
    'た' => 'DC_KANA_TA', 'ち' => 'DC_KANA_CHI', 'つ' => 'DC_KANA_TSU', 'て' => 'DC_KANA_TE', 'と' => 'DC_KANA_TO',
    'だ' => 'DC_KANA_DA', 'ぢ' => 'DC_KANA_DI',  'づ' => 'DC_KANA_DU',  'で' => 'DC_KANA_DE', 'ど' => 'DC_KANA_DO',
    'っ' => 'DC_KANA_XTSU',
    'な' => 'DC_KANA_NA', 'に' => 'DC_KANA_NI', 'ぬ' => 'DC_KANA_NU', 'ね' => 'DC_KANA_NE', 'の' => 'DC_KANA_NO',
    'は' => 'DC_KANA_HA', 'ひ' => 'DC_KANA_HI', 'ふ' => 'DC_KANA_FU', 'へ' => 'DC_KANA_HE', 'ほ' => 'DC_KANA_HO',
    'ば' => 'DC_KANA_BA', 'び' => 'DC_KANA_BI', 'ぶ' => 'DC_KANA_BU', 'べ' => 'DC_KANA_BE', 'ぼ' => 'DC_KANA_BO',
    'ぱ' => 'DC_KANA_PA', 'ぴ' => 'DC_KANA_PI', 'ぷ' => 'DC_KANA_PU', 'ぺ' => 'DC_KANA_PE', 'ぽ' => 'DC_KANA_PO',
    'ま' => 'DC_KANA_MA', 'み' => 'DC_KANA_MI', 'む' => 'DC_KANA_MU', 'め' => 'DC_KANA_ME', 'も' => 'DC_KANA_MO',
    'や' => 'DC_KANA_YA', 'ゆ' => 'DC_KANA_YU', 'よ' => 'DC_KANA_YO',
    'ゃ' => 'DC_KANA_XYA', 'ゅ' => 'DC_KANA_XYU', 'ょ' => 'DC_KANA_XYO',
    'ら' => 'DC_KANA_RA', 'り' => 'DC_KANA_RI', 'る' => 'DC_KANA_RU', 'れ' => 'DC_KANA_RE', 'ろ' => 'DC_KANA_RO',
    'わ' => 'DC_KANA_WA', 'を' => 'DC_KANA_WO', 'ん' => 'DC_KANA_N',

    '「' => 'DC_L_BRACKET', '」' => 'DC_R_BRACKET', 'ー' => 'DC_KANA_HYPHEN',
    '、' => 'DC_KUTEN',     '。' => 'DC_TOUTEN',
    '゛' => 'DC_DAKUTEN',   '゜' => 'DC_HANDAKUTEN',

    '{1dot}'   => 'DC_1DOT',
    '{↓}'     => 'DC_CURSOR_DOWN', '{↑}' => 'DC_CURSOR_UP', '{→}' => 'DC_CURSOR_RIGHT', '{←}' => 'DC_CURSOR_LEFT',
    '{man↓}'  => 'DC_MAN_DOWN', '{man↑}' => 'DC_MAN_UP', '{man→}' => 'DC_MAN_RIGHT', '{man←}' => 'DC_MAN_LEFT',
    '{ufo}'    => 'DC_UFO',      '{snake}' => 'DC_SNAKE',
    '{nicochan1}' => 'DC_NICOCHAN_1', '{nicochan0}' => 'DC_NICOCHAN_0',

    '日' => 'DC_KANJI_SUN', '月' => 'DC_KANJI_MON', '火' => 'DC_KANJI_TUE',
    '水' => 'DC_KANJI_WED', '木' => 'DC_KANJI_THU', '金' => 'DC_KANJI_FRI',
    '土' => 'DC_KANJI_SAT', '生' => 'DC_KANJI_LIVE', '年' => 'DC_KANJI_YEAR',
    '時' => 'DC_KANJI_HOUR', '分' => 'DC_KANJI_MIN', '秒' => 'DC_KANJI_SEC',
    '円' => 'DC_KANJI_YEN',

    '{caps}' => 'DC_CAPS',
];


// -------------------------------- 解析
$inText = file_get_contents($inTextFilename);
$inText = str_replace(array("\r\n", "\r", "\n"), "\n", $inText);    // 改行の統一
$inText = rtrim($inText); // 最後の改行コード等はカット

// 1 文字 1 文字読む`
$arr = [];
$len = 0;
$tag = '';          // タグモード       '{..}' 文字列
$nrLfs = 0;         // 改行モード       連続する改行数
$bCaps = false;     // CAPS モード(false/true = AZカナ/azかな)
$fgColor = -1;      // 文字の色. 初期値は「前の状態を継承」
$bgColor = -1;      // 背景の色. 初期値は「前の状態を継承」
$nrErrs = 0;        // エラー数
for ($i = 0; $i < strlen($inText); $i++) {
    $c = mb_substr($inText, $i, 1);
    if ($c === '') {
        continue;
    }
    if ($c === chr(0x0d)) {// CR
        continue;
    }

    //echo "[$c]\n"; continue;

    if ($tag !== '') {// タグ モード
        $tag .= $c;
        if ($c !== '}') {
            continue;
        }
        // タグ モード終了
        if (strpos($tag, '{col=') === 0) {
            if (checkTagColor($tag, $fgColor, $bgColor, $bCaps, $arr, $len) === false) {
                $nrErrs++;
            }
            $tag = '';
            continue;
        } else if (strpos($tag, '{moveRight=') === 0) {
            if (checkTagMoveRight($tag, $arr, $len) === false) {
                $nrErrs++;
            }
            $tag = '';
            continue;
        } else if (strpos($tag, '{moveDown=') === 0) {
            if (checkTagMoveDown($tag, $arr, $len) === false) {
                $nrErrs++;
            }
            $tag = '';
            continue;
        }
        if (isset(TAB[$tag])) {
            $arr[] = TAB[$tag];
            $len++;
        } else {
            $nrErrs++;
            fwrite(STDERR, "ERROR: The $tag tag is not exist\n");
        }
        $tag = '';
    } else {
        if ($c === chr(0x0a)) {// LF
            if ($nrLfs === 0) {// 改行モード開始
                $nrLfs++;
            } else {
                $arr[] = 'DC_LF2';
                $nrLfs = 0;
            }
            continue;
        } else if ($nrLfs !== 0) {// 改行モード終了
            $arr[] = 'DC_LF';
            $nrLfs = 0;
        }
        if ($c === '{') {// タグ モード開始
            $tag = $c;
            continue;
        } else {
            // CAPS モード
            if ($bCaps) {
                if (preg_match('/[A-Zァ-ン]/u', $c)) {
                    $arr[] = 'DC_CAPS';
                    $bCaps = false;
                }
            } else {
                if (preg_match('/[a-zぁ-ん]/u', $c)) {
                    $arr[] = 'DC_CAPS';
                    $bCaps = true;
                }
            }

            //echo var_export($bCaps, true)."[$c]\n";
            if (isset(TAB[$c])) {
                $arr[] = TAB[$c];
                $len++;
                if (preg_match('/[がぎぐげござじずぜぞだぢづでどばびぶべぼぱぴぷぺぽヴガギグゲゴザジズゼドダヂヅデドバビブベボパピプペポ]/u', $c)) {
                    $len++;
                }
            } else {
                fwrite(STDERR, "ERROR: The character [$c] is not exist\n");
                $nrErrs++;
            }
            continue;
        }
    }
}
if ($nrErrs) {
    exit(1);
}

optimizeAtbCaps($arr);
optimizeAtb($arr, '0x40', 'DC_COL4');
optimizeAtb($arr, '0x50', 'DC_COL5');
optimizeAtb($arr, '0x60', 'DC_COL6');
optimizeAtb($arr, '0x70', 'DC_COL7');
outData($inText, $outFilename, $arr, $len);

// -------------------------------- 引数を持つタグの処理関数
function checkTagColor(string $tag, int &$fgColor, int &$bgColor, bool $bCaps, array &$arr, int &$len): bool
{
    $matches = [];
    if (preg_match('/^\{col=([0-9]),([0-9])\}$/', $tag, $matches) &&
        (count($matches) == 3) &&
        (0 <= $matches[1]) && ($matches[1] <= 7) &&
        (0 <= $matches[2]) && ($matches[2] <= 7)
    ) {
        $arr[] = 'DC_ATB';
        $fgColor = (int)$matches[1];
        $bgColor = (int)$matches[2];
        $atb = ($fgColor << 4) | ($bCaps ? 0x80 : 0x00) | $bgColor;
        $arr[] = sprintf('0x%02x', $atb);
        return true;
    }
    if (preg_match('/^\{col=([0-9])\}$/', $tag, $matches) &&
        (count($matches) == 2) &&
        (0 <= $matches[1]) && ($matches[1] <= 7)
    ) {
        $arr[] = 'DC_ATB';
        $fgColor = (int)$matches[1];
        $bgColor = 0;
        $atb = ($fgColor << 4) | ($bCaps ? 0x80 : 0x00);
        $arr[] = sprintf('0x%02x', $atb);
        return true;
    }

    //print_r($matches);
    fwrite (STDERR, "ERROR: Invalid tag [$tag]\n");
    return false;
}

function checkTagMoveRight(string $tag, array &$arr, int &$len): bool
{
    $matches = [];
    if (!preg_match('/^\{moveRight=([0-9]{1,2})\}$/', $tag, $matches) ||
        (count($matches) != 2) ||
        ($matches[1] < 0) || (40 < $matches[1])) {
        //echo("tag:$tag match:$matches[1]\n");
        fwrite (STDERR, "ERROR: Invalid tag [$tag]\n");
        return false;
    }
    $arr[] = 'DC_MOVE_RIGHT';
    $arr[] = sprintf('0x%02x', $matches[1]);
    $len += $matches[1];
    return true;
}

function checkTagMoveDown(string $tag, array &$arr, int &$len): bool
{
    $matches = [];
    if (!preg_match('/^\{moveDown=([0-9]{1,2})\}$/', $tag, $matches) ||
        (count($matches) != 2) ||
        ($matches[1] < 0) || (25 < $matches[1])) {
        //print_r($matches);
        fwrite (STDERR, "ERROR: Invalid tag [$tag]\n");
        return false;
    }
    $arr[] = 'DC_MOVE_DOWN';
    $arr[] = sprintf('0x%02x', $matches[1]);
    //$len += $matches[1];
    return true;
}


// -------------------------------- 最適化
/** atb と caps の最適化
 * DC_CAPS があれば, 前方に DC_ATB がないかサーチ.
 * サーチ中に英文字・カナ文字があるならば, サーチはキャンセル.
 * サーチ中に先頭にまで達したら,サーチはキャンセル.
 * DC_ATB が見つかったら, 次の属性値を修正して, DC_CAPS は削除
 */
function optimizeAtbCaps(array &$arr): void
{
    foreach ($arr as $i => $char) {
        if ($char === 'DC_CAPS') {
            //echo("caps detected $i\n");

            for ($j = $i - 1; 0 <= $j; $j--) {
                if (!isset($arr[$j])) {
                    continue;
                } else if ($arr[$j] === 'DC_ATB') {
                    //echo("atb detected $j\n");
                    $val = 0;
                    sscanf($arr[$j + 1], '0x%02x', $val);
                    $arr[$j + 1] = sprintf('0x%02x', $val ^ 0x80);
                    unset($arr[$i]);
                    //$arr[$i] = '-----'.sprintf('0x%02x', $val ^ 0x80);//TEST
                    break;
                } else if (preg_match('/^DC_[A-Z]$/', $arr[$j]) || preg_match('/^DC_KANA_[A-Z]+$/', $arr[$j])) {
                    break;
                }
            }
        }
    }
}


/** a6b7 の最適化
 * DC_ATB 0x70 ならば, DC_COL7 に置き換えます
 */
function optimizeAtb(array &$arr, string $value, string $out): void
{
    foreach ($arr as $i => $char) {
        if (($char === 'DC_ATB') &&  ($arr[$i + 1] === $value)) {
            $arr[$i] = $out;
            unset($arr[$i + 1]);
        }
    }
}


// -------------------------------- 出力
function outData(string $inText, string $outFilename, array $arr, int $len): void
{
    // 安全のために 256 文字に抑えておく
    if (256 <= count($arr)) {
        fwrite(STDERR, "ERROR: Too many characters![" . count($arr) . "]\n");
        exit(1);
    }

    //print_r($arr);
    $textArr = explode("\n", $inText);
    $outStr  = '';
    foreach ($textArr as $t) {
        $outStr .= '// ' . $t . "\n";
    }
    $label = pathinfo($outFilename, PATHINFO_FILENAME);
    $outStr .= "static u8 const text_" . $label . "[] = { \n    ";
    foreach ($arr as $c) {
        $outStr .= $c . ', ';
        if ($c === 'DC_LF' || $c === 'DC_LF2') {
            $outStr .= "\n    ";
        }
    }
    $outStr .= "\n    0, };\n";
    $outStr .= "#define TEXT_" . strtoupper($label) . "_LEN $len\n"; // 特殊コードを除いた表示文字の長さ
    file_put_contents($outFilename, $outStr);

    //$macro = strtoupper(ltrim(strtolower(preg_replace('/[A-Z]/', '_\0', $outFilename)), '_')) . "_INCLUDED";// camelCase を SNAKE_CASE に
    //$outStr  = "#ifndef $macro\n";
    //$outStr .= "#define $macro\n";
    //$outStr .= "extern u8 text" . $outFilename . "[];\n";
    //$outStr .= "#endif // $macro\n";
    //file_put_contents($outFilename . ".h", $outStr);
}
