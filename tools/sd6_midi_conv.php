<?php

declare(strict_types = 1);
require_once 'nwk-classes/utils/utils.class.php';
require_once 'nwk-classes/utils/error.class.php';
require_once 'nwk-classes/sound/midi.class.php';
require_once 'nwk-classes/sound/score.class.php';
require_once 'nwk-classes/sound/score-utils.class.php';
require_once 'nwk-classes/sound/score-merge-utils.class.php';

/**
 * おざなり MIDI → sd6Play() 形式データ変換ツール. C ソースの形で出力します
 * 使い方は, Usage: 行を参照してください
 *
 * - ファイル:
 *   - MIDI FORMAT 1 のみ対応
 *   - midi ファイル名の末端が '_barX' ならば, X 小節単位に分割します
 *
 * - 制限:
 *   - tempo は 120 付近にしてください
 *   - 音調の最小分解能は 1/32 音符
 *   - 同時発生音は最大 6 音
 *
 *     トラックの  同時                 割当
 *     チャンネル  発音数   音階        Beeper 役割
 *   ------------------------------------------------------------
 *     ch1         0～1     R,B1～C6    #0～#1 Lead ※Beeper1は Beeper0 のエコー
 *     ch2         0～1     R,A1～B1    なし   LeadSub (スライド,スラー,エンベロープ速度の指定)
 *     ch3-4                                   欠番
 *     ch5         0～1     R,B1～C6    #2     Base
 *     ch6         0～2     R,B1～C6    #3～#4 Chord x 2
 *     ch7-9                                   欠番
 *     ch10        0～1     R,B1～C#2   #5     Drum
 *
 *   - 音階についての注意:
 *     - C5 以降の高音は精度落ちるので実用は3オクターブ程度
 *     - 絶対音階ではありません (A4 == 440Hz ではありません)
 *   - 音色: 主に, 音色でエンベロープを指定します
 *           Lead    ... 音色でエンベロープ: Piano1/2/3 = エンベロープ■/◣/◢
 *           LeadSub ... Leadと同じ位置・長さの音符で指定します(一致しない場合はエラー),
 *                       音の高さでいろいろ指定します
 *                       - Leadエンベロープ=■の時:
 *                         休符/A1/A#1/B1 = スライド無/スライド低速/x2/x4
 *                       - Leadエンベロープ=◣◢の時:
 *                         休符/A1/A#1/B1 = 前回と同じ速度/エンベロープ低速/x2/x4
 *           Base    ... Piano1/2/3 = エンベロープ■/◣/◢
 *           Chord   ... Piano1/2/3 = エンベロープ■/◣/◢
 *                       2音の場合, 2つの音符は同じ位置/長さでなければなりません
 *           Drum    ... AcouBassDrum(B1)/BassDrum1(C2)/SideStick(C#2) = ドラム1/2/3
 *   - 音長: 32分音符～全音符x2. 単位は32分音符で自由に決められます
 *           特別な命令に使われる音長があります. この音長の音符は使えません --exceptLength で指定してください
 *   - ノード, コントロール以外の殆どのイベントは解析しません (スキップします)
 *   - 他形式との互換性
 *     - SD4:エンベロープ速度のみ再調整が必要です
 * - DOMINO での便利な使い方:
 *   - オニオンスキンの設定:
 *     - [表示]->[トラックリストペイン]を表示し, 各トラックのプロパティでオニオンスキンの色を決めます
 *     - トラックを選び, 右上の[オニオン]->[ピアノロールに指定のトラックを表示]で好きなトラックを表示できます
 *   - 音色の変更は[挿入]->[プログラム チェンジ]
 *   - 音符の移動はコピペで
 *   - 曲の終了後に謎の無音が続く場合は, [イベント]->[END OF TRACK] を入れる
 * - 例:
 *   php $argv[0] music.mid music.h --exceptLengths=200#20
 *   php $argv[0] music.mid music.h --bars=55-60 --ch=0
 *
 * @author Snail Barbarian Macho (NWK)
 * - 2024.08.25 最初のバージョン
 * - 2024.10.18 リピート対応
 */

/* -------- SD6 データ 仕様書
 * - スペック:
 *   - 最も内側の Beeper ループの周期は, 大半が最短 252 T-states
 *     - サンプリング周波数:    3579545 / 246 = 14551.0 Hz
 *     - 1分間の四分音符の数♩: 60 * (3579545 / 246) / (256 * 32) = 106.3 (テンポは倍くらいまでいける)
 *
 * - 音符データ (基本)
 *   - 全トラックのデータが一本のデータにまとまってます
 *
 * - 音譜データ (凡例)
 *   - len 音長 ... [1, 2, ...  63, 0] → [4, 8, ... 252, 256音長].
 *     32分音符 = 1, 四分音符 = 8, 全音符 = 32, 全音符 x 8 = 256
 *   - wl  波長 ... [1, 2, ... 255, 0] → [1, 2, ... 255, 256波長]
 *
 * - 音符データ (Lead)
 *   - 一般
 *                      +-+-+-+-+-+-+-+-+
 *     休符             |0 0|    len    |
 *                      +-+-+-+-+-+-+-+-+
 *                      +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *     ■               |1 1|    len    |  |       wl      |
 *                      +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *                      +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *     ◢               |0 1|    len    |  |       wl      |
 *                      +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *                      +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *     ◣               |1 0|    len    |  |       wl      |
 *                      +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   - Lead の特定の音長は命令になります. 音長値は, SD6_EXCEPT_LEN#SD6_EXCEPT_LEN_SZ で定義
 *                      +-+-+-+-+-+-+-+-+
 *     リピート         |0 0|  [e, e+6] |  2～7回,5ネスト迄
 *                      +-+-+-+-+-+-+-+-+
 *                      +-+-+-+-+-+-+-+-+
 *     リピート終了     |0 1|     e     |
 *                      +-+-+-+-+-+-+-+-+
 *                      +-+-+-+-+-+-+-+-+
 *     終了             |0 1|     e+1   |
 *                      +-+-+-+-+-+-+-+-+
 *                      +-+-+-+-+-+-+-+-+  ◢◣用
 *     エンベロープ速度 |1 0|  [e, e+3] |  e/e+1/e+2/e+3 = 1/2, 1/4, 1/8(デフォルト), 1/16
 *                      +-+-+-+-+-+-+-+-+
 *                      +-+-+-+-+-+-+-+-+  ■用. スライド時は, 波長カウンタのリセットや音量の再設定はしません
 *     スライド速度     |1 1|  [e, e+3] |  e/e+1/e+2 = スライド無(デフォルト), 1/1, 1/2, 1/3
 *                      +-+-+-+-+-+-+-+-+
 *
 * - 音符データ (Chord x 2)
 *          +-+-+-+-+-+-+-+-+
 *   休符   |0 0|    len    |
 *          +-+-+-+-+-+-+-+-+
 *          +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ◢     |0 1|    len    |  |       wl0     |  |       wl1     |
 *          +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *          +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ◣     |1 0|    len    |  |       wl0     |  |       wl1     |
 *          +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *          +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ■     |1 1|    len    |  |       wl0     |  |       wl1     |
 *          +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *
 * - 音符データ (Base)
 *          +-+-+-+-+-+-+-+-+
 *   休符   |0 0|    len    |
 *          +-+-+-+-+-+-+-+-+
 *          +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ◢     |0 1|    len    |  |       wl      |
 *          +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *          +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ◣     |1 0|    len    |  |       wl      |
 *          +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *          +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ■     |1 1|    len    |  |       wl      |
 *          +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *
 * - 音符データ (Drum)
 *                  +-+-+-+-+-+-+-+-+
 *   休符/ドラム    |   len     |d d|      d ... ドラム番号[1～3]. 0 は休符
 *                  +-+-+-+-+-+-+-+-+
 */

// --------------------------------
// サポートするトラック,チャンネル,音源,音階のテーブル
const SCALE_LEAD_TAB = [
    'B1',
    'C2',  'C#2',  'D2',  'D#2',  'E2',  'F2',  'F#2',  'G2',  'G#2',  'A2',  'A#2',  'B2',
    'C3',  'C#3',  'D3',  'D#3',  'E3',  'F3',  'F#3',  'G3',  'G#3',  'A3',  'A#3',  'B3',
    'C4',  'C#4',  'D4',  'D#4',  'E4',  'F4',  'F#4',  'G4',  'G#4',  'A4',  'A#4',  'B4',
    'C5',  'C#5',  'D5',  'D#5',  'E5',  'F5',  'F#5',  'G5',  'G#5',  'A5',  'A#5',  'B5',
    'C6',  \nwk\sound\Note::SCALE_R,
];
const SCALE_BASE_TAB = [
    'B1',
    'C2',  'C#2',  'D2',  'D#2',  'E2',  'F2',  'F#2',  'G2',  'G#2',  'A2',  'A#2',  'B2',
    'C3',  'C#3',  'D3',  'D#3',  'E3',  'F3',  'F#3',  'G3',  'G#3',  'A3',  'A#3',  'B3',
    'C4',  'C#4',  'D4',  'D#4',  'E4',  'F4',  'F#4',  'G4',  'G#4',  'A4',  'A#4',  'B4',
    'C5',  'C#5',  'D5',  'D#5',  'E5',  'F5',  'F#5',  'G5',  'G#5',  'A5',  'A#5',  'B5',
    'C6',  \nwk\sound\Note::SCALE_R,
];
const SCALE_SLIDE_TAB = [ // エンベロープ/スライド速度
    'G#1', 'A1',  'A#1',  'B1', \nwk\sound\Note::SCALE_R,
];
const SCALE_DRUM_TAB = [ // ドラム0/1/2
    'B1',  'C2',  'C#2', \nwk\sound\Note::SCALE_R,
];
const TONE_TAB = [
    \nwk\sound\Note::TONE_PIANO1, \nwk\sound\Note::TONE_PIANO2, \nwk\sound\Note::TONE_PIANO3,  // それぞれ, エンベロープ ■, ◣, ◢
];
const TONE_SUB_TAB = [
    \nwk\sound\Note::TONE_PIANO1,
];
const TONE_DRUM_TAB = [
    \nwk\sound\Note::TONE_PIANO1,
];

// チャンネル
const CH_LEAD     = 0;
const CH_LEAD_SUB = 1;
const CH_BASE     = 4;
const CH_CHORD    = 5;
const CH_DRUM     = \nwk\sound\midiReader::CH_DRUM;
const CHANNELS = [CH_LEAD, CH_LEAD_SUB, CH_BASE, CH_CHORD, CH_DRUM];

const TRACK_TAB = [
    CH_LEAD     => ['nr_sources' => 1, 'tones' => TONE_TAB,      'scales' => SCALE_LEAD_TAB,  'name' => 'Lead'],
    CH_LEAD_SUB => ['nr_sources' => 1, 'tones' => TONE_SUB_TAB,  'scales' => SCALE_SLIDE_TAB, 'name' => 'LeadSub'],
    CH_BASE     => ['nr_sources' => 1, 'tones' => TONE_TAB,      'scales' => SCALE_BASE_TAB,  'name' => 'Base'],
    CH_CHORD    => ['nr_sources' => 2, 'tones' => TONE_TAB,      'scales' => SCALE_BASE_TAB,  'name' => 'Chord0/1'],
    CH_DRUM     => ['nr_sources' => 1, 'tones' => TONE_DRUM_TAB, 'scales' => SCALE_DRUM_TAB,  'name' => 'Drum'],
];

const DATA_BAR_TIME = 128;  // 出力データでは1小節分の音長=128
const MAX_REPEAT_TIMES = 8; // 最大リピート回数
const MAX_REPEAT_LEVEL = 4; // 最大リピート レベル

const OPT_HELP             = 'help';
const OPT_VERBOSE          = 'verbose';
const OPT_CHANNELS         = 'channels';
const OPT_BARS             = 'bars';
const OPT_EXCEPT_LENGTHS   = 'exceptLengths';
const OPT_DEBUG_SCORE_DUMP = 'debugScoreDump';

// MARK: 引数解析 --------------------------------
$error = new \nwk\utils\Error();
$debugdump = false;
$b_verbose = false;

// 引数チェック
$args = [];
$errs = '';
$options = \nwk\utils\Utils::getOpt($argv, array('h', 'v'),
    array(OPT_HELP, OPT_VERBOSE, OPT_CHANNELS . ':', OPT_BARS . ':', OPT_EXCEPT_LENGTHS . ':', OPT_DEBUG_SCORE_DUMP),
    $args, $errs);
if ($errs !== '') {
    $error->error("Invalid options:($errs)");
    exit(1);
}

if (count($args) !== 2 || isset($options[OPT_HELP]) || isset($options['h'])) {
    $error->error('Usage: php ' . $argv[0] . " in.mid out.txt [options...]\n" .
        "  Options are:\n" .
        "  -h, --" . OPT_HELP . "\n" .
        "  -v, --" . OPT_VERBOSE . "\n" .
        "  --" . OPT_CHANNELS . "=n       変換するチャンネル. ch番号(1～16)を範囲指定※(default:all)\n" . // 例:「4」「1,2」\n"
        "  --" . OPT_BARS .     "=n           変換する小節. 小節番号(2～256)を範囲指定※(default:all)\n" .
        "  --" . OPT_EXCEPT_LENGTHS . "=n  特別な命令に利用してる音長(4の倍数. 4～256)を範囲指定※\n" .
        "    ※範囲指定は [0-9,-#]の他, 文字列「all」,「none」 が使えます.\n" .
        "    例: 「-1,4-6,8#2,11-」ならば「0,1,4,5,6,8,9,11,12,...」を意味します\n" .
        "  --" . OPT_DEBUG_SCORE_DUMP .  "   楽譜データを標準出力に表示\n"
        );
    exit(1);
}

if (isset($options[OPT_VERBOSE]) || isset($options['v'])) {
    $b_verbose = true;
}

$opt = $options[OPT_CHANNELS] ?? 'all';
$channels = \nwk\utils\Utils::getRangeList($opt, 1, 16);
if ($channels === false) {
    $error->error("Invalid option: --" . OPT_CHANNELS);
    exit(1);
}
if (count($channels) === 0) {
    $error->error("チャンネルは1つ以上指定してください");
    exit(1);
}
if ($b_verbose) {
    echo('  channels:' . \nwk\utils\Utils::getRangeListString($channels, $opt === 'all') . "\n");
}

$opt = $options[OPT_EXCEPT_LENGTHS] ?? 'none';
$except_lens = \nwk\utils\Utils::getRangeList($opt, 4, 256);
if ($except_lens === false) {
    $error->error("Invalid option: --" . OPT_EXCEPT_LENGTHS);
    exit(1);
}
foreach($except_lens as $i => $len) {
    if (($len & 3) !== 0) {
        unset($except_lens[$i]);
    }
}
if ($b_verbose) {
    echo('  except_lengths:' . \nwk\utils\Utils::getRangeListString($except_lens, $opt === 'all') . "\n");
}

$bar_start = \nwk\sound\MidiReader::BAR_OFFSET;
$bar_end   = 256;
if (isset($options[OPT_BARS])) {
    $bars = \nwk\utils\Utils::getRangeList($options[OPT_BARS], \nwk\sound\MidiReader::BAR_OFFSET, 1000);
    //print_r($bars);
    if ($bars === false) {
        $error->error("Invalid option: --" . OPT_BARS);
        exit(1);
    }
    if (count($bars) === 0) {
        $error->error("Invalid option: --" . OPT_BARS . " 小節を全て省略することはできません");
        exit(1);
    }
    if (!\nwk\utils\Utils::isRangeListContinuous($bars)) {
        $error->error("Invalid option: --" . OPT_BARS . " 小節の範囲は連続しなければなりません");
        exit(1);
    }
    $bar_start = $bars[0];
    $bar_end   = end($bars);
}
if ($b_verbose) {
    echo("  bars: $bar_start-$bar_end\n");
}
$bar_start -= \nwk\sound\MidiReader::BAR_OFFSET;
$bar_end   -= \nwk\sound\MidiReader::BAR_OFFSET;

if (isset($options[OPT_DEBUG_SCORE_DUMP])) {
    $debugdump = true;
}


// MARK: ファイル チェック --------------------------------
// ---- ファイル名チェック
$in_midi_file = $argv[1];
$out_c_file   = $argv[2];
$error->setFilename($in_midi_file);

// midi ファイル名の末端が '_barX' ならば, X 小節(bar)単位に分割します
$div_bars = PHP_INT_MAX;
$matches  = [];
if (preg_match('/_bar([0-9]+)$/', pathinfo($in_midi_file, PATHINFO_FILENAME), $matches) === 1) {
    $div_bars = (int)$matches[1];
    if ($div_bars < 1) {
        $error->error("Invalid value div_bars\n");
        exit(1);
    }
}

// ファイル存在チェック
if (file_exists($in_midi_file) === false) {
    $error->error("File not found\n");
    exit(1);
}


// MARK: MIDI ファイル読込 --------------------------------
$midi_data = file_get_contents($in_midi_file);
if ($midi_data === false) {
    $error->error("File read error\n");
    exit(1);
}

// unpackで作られる配列は 1 からなので, arrat_merge() で 0 からにします
$midi_reader = new \nwk\sound\MidiReader();
if ($midi_reader->read(array_merge(unpack('C*', $midi_data))) == false) {
    exit(1);
}
$error->setBarTime($midi_reader->getBarTime());

$score = $midi_reader->getScore();
if ($debugdump) {
    $midi_reader->debugScoreDump($score);
}

// MARK: データ加工 --------------------------------
$div_len = ($div_bars === PHP_INT_MAX) ? PHP_INT_MAX : $midi_reader->getBarLen() * $div_bars;
// unset($data[4]); デバッグ用にわざと一部データを消す
$score_arr = \nwk\sound\ScoreUtils::divideScore($midi_reader, $score, $div_len, $error);
if (!$error->getNrErrors()) {

    $out_str = "/**** This file is made by $argv[0]. DO NOT MODIFY! ****/\n";
    //$out_str .= $midi_reader->createScoreInfo();
    $total_play_time = $midi_reader->getTotalPlayTime();
    $bar_time        = $midi_reader->getBarTime();
    $nr_bars         = (int)ceil($total_play_time / $bar_time);
    $time_resolution = $midi_reader->getTimeResolution();
    $min_len       = $time_resolution / 8;
    $max_len       = $time_resolution * 8;

    foreach ($score_arr as $score_nr => $r_score) {
        //echo("score[$score_nr]\n");
        \nwk\sound\ScoreUtils::checkTracks( $r_score, TRACK_TAB, $channels, $bar_time, $total_play_time, $error);// トラック,音階調査
        if ($error->getNrErrors()) { break; }
        \nwk\sound\ScoreUtils::extractBars( $r_score, $bar_start, $bar_end, $nr_bars, $bar_time, $error);   // 小節の抽出
        if ($error->getNrErrors()) { break; }
//        \nwk\sound\ScoreUtils::changeDrumLen( $r_score, $time_resolution / 4, $error);   // ドラム長を16分音符に変換
//        if ($error->getNrErrors()) { break; }
        \nwk\sound\ScoreUtils::integrateDrumsDR($r_score, $min_len, $max_len, $error);  // ドラムと休符を統合
        if ($error->getNrErrors()) { break; }
        \nwk\sound\ScoreUtils::splitRest(   $r_score, $max_len,             $error);    // 休符は2小節単位に分割
        if ($error->getNrErrors()) { break; }
        \nwk\sound\ScoreUtils::processLead( $r_score, CH_LEAD, CH_LEAD_SUB, $error);    // スライド処理
        if ($error->getNrErrors()) { break; }
        \nwk\sound\ScoreUtils::processBase( $r_score, CH_BASE,              $error);    // ベース処理
        if ($error->getNrErrors()) { break; }
        \nwk\sound\ScoreUtils::processChord($r_score, CH_CHORD,             $error);    // コード処理
        if ($error->getNrErrors()) { break; }
        \nwk\sound\ScoreUtils::processDrum( $r_score, CH_DRUM,              $error);    // ドラム処理
        if ($error->getNrErrors()) { break; }
        \nwk\sound\ScoreUtils::checkTime($midi_reader, $r_score, DATA_BAR_TIME, 4, 4, 256, $except_lens, $error); // 時間と音長チェック
        if ($error->getNrErrors()) { break; }

        [$merged_notes, $track_works] = \nwk\sound\ScoreMergeUtils::merge($midi_reader, $r_score, DATA_BAR_TIME);// マージ
        if ($b_verbose) {
            echo("  notes: " . count($merged_notes) . "\n");
        }
        [$max_repeat_times, $max_repeat_level] = \nwk\sound\ScoreMergeUtils::compressRepeat($merged_notes, CH_LEAD, MAX_REPEAT_TIMES, MAX_REPEAT_LEVEL, $error); // 圧縮
        if ($error->getNrErrors()) { break; }
        if ($b_verbose) {
            echo("  compressed notes: " . count($merged_notes) . "\n");
        }
        $str   = \nwk\sound\ScoreMergeUtils::genHistgramString($merged_notes, TRACK_TAB); // ヒストグラム文字列作成
        $bytes = setNoteString_($merged_notes); // 文字列作成
        $str  .= \nwk\sound\ScoreMergeUtils::genInfoString($max_repeat_times, $max_repeat_level, $bytes); // その他情報文字列作成
        $str  .= \nwk\sound\ScoreMergeUtils::genFormattedSourceString($midi_reader, $merged_notes, DATA_BAR_TIME, TRACK_TAB, $score_nr); // 整形
        $str  .= "\n    SD6_L_END\n};\n";
        //echo("---- score[$score_nr]\n"); $midi_reader->debugScoreDump($r_score);
        $out_str .= $str;
    }
}

// MARK: 出力 --------------------------------
$error->report();
file_put_contents($out_c_file, $out_str);
exit (0);

// MARK: setNoteString_()
/** 音符からマクロ文字列を作成します
 * @param $r_merged_notes マージしたMergedNoteの配列
 * @return バイト数を返す
 */
function setNoteString_(array &$r_merged_notes): int
{
    $bytes = 1; // SD6_END

    foreach($r_merged_notes as &$r_merged_note) {
        $scale = str_replace('#', 'S', $r_merged_note->r_note->data_scale);
        $len = $r_merged_note->len;
        $r_track_work = $r_merged_note->r_track_work;

        // リピート開始
        foreach ($r_merged_note->repeats as $repeat) {
            $r_merged_note->strs[] = "SD6_L_REP_$repeat";
            $bytes += 1;
        }

        switch ($r_track_work->ch) {
            case CH_LEAD:
                if ($scale === \nwk\sound\Note::SCALE_R) {
                    $r_merged_note->strs[] = "SD6_L_REST($len)";
                    $bytes += 1;
                    break;
                }
                //echo($r_merged_note->r_note->data_scale);

                $scales = explode(':', $scale);
                switch ($r_merged_note->r_note->tone) {
                    case \nwk\sound\Note::TONE_PIANO1: // ■ scale が, "音階:音階" ならばスライド有. 音階のみならばスライド無
                        if (count($scales) === 1) { // スライド無し
                            if ($r_track_work->curr_slide !== '') {
                                $r_track_work->curr_slide = '';
                                $r_merged_note->strs[] = "SD6_L_SLIDE_OFF";
                                $bytes += 1;
                            }
                            $r_merged_note->strs[] = "SD6_L_FL($len, SD6_$scale)";
                            $bytes += 2;
                            break;
                        }
                        // スライド有
                        if ($r_track_work->curr_slide !== $scales[1]) {
                            $r_track_work->curr_slide = $scales[1];
                            //echo("slide: $scale\n");
                            switch ($scales[1]) {
                                case 'B1':  $r_merged_note->strs[] = "SD6_L_SLIDE_SPEED_R1";  break;  // 速
                                case 'AS1': $r_merged_note->strs[] = "SD6_L_SLIDE_SPEED_R2";  break;
                                case 'A1':  $r_merged_note->strs[] = "SD6_L_SLIDE_SPEED_R4";  break;  // 遅
                                case 'GS1': $error->errorScore($r_track_work->ch, 0, $r_merged_note->r_note, "スライドでは G#1 は使えません"); break;
                            }
                            $bytes += 1;
                        }
                        $r_merged_note->strs[] = "SD6_L_FL($len, SD6_$scales[0])";
                        $bytes += 2;
                        break;
                    case \nwk\sound\Note::TONE_PIANO2: // ◣  scale が, "音階:音階" ならばエンベロープ速度変更. 音階のみならば変更なし
                    case \nwk\sound\Note::TONE_PIANO3: // ◢
                        if (count($scales) === 2) { // エンベロープ速度変更あり
                            //echo("env: $scale\n");
                            if ($r_track_work->curr_env !== $scales[1]) {
                                $r_track_work->curr_env = $scales[1];
                                switch ($scales[1]) {
                                    case 'B1':  $r_merged_note->strs[] = "SD6_L_ENV_SPEED_R2";  break;   // 速
                                    case 'AS1': $r_merged_note->strs[] = "SD6_L_ENV_SPEED_R4";  break;
                                    case 'A1':  $r_merged_note->strs[] = "SD6_L_ENV_SPEED_R8";  break;
                                    case 'GS1': $r_merged_note->strs[] = "SD6_L_ENV_SPEED_R16"; break;   // 遅
                                }
                                $bytes += 1;
                            }
                        }
                        //特定の場所の音は変換しないバグ用デバッグ
                        //if (90240 <= $r_note->time && $r_note->time <= 92160) { echo("$r_note->time $main_scale tone:$r_note->tone\n"); }

                        if ($r_merged_note->r_note->tone === \nwk\sound\Note::TONE_PIANO2) {
                            $r_merged_note->strs[] = "SD6_L_DW($len, SD6_$scales[0])";
                        } else {
                            $r_merged_note->strs[] = "SD6_L_UP($len, SD6_$scales[0])";
                        }
                        $bytes += 2;
                        break;
                }
                break;
            case CH_BASE:
                if ($scale === \nwk\sound\Note::SCALE_R) {
                    $r_merged_note->strs[] = "SD6_B_REST($len)";
                    $bytes += 1;
                    break;
                }
                switch ($r_merged_note->r_note->tone) {
                    case \nwk\sound\Note::TONE_PIANO1: $r_merged_note->strs[] = "SD6_B_FL($len, SD6_$scale)"; break;
                    case \nwk\sound\Note::TONE_PIANO2: $r_merged_note->strs[] = "SD6_B_DW($len, SD6_$scale)"; break;
                    case \nwk\sound\Note::TONE_PIANO3: $r_merged_note->strs[] = "SD6_B_UP($len, SD6_$scale)"; break;
                }
                $bytes += 2;
                break;
            case CH_CHORD:
                $scales = explode(':', $scale);
                // 両方休符なら専用命令
                if ($scales[0] === \nwk\sound\Note::SCALE_R && $scales[1] === \nwk\sound\Note::SCALE_R) {
                    $r_merged_note->strs[] = "SD6_C2_REST($r_merged_note->len)";
                    $bytes += 1;
                    break;
                }
                // 片方が休符ならばもう一方と同じ音にします
                if ($scales[0] === \nwk\sound\Note::SCALE_R) {
                    $scales[0] = $scales[1];
                }
                if ($scales[1] === \nwk\sound\Note::SCALE_R) {
                    $scales[1] = $scales[0];
                }
                switch ($r_merged_note->r_note->tone) {
                    case \nwk\sound\Note::TONE_PIANO1: $r_merged_note->strs[] = "SD6_C2_FL($len, SD6_$scales[0], SD6_$scales[1])"; break;
                    case \nwk\sound\Note::TONE_PIANO2: $r_merged_note->strs[] = "SD6_C2_DW($len, SD6_$scales[0], SD6_$scales[1])"; break;
                    case \nwk\sound\Note::TONE_PIANO3: $r_merged_note->strs[] = "SD6_C2_UP($len, SD6_$scales[0], SD6_$scales[1])"; break;
                }
                $bytes += 3;
                break;
            case CH_DRUM:
                switch ($scale) {
                    default:    $r_merged_note->strs[] = "SD6_D_REST($len)"; break;
                    case 'B1':  $r_merged_note->strs[] = "SD6_D_1($len)"; break;
                    case 'C2':  $r_merged_note->strs[] = "SD6_D_2($len)"; break;
                    case 'CS2': $r_merged_note->strs[] = "SD6_D_3($len)"; break;
                }
                $bytes += 1;
                break;
        } // switch (ch)

        // リピート終了
        for($i = 0; $i < $r_merged_note->nr_endps; $i++) {
            $r_merged_note->strs[] = "SD6_L_ENDR";
            $bytes += 1;
        }

    }
    return $bytes;
}