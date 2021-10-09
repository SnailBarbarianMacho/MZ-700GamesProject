<?php

declare(strict_types = 1);
/**
 * テキスト文字列をディスプレイ コードの配列に変換
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
 * @author Snail Barbarian Macho (NWK) 2021.09.08
 */

// --------------------------------
// 引数チェック
if (count($argv) != 3)
{
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " in.txt out.h\n");
    exit(1);
}
$inTextFilename = $argv[1];
$outCFilename   = $argv[2];

if (file_exists($inTextFilename) === false) {
    fwrite(STDERR, "file not found[$inTextFilename]\n");
    exit(1);
}

const TAB = [
    ' ' => 'CHAR_SP', '　' => 'CHAR_SP', '{sp}' => 'CHAR_SP',
    '{lf}' => 'CHAR_LF', '{lf2}' => 'CHAR_LF2',

    'A' => 'CHAR_A', 'B' => 'CHAR_B', 'C' => 'CHAR_C', 'D' => 'CHAR_D', 'E' => 'CHAR_E',
    'F' => 'CHAR_F', 'G' => 'CHAR_G', 'H' => 'CHAR_H', 'I' => 'CHAR_I', 'J' => 'CHAR_J',
    'K' => 'CHAR_K', 'L' => 'CHAR_L', 'M' => 'CHAR_M', 'N' => 'CHAR_N', 'O' => 'CHAR_O',
    'P' => 'CHAR_P', 'Q' => 'CHAR_Q', 'R' => 'CHAR_R', 'S' => 'CHAR_S', 'T' => 'CHAR_T',
    'U' => 'CHAR_U', 'V' => 'CHAR_V', 'W' => 'CHAR_W', 'X' => 'CHAR_X', 'Y' => 'CHAR_Y',
    'Z' => 'CHAR_Z',
    'a' => 'CHAR_A', 'b' => 'CHAR_B', 'c' => 'CHAR_C', 'd' => 'CHAR_D', 'e' => 'CHAR_E',
    'f' => 'CHAR_F', 'g' => 'CHAR_G', 'h' => 'CHAR_H', 'i' => 'CHAR_I', 'j' => 'CHAR_J',
    'k' => 'CHAR_K', 'l' => 'CHAR_L', 'm' => 'CHAR_M', 'n' => 'CHAR_N', 'o' => 'CHAR_O',
    'p' => 'CHAR_P', 'q' => 'CHAR_Q', 'r' => 'CHAR_R', 's' => 'CHAR_S', 't' => 'CHAR_T',
    'u' => 'CHAR_U', 'v' => 'CHAR_V', 'w' => 'CHAR_W', 'x' => 'CHAR_X', 'y' => 'CHAR_Y',
    'z' => 'CHAR_Z',
    '0' => 'CHAR_0', '1' => 'CHAR_1', '2' => 'CHAR_2', '3' => 'CHAR_3', '4' => 'CHAR_4',
    '5' => 'CHAR_5', '6' => 'CHAR_6', '7' => 'CHAR_7', '8' => 'CHAR_8', '9' => 'CHAR_9',
    '+' => 'CHAR_PLUS',   '-' => 'CHAR_HYPHEN', '*' => 'CHAR_ASTERISK', '/' => 'CHAR_SLASH',
    '#' => 'CHAR_NUMBER', '$' => 'CHAR_DOLLER', '%' => 'CHAR_PERCENT',  '&' => 'CHAR_AMPERSAND',
    '=' => 'CHAR_EQUALS',        '@' => 'CHAR_AT',
    ':' => 'CHAR_COLON',         ';' => 'CHAR_SEMICOLON',
    '.' => 'CHAR_PERIOD',        ',' => 'CHAR_COMMA',
    '?' => 'CHAR_QUESTIO',       '？' => 'CHAR_QUESTIO',
    '!' => 'CHAR_EXCLAMATION',   '！' => 'CHAR_EXCLAMATION',
    '"' => 'CHAR_QUOTATION',     "'" => 'CHAR_APOSTROPHE',
    '(' => 'CHAR_L_PARENTHESIS', ')' => 'CHAR_R_PARENTHESIS',
    '[' => 'CHAR_L_SQ_BLACKET',  ']' => 'CHAR_R_SQ_BLACKET',
    '<' => 'CHAR_LT',            '>' => 'CHAR_GT',
    'π' => 'CHAR_PI',           '￥' => 'CHAR_YEN',      '\\' => 'CHAR_BACK_SLASH',
    '←' => 'CHAR_R_ARROW',      '↑' => 'CHAR_U_ARROW',

    '■' => 'CHAR_SQUARE',
    '♠' => 'CHAR_SPADE', '♦' => 'CHAR_DIAMOND', '♣' => 'CHAR_CLUB', '♥' => 'CHAR_HEART',

    'ア' => 'CHAR_KANA_A',  'イ' => 'CHAR_KANA_I',  'ウ' => 'CHAR_KANA_U',  'エ' => 'CHAR_KANA_E',  'オ' => 'CHAR_KANA_O',
    'ァ' => 'CHAR_KANA_XA', 'ィ' => 'CHAR_KANA_XI', 'ゥ' => 'CHAR_KANA_XU', 'ェ' => 'CHAR_KANA_XE', 'ォ' => 'CHAR_KANA_XO',
    'ヴ' => 'CHAR_KANA_VU',
    'カ' => 'CHAR_KANA_KA', 'キ' => 'CHAR_KANA_KI',  'ク' => 'CHAR_KANA_KU',  'ケ' => 'CHAR_KANA_KE', 'コ' => 'CHAR_KANA_KO',
    'ガ' => 'CHAR_KANA_GA', 'ギ' => 'CHAR_KANA_GI',  'グ' => 'CHAR_KANA_GU',  'ゲ' => 'CHAR_KANA_GE', 'ゴ' => 'CHAR_KANA_GO',
    'サ' => 'CHAR_KANA_SA', 'シ' => 'CHAR_KANA_SHI', 'ス' => 'CHAR_KANA_SU',  'セ' => 'CHAR_KANA_SE', 'ソ' => 'CHAR_KANA_SO',
    'ザ' => 'CHAR_KANA_ZA', 'ジ' => 'CHAR_KANA_ZI',  'ズ' => 'CHAR_KANA_ZU',  'ゼ' => 'CHAR_KANA_ZE', 'ゾ' => 'CHAR_KANA_ZO',
    'タ' => 'CHAR_KANA_TA', 'チ' => 'CHAR_KANA_CHI', 'ツ' => 'CHAR_KANA_TSU', 'テ' => 'CHAR_KANA_TE', 'ト' => 'CHAR_KANA_TO',
    'ダ' => 'CHAR_KANA_DA', 'ヂ' => 'CHAR_KANA_DI',  'ヅ' => 'CHAR_KANA_DU',  'デ' => 'CHAR_KANA_DE', 'ド' => 'CHAR_KANA_DO',
    'ッ' => 'CHAR_KANA_XTSU',
    'ナ' => 'CHAR_KANA_NA', 'ニ' => 'CHAR_KANA_NI', 'ヌ' => 'CHAR_KANA_NU', 'ネ' => 'CHAR_KANA_NE', 'ノ' => 'CHAR_KANA_NO',
    'ハ' => 'CHAR_KANA_HA', 'ヒ' => 'CHAR_KANA_HI', 'フ' => 'CHAR_KANA_FU', 'ヘ' => 'CHAR_KANA_HE', 'ホ' => 'CHAR_KANA_HO',
    'バ' => 'CHAR_KANA_BA', 'ビ' => 'CHAR_KANA_BI', 'ブ' => 'CHAR_KANA_BU', 'ベ' => 'CHAR_KANA_BE', 'ボ' => 'CHAR_KANA_BO',
    'パ' => 'CHAR_KANA_PA', 'ピ' => 'CHAR_KANA_PI', 'プ' => 'CHAR_KANA_PU', 'ペ' => 'CHAR_KANA_PE', 'ポ' => 'CHAR_KANA_PO',
    'マ' => 'CHAR_KANA_MA', 'ミ' => 'CHAR_KANA_MI', 'ム' => 'CHAR_KANA_MU', 'メ' => 'CHAR_KANA_ME', 'モ' => 'CHAR_KANA_MO',
    'ヤ' => 'CHAR_KANA_YA', 'ユ' => 'CHAR_KANA_YU', 'ヨ' => 'CHAR_KANA_YO',
    'ャ' => 'CHAR_KANA_XYA', 'ュ' => 'CHAR_KANA_XYU', 'ョ' => 'CHAR_KANA_XYO',
    'ラ' => 'CHAR_KANA_RA', 'リ' => 'CHAR_KANA_RI', 'ル' => 'CHAR_KANA_RU', 'レ' => 'CHAR_KANA_RE', 'ロ' => 'CHAR_KANA_RO',
    'ワ' => 'CHAR_KANA_WA', 'ヲ' => 'CHAR_KANA_WO', 'ン' => 'CHAR_KANA_N',

    'あ' => 'CHAR_KANA_A',  'い' => 'CHAR_KANA_I',   'う' => 'CHAR_KANA_U',   'え' => 'CHAR_KANA_E',  'お' => 'CHAR_KANA_O',
    'ぁ' => 'CHAR_KANA_XA', 'ぃ' => 'CHAR_KANA_XI',  'ぅ' => 'CHAR_KANA_XU',  'ぇ' => 'CHAR_KANA_XE', 'ぉ' => 'CHAR_KANA_XO',
    'か' => 'CHAR_KANA_KA', 'き' => 'CHAR_KANA_KI',  'く' => 'CHAR_KANA_KU',  'け' => 'CHAR_KANA_KE', 'こ' => 'CHAR_KANA_KO',
    'が' => 'CHAR_KANA_GA', 'ぎ' => 'CHAR_KANA_GI',  'ぐ' => 'CHAR_KANA_GU',  'げ' => 'CHAR_KANA_GE', 'ご' => 'CHAR_KANA_GO',
    'さ' => 'CHAR_KANA_SA', 'し' => 'CHAR_KANA_SHI', 'す' => 'CHAR_KANA_SU',  'せ' => 'CHAR_KANA_SE', 'そ' => 'CHAR_KANA_SO',
    'ざ' => 'CHAR_KANA_ZA', 'じ' => 'CHAR_KANA_ZI',  'ず' => 'CHAR_KANA_ZU',  'ぜ' => 'CHAR_KANA_ZE', 'ぞ' => 'CHAR_KANA_ZO',
    'た' => 'CHAR_KANA_TA', 'ち' => 'CHAR_KANA_CHI', 'つ' => 'CHAR_KANA_TSU', 'て' => 'CHAR_KANA_TE', 'と' => 'CHAR_KANA_TO',
    'だ' => 'CHAR_KANA_DA', 'ぢ' => 'CHAR_KANA_DI',  'づ' => 'CHAR_KANA_DU',  'で' => 'CHAR_KANA_DE', 'ど' => 'CHAR_KANA_DO',
    'っ' => 'CHAR_KANA_XTSU',
    'な' => 'CHAR_KANA_NA', 'に' => 'CHAR_KANA_NI', 'ぬ' => 'CHAR_KANA_NU', 'ね' => 'CHAR_KANA_NE', 'の' => 'CHAR_KANA_NO',
    'は' => 'CHAR_KANA_HA', 'ひ' => 'CHAR_KANA_HI', 'ふ' => 'CHAR_KANA_FU', 'へ' => 'CHAR_KANA_HE', 'ほ' => 'CHAR_KANA_HO',
    'ば' => 'CHAR_KANA_BA', 'び' => 'CHAR_KANA_BI', 'ぶ' => 'CHAR_KANA_BU', 'べ' => 'CHAR_KANA_BE', 'ぼ' => 'CHAR_KANA_BO',
    'ぱ' => 'CHAR_KANA_PA', 'ぴ' => 'CHAR_KANA_PI', 'ぷ' => 'CHAR_KANA_PU', 'ぺ' => 'CHAR_KANA_PE', 'ぽ' => 'CHAR_KANA_PO',
    'ま' => 'CHAR_KANA_MA', 'み' => 'CHAR_KANA_MI', 'む' => 'CHAR_KANA_MU', 'め' => 'CHAR_KANA_ME', 'も' => 'CHAR_KANA_MO',
    'や' => 'CHAR_KANA_YA', 'ゆ' => 'CHAR_KANA_YU', 'よ' => 'CHAR_KANA_YO',
    'ゃ' => 'CHAR_KANA_XYA', 'ゅ' => 'CHAR_KANA_XYU', 'ょ' => 'CHAR_KANA_XYO',
    'ら' => 'CHAR_KANA_RA', 'り' => 'CHAR_KANA_RI', 'る' => 'CHAR_KANA_RU', 'れ' => 'CHAR_KANA_RE', 'ろ' => 'CHAR_KANA_RO',
    'わ' => 'CHAR_KANA_WA', 'を' => 'CHAR_KANA_WO', 'ん' => 'CHAR_KANA_N',

    '「' => 'CHAR_L_BRACKET', '」' => 'CHAR_R_BRACKET', 'ー' => 'CHAR_HYPHEN',
    '、' => 'CHAR_KUTEN',     '。' => 'CHAR_TOUTEN',
    '゛' => 'CHAR_DAKUTEN',   '゜' => 'CHAR_HANDAKUTEN',

    '{small-}' => 'CHAR_SMALL_HYPHEN',
    '{1dot}'   => 'CHAR_1DOT',
    '{↓}'     => 'CHAR_CURSOR_DOWN', '{↑}' => 'CHAR_CURSOR_UP', '{→}' => 'CHAR_CURSOR_RIGHT', '{←}' => 'CHAR_CURSOR_LEFT',
    '{man↓}'  => 'CHAR_MAN_DOWN', '{man↑}' => 'CHAR_MAN_UP', '{man→}' => 'CHAR_MAN_RIGHT', '{man←}' => 'CHAR_MAN_LEFT',
    '{ufo}'    => 'CHAR_UFO',      '{snake}' => 'CHAR_SNAKE',
    '{nicochan1}' => 'CHAR_NICOCHAN_1', '{nicochan0}' => 'CHAR_NICOCHAN_0',

    '日' => 'CHAR_KNAJI_SUN', '月' => 'CHAR_KNAJI_MON', '火' => 'CHAR_KNAJI_TUE',
    '水' => 'CHAR_KNAJI_WED', '木' => 'CHAR_KNAJI_THU', '金' => 'CHAR_KNAJI_FRI',
    '土' => 'CHAR_KNAJI_SAT', '生' => 'CHAR_KNAJI_LIVE', '年' => 'CHAR_KNAJI_YEAR',
    '時' => 'CHAR_KNAJI_HOUR', '分' => 'CHAR_KNAJI_MIN', '秒' => 'CHAR_KNAJI_SEC',
    '円' => 'CHAR_KNAJI_YEN',

    '{caps}' => 'CHAR_CAPS',
];


// -------------------------------- 解析
$inText = file_get_contents($inTextFilename);
$inText = str_replace(array("\r\n", "\r", "\n"), "\n", $inText);    // 改行の統一
$inText = rtrim($inText); // 最後の改行コード等はカット

// 1 文字 1 文字読む`
$arr = [];
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
            if (checkTagColor($tag, $fgColor, $bgColor, $bCaps, $arr) === false) {
                $nrErrs++;
            }
            $tag = '';
            continue;
        } else if (strpos($tag, '{moveRight=') === 0) {
            if (checkTagMoveRight($tag, $arr) === false) {
                $nrErrs++;
            }
            $tag = '';
            continue;
        } else if (strpos($tag, '{moveDown=') === 0) {
            if (checkTagMoveDown($tag, $arr) === false) {
                $nrErrs++;
            }
            $tag = '';
            continue;
        }
        if (isset(TAB[$tag])) {
            $arr[] = TAB[$tag];
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
                $arr[] = 'CHAR_LF2';
                $nrLfs = 0;
            }
            continue;
        } else if ($nrLfs !== 0) {// 改行モード終了
            $arr[] = 'CHAR_LF';
            $nrLfs = 0;
        }
        if ($c === '{') {// タグ モード開始
            $tag = $c;
            continue;
        } else {
            // CAPS モード
            if ($bCaps) {
                if (preg_match('/[A-Zァ-ン]/u', $c)) {
                    $arr[] = 'CHAR_CAPS';
                    $bCaps = false;
                }
            } else {
                if (preg_match('/[a-zぁ-ん]/u', $c)) {
                    $arr[] = 'CHAR_CAPS';
                    $bCaps = true;
                }
            }

            //echo var_export($bCaps, true)."[$c]\n";
            if (isset(TAB[$c])) {
                $arr[] = TAB[$c];
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
optimizeAtb($arr, '0x40', 'CHAR_COL4');
optimizeAtb($arr, '0x50', 'CHAR_COL5');
optimizeAtb($arr, '0x60', 'CHAR_COL6');
optimizeAtb($arr, '0x70', 'CHAR_COL7');
outData($inText, $outCFilename, $arr);

// -------------------------------- 引数を持つタグの処理関数
function checkTagColor(string $tag, int &$fgColor, int &$bgColor, bool $bCaps, array &$arr): bool
{
    $matches = [];
    if (preg_match('/^\{col=([0-9]),([0-9])\}$/', $tag, $matches) &&
        (count($matches) == 3) &&
        (0 <= $matches[1]) && ($matches[1] <= 7) &&
        (0 <= $matches[2]) && ($matches[2] <= 7)
    ) {
        $arr[] = 'CHAR_ATB';
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
        $arr[] = 'CHAR_ATB';
        $fgColor = (int)$matches[1];
        $bgColor = 0;
        $atb = ($fgColor << 4) | ($bCaps ? 0x80 : 0x00);
        $arr[] = sprintf('0x%02x', $atb);
        return true;
    }

    //print_r($matches);
    fwrite (STDERR, "ERROR: Invalid tag $tag\n");
    return false;
}

function checkTagMoveRight(string $tag, array &$arr): bool
{
    $matches = [];
    if (!preg_match('/^\{moveRight=([0-9]{1,2})\}$/', $tag, $matches) ||
        (count($matches) != 2) ||
        ($matches[1] < 0) || (40 < $matches[1])) {
        //echo("tag:$tag match:$matches[1]\n");
        fwrite (STDERR, "ERROR: Invalid tag $tag\n");
        return false;
    }
    $arr[] = 'CHAR_MOVE_RIGHT';
    $arr[] = sprintf('0x%02x', $matches[1]);
    return true;
}

function checkTagMoveDown(string $tag, array &$arr): bool
{
    $matches = [];
    if (!preg_match('/^\{moveDown=([0-9]{1,2})\}$/', $tag, $matches) ||
        (count($matches) != 2) ||
        ($matches[1] < 0) || (25 < $matches[1])) {
        //print_r($matches);
        fwrite (STDERR, "ERROR: Invalid tag $tag\n");
        return false;
    }
    $arr[] = 'CHAR_MOVE_DOWN';
    $arr[] = sprintf('0x%02x', $matches[1]);
    return true;
}


// -------------------------------- 最適化
/** atb と caps の最適化
 * CHAR_CAPS があれば, 前方に CHAR_ATB がないかサーチ.
 * サーチ中に英文字・カナ文字があるならば, サーチはキャンセル.
 * サーチ中に先頭にまで達したら,サーチはキャンセル.
 * CHAR_ATB が見つかったら, 次の属性値を修正して, CHAR_CAPS は削除
 */
function optimizeAtbCaps(array &$arr): void
{
    foreach ($arr as $i => $char) {
        if ($char === 'CHAR_CAPS') {
            //echo("caps detected $i\n");

            for ($j = $i - 1; 0 <= $j; $j--) {
                if (!isset($arr[$j])) {
                    continue;
                } else if ($arr[$j] === 'CHAR_ATB') {
                    //echo("atb detected $j\n");
                    $val = 0;
                    sscanf($arr[$j + 1], '0x%02x', $val);
                    $arr[$j + 1] = sprintf('0x%02x', $val ^ 0x80);
                    unset($arr[$i]);
                    //$arr[$i] = '-----'.sprintf('0x%02x', $val ^ 0x80);//TEST
                    break;
                } else if (preg_match('/^CHAR_[A-Z]$/', $arr[$j]) || preg_match('/^CHAR_KANA_[A-Z]+$/', $arr[$j])) {
                    break;
                }
            }
        }
    }
}


/** a6b7 の最適化
 * CHAR_ATB 0x70 ならば, CHAR_COL7 に置き換えます
 */
function optimizeAtb(array &$arr, string $value, string $out): void
{
    foreach ($arr as $i => $char) {
        if (($char === 'CHAR_ATB') &&  ($arr[$i + 1] === $value)) {
            $arr[$i] = $out;
            unset($arr[$i + 1]);
        }
    }
}


// -------------------------------- 出力
function outData(string $inText, string $outCFilename, array $arr): void
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
    $outStr .= 'static const u8 str_' . pathinfo($outCFilename, PATHINFO_FILENAME) . "[] = { \n    ";
    foreach ($arr as $c) {
        $outStr .= $c . ', ';
        if (($c === 'CHAR_LF') || ($c === 'CHAR_LF2')) {
            $outStr .= "\n    ";
        }
    }
    $outStr .= "\n    0, };";
    file_put_contents($outCFilename, $outStr);
}
