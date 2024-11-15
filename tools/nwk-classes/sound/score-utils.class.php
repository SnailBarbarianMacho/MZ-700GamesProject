<?php
/** 音楽データ ユーティリティ
 *
 * @author Snail Barbarian Macho (NWK) 2024.06.29
 */

declare(strict_types = 1);
namespace nwk\sound;

require_once(__DIR__ . '/../utils/error.class.php');
require_once 'score.class.php';
require_once 'midi.class.php';

class ScoreUtils {
    // ---------------------------------------------------------------- 楽譜分割
    // MARK: divideScore()
    /** 必要に応じて楽譜を複数に分割します(小節単位など)
     * - 休符は分割されますが, 休符以外は分割できません(エラー)
     * @param split_len 分割したい長さ(単位は $midi_reader )
     * @return 成功したら加工済データ(Score の配列). 失敗したら null
     */
    public static function divideScore(
        \nwk\sound\MidiReader $midi_reader, \nwk\sound\Score &$r_score, int $split_len, \nwk\utils\Error $error): ?array
    {
        if ($split_len === PHP_INT_MAX) { return [ $r_score ]; }

        $play_time = $midi_reader->getTotalPlayTime();

        // -------- (1) 楽譜を $split_len 単位で分割できるように, 休符を分割します.
        foreach ($r_score->tracks as $track_nr => &$r_track) {
            foreach ($r_track->sources as $source_nr => &$r_source) {
                //echo("ch[$r_track->ch] source[$source_nr]\n");
                $note_nr = 0;
                for ($time = $split_len; $time < $play_time; $time += $split_len) {
                    $note_nr = divideRestNoteByTime_($r_source->notes, $note_nr, $time);
                    if ($note_nr === false) {
                        $error->errorScore($r_track->ch, $source_nr, null, "時間 $time で分割できませんでした");
                        break;
                    }
                }
            }
        }
        if ($error->getNrErrors()) { return null; }

        // -------- (2) 楽譜を分割し, score_arr に設定します(手抜き版で遅い)
        $score_arr = [];
        for ($time = 0; $time < $play_time; $time += $split_len) {
            $time_end = $time + $split_len;
            $score = new \nwk\sound\Score();
            foreach ($r_score->tracks as &$r_track) {
                $track = new Track($r_track->ch);
                foreach ($r_track->sources as &$r_source) {
                    $source = new Source();
                    foreach ($r_source->notes as &$r_note) {
                        if ($time <= $r_note->time && $r_note->time < $time_end) {
                            $source->notes[] = $r_note;
                        }
                    }
                    $track->sources[] = $source;
                }
                $score->tracks[] = $track;
            }
            $score_arr[] = $score;
        }

        return $score_arr;
    }


    // MARK: divideRestNoteByTime_()
    /** 音譜の配列を走査して, $time を跨ぐ休符があれば分割します
     * - note->time === time ならば何もしないでその音譜のインデックスを返します
     * - $time を跨ぐ休符以外はエラー
     * - $time を跨ぐ休符は分割して後ろ半分のインデックスを返します
     * @param $r_notes [in][out] 音譜の配列. 分割成功したら要素が1個増えます
     * @param $note_idx 走査を開始するノートの添え字
     * @return 成功したら音譜のインデックスを返します. 末端まで来たら count(r_notes), 分割できないなら false
     */
    private static function divideRestNoteByTime_(array &$r_notes, int $note_idx, int $time): int|false
    {
        for (; $note_idx < count($r_notes); $note_idx++) {
            $r_note = &$r_notes[$note_idx];

            if ($r_note->time == $time) {
                // $time は $r_note の先頭にあり, そのまま分割できる
                //echo("  $time $r_note->scale $r_note->time\n");
                return $note_idx;
            }
            if ($r_note->time <= $time && $time < $r_note->time + $r_note->len) {
                // $time は $r_note の内側にある. 休符なら分割. それでないならエラー
                //echo("  $time $r_note->scale $r_note->time#$r_note->len\n");
                if ($r_note->scale !== \nwk\sound\Note::SCALE_R) {
                    return false;
                }
                // 今の休符の前に音譜を挿入します
                array_splice($r_notes, $note_idx, 0, 1); //オブジェクトを直接挿入すると分解されてしまうのでとりあえず '1'
                $r_notes[$note_idx] = new \nwk\midi\Note(
                    \nwk\sound\Note::SCALE_R,
                    0,
                    $r_note->time,
                    $time - $r_note->time,
                    0);
                $r_note->len  = $r_note->time + $r_note->len - $time;
                $r_note->time = $time;
                return $note_idx + 1;
            }
            // $time は $r_note より未来にあるので, ノートの走査は続く
        }
        return $note_idx;   // 見つからなかった
    }


    // ---------------------------------------------------------------- 楽譜データが仕様を満たしてるかどうかを調査
    // MARK: checkTracks()
    /** トラック割当, 音源, 音色, 音階を調査します
     * - トラックが不足してたら追加します
     * - 知らないチャンネルのトラックならばエラー
     * - 音源数が多いならエラー
     *
     * @param $r_score [in][out]楽譜データ
     * @param $track_tab サポートするトラックの形式テーブル. チャンネル番号がキーになります. 例えば次のように書きます:
     * [
     *    0 => [ 'nr_sources' => 1, 'tones' => TONE_TAB, 'scales' => SCALE_TAB,      'name' => '名前0'],// midiデータの ch0 があるトラックから, 1音源採用
     *    1 => [ 'nr_sources' => 1, 'tones' => TONE_TAB, 'scales' => SCALE_TAB,      'name' => '名前1'],// midiデータの ch1 があるトラックから, 1音源採用
     *    4 => [ 'nr_sources' => 2, 'tones' => TONE_TAB, 'scales' => SCALE_TAB,      'name' => '名前4'],// midiデータの ch4 があるトラックから, 3音源採用
     *    9 => [ 'nr_sources' => 1, 'tones' => [0],      'scales' => SCALE_DRUM_TAB, 'name' => '名前9'],// midiデータの ch10 (ドラム)があるトラックから, 1音源採用
     * ];
     * TONE_TAB は例えば,
     * [
     *    0, 1, 2, // GMでのピアノ系3つ
     * ]
     * SCALE_TAB は例えば,
     * [
     *   'C2',  'C#2',  'D2',  'D#2',  'E2',  'F2',  'F#2',  'G2',  'G#2',  'A2',  'A#2',  'B2',
     *   'C3',  'C#3',  'D3',  'D#3',  'E3',  'F3',  'F#3',  'G3',  'G#3',  'A3',  'A#3',  'B3',
     *   'C4',  'C#4',  'D4',  'D#4',  'E4',  'F4',  'F#4',  'G4',  'G#4',  'A4',  'A#4',  'B4',
     *   'C5',  'C#5',  'D5',  'D#5',  'E5',  'F5',  'F#5',  'G5',  'G#5',  'A5',  'A#5',  'B5',
     *   'C6',  nwk\sound\Note::SCALE_R,
     * ]
     * SCALE_DRUM_TAB は例えば,
     * [
     *    'B1', 'C2',  'C#2', nwk\sound\Note::SCALE_R, // ドラム3つ
     * ]
     * @param $channels キー=チャンネル. キーが存在するなら変換対象とします. それ以外は変換しませんが,
     *    例外として配列の大きさ自体が 0 ならば全て変換します
     */
    public static function checkTracks(\nwk\sound\Score &$r_score, array $track_tab, array $channels, int $bar_time, int $total_play_time, \nwk\utils\Error $error): void
    {
        // ---------------- 指定したチャンネルのトラックが無いなら追加
        foreach ($track_tab as $ch => $track_tab_elem) {
            if (ScoreUtils::searchChannelInScore_($ch, $r_score) === null) {
                $error->warnScore($ch, -1, null, "このチャンネルのトラックが存在しません. 追加します");
                $r_score->tracks[] = new \nwk\sound\Track($ch);
            }
        }
        usort($r_score->tracks, function($a, $b) { return $a->ch > $b->ch; });

        // ---------------- 内容のチェック
        foreach ($r_score->tracks as $track_nr => &$r_track) {
            $ch = $r_track->ch;

            // -------- サポートしないチャンネル番号があればエラー
            if (!isset($track_tab[$ch])) {
                $error->warnScore($ch, -1, null, "このチャンネルのトラックは対応しません. 無視します");
                unset($r_score->tracks[$track_nr]);    // このトラックは削除
                continue;
            }

            // -------- サポートする音源数を越えたらエラー
            $nr_sources = count($r_track->sources);
            $max_sources = $track_tab[$ch]['nr_sources'];
            if ($max_sources < $nr_sources) {
                // 音源が多い場合は, 各音源の最初の音(休符以外)の場所を提示します
                $str = '';
                foreach ($r_track->sources as $source_nr => &$r_source) {
                    foreach ($r_source->notes as &$r_note) {
                        if ($r_note->scale !== \nwk\sound\Note::SCALE_R) {
                            $str .= "\n    音源[$source_nr]の最初の音符:" . $r_note->genInfo($bar_time);
                            break;
                        }
                    }
                }
                $error->errorScore($r_track->ch, -1, null, "このトラックの音源は多すぎます($max_sources < $nr_sources)$str");
            }

            // -------- 音階・音色のチェック
            foreach ($r_track->sources as $source_nr => &$r_source) {
                $r_tone_tab  = &$track_tab[$ch]['tones'];
                $r_scale_tab = &$track_tab[$ch]['scales'];
                //if ($track_nr === 1) { print_r($r_scale_tab); }
                foreach ($r_source->notes as $note_nr => &$r_note) {
                    //if ($track_nr == 1) { echo($r_note->scale . "\n"); }
                    $tone  = $r_note->tone;
                    $scale = $r_note->scale;
                    if (array_search($tone, $r_tone_tab) === false) {
                        $error->errorScore($r_track->ch, $source_nr, $r_note, "この音色[$tone]はサポートしません");
                    };
                    if (array_search($scale, $r_scale_tab) === false) {
                        $error->errorScore($r_track->ch, $source_nr, $r_note, 'この音階はサポートしません');
                    };
                }
            }

            // -------- 音源数が少ないなら追加して全部休符
            for ($i = $nr_sources; $i < $max_sources; $i++) {
                $error->warnScore($r_track->ch, $i, null, "この音源を追加します");
                $r_track->sources[] = new \nwk\sound\Source();
                end($r_track->sources)->notes[] = new \nwk\sound\Note(
                    \nwk\sound\Note::SCALE_R,
                    0,
                    0,
                    $total_play_time,
                    0);
            }

            //print_r($r_track->sources);
            //echo("track idx[$track_nr], ch[$r_track->ch] sources[" . count($r_track->sources) . "]\n");
        }

        // ---------------- 削除したトラックは整理
        $r_score->tracks = array_values($r_score->tracks);

        // ---------------- b_channels[] == false なトラックは全部休符にします
        if (count($channels) !== 0) {
            foreach ($r_score->tracks as $track_nr => &$r_track) {
                if (!isset($channels[$r_track->ch])) {
                    foreach ($r_track->sources as &$r_source) {
                        $r_source = new \nwk\sound\Source();
                        $r_source->notes[] = new \nwk\sound\Note(
                            \nwk\sound\Note::SCALE_R,
                            0,
                            0,
                            $total_play_time,
                            0);
                    }
                }
            }
        }

        //foreach ($r_score->tracks as $track_nr => &$r_track) { echo("track idx[$track_nr], ch[$r_track->ch] sources[" . count($r_track->sources) . "]\n"); }
    }


    // MARK: extractBars()
    /** 小節を抽出します
     * @param $bar_start 小節番号 0～
     * @param $bar_end   小節番号 0～
     */
    public static function extractBars(\nwk\sound\Score $r_score, int $bar_start, int $bar_end, int $nr_bars, int $bar_time, \nwk\utils\Error $error)
    {
        $time_start = $bar_start     * $bar_time;
        $time_end   = ($bar_end + 1) * $bar_time;
        //echo("bar: $bar_start - $bar_end # = $nr_bars\n");
        //echo("time: $time_start - $time_end\n");

        foreach ($r_score->tracks as &$r_track) {
            foreach ($r_track->sources as $source_nr => &$r_source) {
                foreach ($r_source->notes as $note_nr => &$r_note) {
                    $note_time_start = $r_note->time;
                    $note_time_end   = $note_time_start + $r_note->len;

                    // $time_start より昔の音符は削除
                    if ($note_time_end <= $time_start) {
                        unset($r_source->notes[$note_nr]);
                        continue;
                    }
                    // $time_end より先の音符は削除
                    if ($time_end <= $r_note->time) {
                        unset($r_source->notes[$note_nr]);
                        continue;
                    }
                    // $time_start を跨ぐ音符は昔を切り詰める
                    if ($note_time_start < $time_start && $time_start < $note_time_end) {
                        $r_note->len -= $time_start - $r_note->time;
                        $r_note->time = $time_start;
                        $note_time_start = $r_note->time;
                        $note_time_end   = $note_time_start + $r_note->len;
                    }
                    // $time_end を跨ぐ音符は先を切り詰める
                    if ($note_time_start < $time_end && $time_end < $note_time_end) {
                        $r_note->len = $time_end - $note_time_start;
                    }
                    // あとはそのまま
                }

                $r_source->notes = array_values($r_source->notes);
                //echo("-----ch[$r_track->ch] source[$source_nr] time[$time_start-$time_end]\n");
                //foreach ($r_source->notes as &$r_note) { echo("$r_note->time-" . ($r_note->time + $r_note->len) . " $r_note->scale\n"); };
            }
        }
    }



    // ---------------------------------------------------------------- ドラム パートの加工
    // MARK: changeDrumLen()
    /** midi データでは, ドラムの長さは短い時間に固定されてます. この長さを変更します. 次の休符から時間を拝借します
     * - 2024.07.24 使用されてません. メンテされてません. 多分廃止
     * @param $r_score      [in][out]楽譜データ
     * @param $drum_len     変更後のドラムの長さ
     */
    public static function changeDrumLen(\nwk\sound\Score &$r_score, int $drum_len, \nwk\utils\Error $error): void
    {
        foreach ($r_score->tracks as $track_nr => &$r_track) {
            if ($r_track->ch !== \nwk\sound\MidiReader::CH_DRUM) { continue; }
            foreach ($r_track->sources as $source_nr => &$r_source) {
                //echo("changeDrum()before\n");foreach ($r_source->notes as $note_nr => &$r_note) { echo($note_nr . ' ' . $r_note->scale . ' ' . $r_note->time . '#' . $r_note->len . "\n"); }// 整理前の状態を表示

                // (1) ドラム長さの変更
                $t = -1; // 次の休符の時間を変更(-1ならば変更なし)
                $l =  0; // 次の休符から取る音長
                foreach ($r_source->notes as $note_nr => &$r_note) {
                    $scale = $r_note->scale;
                    if ($scale === \nwk\sound\Note::SCALE_R) {  // 休符
                        if ($t !== -1) {
                            $r_note->len  -= $l;
                            $r_note->time  = $t;
                            $t = -1;
                        }
                    } else {                                    // ドラム
                        if ($t !== -1) {
                            $error->errorScore($r_track->ch, $source_nr, $r_note, "ドラム音の整理中に, 連続してるドラム音を発生しました");
                        }
                        $l = $drum_len - $r_note->len;          // 借りる時間
                        $t = $r_note->time + $drum_len;
                        $r_note->len = $drum_len;
                    }
                    //echo($r_note->scale . ' ' . ($r_note->time / 480). "\n");
                }
                if ($t !== -1) {
                    $error->errorScore($r_track->ch, $source_nr, $r_note, "最後がドラムで終了しています");
                }
                //echo("changeDrum()mid\n");foreach ($r_source->notes as $note_nr => &$r_note) { echo($note_nr . ' ' . $r_note->scale . ' ' . $r_note->time . '#' . $r_note->len . "\n"); }// 整理中の状態を表示

                // (2) 長さが 0 になった音符は削除(休符だけのはず!)
                foreach ($r_source->notes as $note_nr => &$r_note) {
                    if ($r_note->len === 0) {
                        if ($r_note->scale !== \nwk\sound\Note::SCALE_R) {
                            $error->errorScore($r_track->ch, $source_nr, $r_note, "ドラム音を整理中に異常が発生しました");
                        }
                        unset($r_source->notes[$note_nr]);
                    }
                }
                $r_source->notes = array_values($r_source->notes);
                //echo("changeDrum()after\n");foreach ($r_source->notes as $note_nr => &$r_note) {echo($note_nr . ' ' . $r_note->scale . ' ' . $r_note->time . '#' . $r_note->len . "\n"); }// 整理後の状態を表示
            }
        }
    }


    // MARK: integrateDrumsRD()
    /** ドラム パートの音符の並びが「休符->ドラム」ならば, 1つにまとめます
     * - 2024.07.24 使用されてません. メンテされてません
     * @param $r_score [in][out]楽譜データ
     */
    public static function integrateDrumsRD(\nwk\sound\Score &$r_score, \nwk\utils\Error $error): void
    {
        foreach ($r_score->tracks as $track_nr => &$r_track) {
            if ($r_track->ch !== \nwk\sound\MidiReader::CH_DRUM) { continue; }

            foreach ($r_track->sources as $source_nr => &$r_source) {

                $r_rest = null;
                foreach ($r_source->notes as $note_nr => &$r_note) {
                    $scale = $r_note->scale;

                    if ($scale === \nwk\sound\Note::SCALE_R) {  // 休符
                        $r_rest = $r_note;
                    } else {                                    // ドラム
                        if ($r_rest !== null) {                 // 1つ前に休符がある
                            // 休符に情報を移してドラムを消す
                            $r_rest->scale = $r_note->scale;
                            $r_rest->len  += $r_note->len;
                            unset($r_source->notes[$note_nr]);
                            $r_rest = null;
                        }
                    }
                }

                $r_source->notes = array_values($r_source->notes);
                //print_r($r_source->notes);
                //foreach ($r_source->notes as $note_nr => &$r_note) {echo("$note_nr $r_note->scale $r_note->time#$r_note->len\n"); }
            }
        }
    }


    // MARK: integrateDrumsDR()
    /** ドラム パートの音符の並びが「ドラム->休符」ならば, 1つにまとめます
     * 一番最後のドラムは, 最小の長さを追加します
     * @param $r_score [in][out]楽譜データ
     * @param $min_len ドラム音符の最短
     * @param $max_len ドラム音符の最長
     */
    public static function integrateDrumsDR(\nwk\sound\Score &$r_score, int $min_len, int $max_len, \nwk\utils\Error $error): void
    {
        foreach ($r_score->tracks as $track_nr => &$r_track) {
            if ($r_track->ch !== \nwk\sound\MidiReader::CH_DRUM) { continue; }

            foreach ($r_track->sources as $source_nr => &$r_source) {

                $r_drum = null;
                foreach ($r_source->notes as $note_nr => &$r_note) {
                    $scale = $r_note->scale;

                    if ($scale !== \nwk\sound\Note::SCALE_R) {  // ドラム
                        $r_drum = $r_note;
                    } else {                                    // 休符
                        if ($r_drum !== null) {                 // 1つ前にドラムがある
                            // ドラムに情報を移して休符を消す. ドラム+休符の長さが $max_len を越えたら休符は残す
                            if ($r_drum->len + $r_note->len <= $max_len) {
                                $r_drum->len  += $r_note->len;
                                unset($r_source->notes[$note_nr]);
                            } else {
                                $r_note->time = $r_drum->time + $max_len;
                                $r_note->len -= $max_len - $r_drum->len;
                                $r_drum->len  = $max_len;
                            }
                            $r_drum = null;
                        }
                    }
                }

                $r_source->notes = array_values($r_source->notes);
                //print_r($r_source->notes);
                //foreach ($r_source->notes as $note_nr => &$r_note) {echo("$note_nr $r_note->scale $r_note->time#$r_note->len\n"); }

                // 一番最後のドラムの長さを修正する
                //$r_last_note = &$r_source->notes[count($r_source->notes) - 1];
                //if ($r_last_note->scale !== \nwk\sound\Note::SCALE_R) {
                //    $r_last_note->len   = $min_len;
                //}
            }
        }
    }


    // ---------------------------------------------------------------- 長い休符を分割
    // MARK: splitRest()
    /** 2小節を超える休符やドラムは分割します
     * - 2小節を超えるその他の音はエラーです. 手動で分割してください
     * @param $r_score [in][out]楽譜データ
     * @param $split_len 分割したい長さ
     */
    public static function splitRest(\nwk\sound\Score &$r_score, int $split_len, \nwk\utils\Error $error): void
    {
        foreach ($r_score->tracks as $track_nr => &$r_track) {
            $ch = $r_track->ch;
            foreach ($r_track->sources as $source_nr => &$r_source) {
                for ($note_nr = 0; $note_nr < count($r_source->notes); $note_nr++) {
                    $r_note = &$r_source->notes[$note_nr];
                    //echo("trk:$track_nr, ch:$ch, src:$source_nr, note:$note_nr\n");
                    //print_r($note);
                    if ($split_len < $r_note->len) {
                        if ($ch !== \nwk\sound\MidiReader::CH_DRUM && $r_note->scale !== \nwk\sound\Note::SCALE_R) {
                            $error->errorScore($r_track->ch, $source_nr, $r_note, "音が長すぎます($r_note->len)");
                            continue;
                        }
                        $len  = $r_note->len  - $split_len;
                        $time = $r_note->time + $split_len;
                        $r_source->notes[$note_nr]->len = $split_len;
                        array_splice($r_source->notes, $note_nr + 1, 0, 1); //オブジェクトを直接挿入すると分解されてしまう
                        $r_source->notes[$note_nr + 1] = new \nwk\sound\Note(
                            \nwk\sound\Note::SCALE_R,
                            0,
                            $time,
                            $len,
                            0);
                    }
                }
            }
        }
    }


    // ---------------------------------------------------------------- 各パートの処理
    // MARK: processLead()
    /** リードの処理 スライド速度,エンベロープ速度
     * @param $r_score [in][out]楽譜データ
     * @param $main_ch  チャンネル
     * @param $sub_ch   サブ チャンネル
     * - 音譜の data_scale が, 「$chの音階:$ch+1の音階」になります. 例:「C3:E3」
     * - サブ チャンネル は削除されます
     */
    static public function processLead(\nwk\sound\Score &$r_score, int $main_ch, int $sub_ch, \nwk\utils\Error $error): void
    {
        // トラックがあるか探す
        $main_track_nr = ScoreUtils::searchChannelInScore_($main_ch, $r_score);  // メインのトラック
        $sub_track_nr  = ScoreUtils::searchChannelInScore_($sub_ch,  $r_score);  // サブのトラック
        if ($main_track_nr === false) {
            $error->errorScore($main_ch, -1, null, "メイン チャンネルが見つかりません");
            return;
        }
        if ($sub_track_nr === false) {
            $error->errorScore($sub_ch, -1, null, "サブ チャンネルが見つかりません");
            return;
        }

        //echo("before $sub_track_nr->$main_track_nr\n"); foreach ($r_score->tracks as $track_nr => &$r_track) { echo("track#$track_nr ch[$r_track->ch]: sources:[" . count($r_track->sources) . "]\n"); }

        foreach ($r_score->tracks[$main_track_nr]->sources[0]->notes as &$r_main_note) {
            $r_main_note->data_scale = $r_main_note->scale;
        }

        // 音符のチェックとスライド処理
        $source_nr = 0;
        foreach ($r_score->tracks[$sub_track_nr]->sources[0]->notes as $sub_note_nr => &$r_sub_note) {
            // サブ チャンネルの音符情報を取得
            if ($r_sub_note->scale !== \nwk\sound\Note::SCALE_R) {
                $sub_time = $r_sub_note->time; // スライド元音符の時間
                $sub_len  = $r_sub_note->len;  // スライド元音符の長さ
                // サブ チャンネルの音符の時間・長さが一致する音符を, メイン チャンネルから探します. なければエラー
                // 見つかればメイン音階を, "サブ音階:メイン音階" に書き換えます
                foreach ($r_score->tracks[$main_track_nr]->sources[$source_nr]->notes as $main_note_nr => &$r_main_note) {
                    if ($r_main_note->time === $sub_time && $r_main_note->len === $sub_len) {
                        $r_main_note->data_scale = $r_main_note->scale . ':' . $r_sub_note->scale;
                        //echo("$r_main_note->tone $r_main_note->data_scale\n");
                        $sub_time = -1;
                        break;
                    }
                }
                if ($sub_time !== -1) {
                    $error->errorScore($sub_ch, $source_nr, $r_sub_note, "このサブ チャンネルの音符と時間・長さが一致するメイン チャンネル音符がありません");
                }
            }
        }

        unset($r_score->tracks[$sub_track_nr]);
        $r_score->tracks = array_values($r_score->tracks);
        //foreach ($r_score->tracks[$main_track_nr]->sources[0]->notes as &$r_note) { echo($r_note->data_scale); }
        //echo("after main/sub=[$main_track_nr/$sub_track_nr]\n"); foreach ($r_score->tracks as $track_nr => &$r_track) { echo("track#$track_nr ch[$r_track->ch]: sources:[" . count($r_track->sources) . "]\n"); }
    }


    // MARK: processBase()
    /** ベース処理
     * @param $r_score [in][out]楽譜データ
     * @param $ch このチャンネルが処理の対象になります
     * - 大したことしてません
     */
    static public function processBase(\nwk\sound\Score &$r_score, int $ch, \nwk\utils\Error $error): void
    {
        // トラックがあるか探す
        $track_nr = ScoreUtils::searchChannelInScore_($ch, $r_score);
        if ($track_nr === false) {
            $error->errorScore($ch, -1, null, "チャンネルが見つかりません");
            return;
        }

        $r_track = &$r_score->tracks[$track_nr];

        foreach($r_track->sources as &$r_source) {
            foreach($r_source->notes as &$r_note) {
                $r_note->data_scale = $r_note->scale;
            }
        }
    }


    // MARK: processChord()
    /** コード処理
     * @param $r_score [in][out]楽譜データ
     * @param $ch このチャンネルが処理の対象になります
     * - 複数の音源は1つに纏められ,
     *   音譜の data_scale プロパティに書き込まれます.
     *   例1: 各音源の音符「C3」,「E3」,「G3」ならば, 「C3:E3:G3」に纏まる
     *   例2: 各音源の音符「C3」,「無音」,「E3」ならば, 「C3:R:E3」に纏まる
     * - 各音源の音符は, 位置と長さが一致しなければなりません
     */
    static public function processChord(\nwk\sound\Score &$r_score, int $ch, \nwk\utils\Error $error): void
    {
        // トラックがあるか探す
        $track_nr = ScoreUtils::searchChannelInScore_($ch, $r_score);
        if ($track_nr === false) {
            $error->errorScore($ch, -1, null, "チャンネルが見つかりません");
            return;
        }

        $r_track = &$r_score->tracks[$track_nr];
        $nr_sources = count($r_track->sources);
        if ($nr_sources <= 1) {
            return;     // 纏める必要なし
        }

        $b_ok = true;
        //foreach($r_track->sources as $source_nr => &$r_source) { echo("source$source_nr\n"); foreach ($r_source->notes as &$r_note) { echo($r_note->scale); } echo("\n"); }

        // 音源1以降の音符の位置と長さに一致する音符(休符除く)が, 音源0にあるかを調査. 無ければエラー
        for ($i = 1; $i < $nr_sources; $i++) {
            foreach ($r_track->sources[$i]->notes as $note_nr => &$r_note) {
                if ($r_note->scale === 'R') {   // 休符は削除します
                    unset($r_track->sources[$i]->notes[$note_nr]);
                    continue;
                }
                $time  = $r_note->time;
                $len   = $r_note->len;

                $b_found = false;
                // 手抜きなので総当たりで検索 (実際は前回ヒットしたところからの検索でいい)
                foreach ($r_track->sources[0]->notes as &$r_note0) {
                    if ($r_note0->time === $time && $r_note0->len === $len) {
                        $b_found = true;
                        break;
                    }
                }
                if ($b_found === false) {
                    $error->errorScore($r_track->ch, 0, $r_note, "この音譜と時間・音長が一致する音符が, 音源0にありません");
                    $b_ok = false;
                }
            }
            $r_track->sources[$i]->notes = array_values($r_track->sources[$i]->notes);
        }
        if ($b_ok === false) { return; }
        //foreach($r_track->sources as $source_nr => &$r_source) { echo("source$source_nr\n"); foreach ($r_source->notes as &$r_note) { echo($r_note->scale); } echo("\n"); }

        // 音源0の音譜の位置と長さに一致する音符が, 音源1以降にあるかを調査.
        // 有るなら音源0の音符の data_scale プロパティに該当音符の音程を合成
        // 無いなら音源0の音符の data_scale プロパティに休符を合成
        foreach($r_track->sources[0]->notes as $note_nr => &$r_note0) {
            $r_note0->data_scale = $r_note0->scale;
            $len0  = $r_note0->len;
            $time0 = $r_note0->time;
            for ($i = 1; $i < $nr_sources; $i++) {
                $b_found = false;
                // 手抜きなので総当たりで検索 (実際は前回ヒットしたところからの検索でいい)
                foreach ($r_track->sources[$i]->notes as &$r_note) {
                    if ($r_note->time === $time0 && $r_note->len === $len0) {
                        $r_note0->data_scale .= ':'. $r_note->scale;
                        $b_found = true;
                        break;
                    }
                }
                // 音源1～の音符が無ければ, 休符とみなす
                if ($b_found === false) {
                    //$error->warnScore($r_track->ch, 0, $r_note0, "この音譜と同じ時間・長さの音符が, 音源$i に無いので, 休符とみなします");
                    $r_note0->data_scale .= ':R';
                }
            }
        }
        //foreach ($r_track->sources[0]->notes as &$r_note) { echo($r_note->data_scale.' '); };

        // 不要になった音源1以降を削除
        for ($i = 1; $i < $nr_sources; $i++) {
            unset($r_track->sources[$i]);
        }
        $r_track->sources = array_values($r_track->sources);
    }


    // MARK: processDrum()
    /** ドラム処理
     * @param $r_score [in][out]楽譜データ
     * @param $ch このチャンネルが処理の対象になります
     * - 大したことしてません
     */
    static public function processDrum(\nwk\sound\Score &$r_score, int $ch, \nwk\utils\Error $error): void
    {
        // トラックがあるか探す
        $track_nr = ScoreUtils::searchChannelInScore_($ch, $r_score);
        if ($track_nr === false) {
            $error->errorScore($ch, -1, null, "チャンネルが見つかりません");
            return;
        }

        $r_track = &$r_score->tracks[$track_nr];

        foreach($r_track->sources as &$r_source) {
            foreach($r_source->notes as &$r_note) {
                $r_note->data_scale = $r_note->scale;
            }
        }
    }


    // ---------------------------------------------------------------- 時間単位変換
    // MARK: checkTime()
    /**
     * 時間と音長の値をチェックします
     * @param $bar_time    1小節がこの音長になるように変換してチェックします
     * @param $time_unit   変換後の時間,音長の単位(例:4ならば4の倍数であること)
     * @param $len_min     変換後の音長の最小値
     * @param $len_max     変換後の音長の最大値
     * @param $except_lens 変換後の音長はこの配列のどの値であっもいけません
     */
    static public function checkTime(
        \nwk\sound\MidiReader $midi_reader, \nwk\sound\Score &$r_score,
        int $bar_time, int $time_unit, int $len_min, int $len_max, array $except_lens,
        \nwk\utils\Error $error): void
    {
        $midi_bar_time = $midi_reader->getBarTime();

        foreach ($r_score->tracks as &$r_track) {
            foreach ($r_track->sources as $source_nr => &$r_source) {
                foreach ($r_source->notes as &$r_note) {

                    $t = $r_note->time * $bar_time / $midi_bar_time;
                    $time = (int)$t;
                    if ($time !== $t || $time !== (int)($time / $time_unit) * $time_unit) {
                        $error->errorScore($r_track->ch, $source_nr, $r_note,
                            "この音符の位置[$r_note->time]が中途半端($time)に変換されました. 変換後の値は $time_unit の倍数でなければなりません");
                        continue;
                    }

                    $t = $r_note->len * $bar_time / $midi_bar_time;
                    $len = (int)$t;
                    if ($len !== $t || $len !== (int)($len / $time_unit) * $time_unit) {
                        $error->errorScore($r_track->ch, $source_nr, $r_note,
                            "この音符の音長[$r_note->len]が中途半端($len)に変換されました. 変換後の値は $time_unit の倍数でなければなりません");
                        continue;
                    }
                    if ($len < $len_min || $len_max < $len) {
                        $error->errorScore($r_track->ch, $source_nr, $r_note,
                            "この音符の音長[$r_note->len]が範囲[$len_min, $len_max]を超えてます");
                        continue;
                    }

                    if (array_search($len, $except_lens) !== false) {
                        $error->errorScore($r_track->ch, $source_nr, $r_note,
                            "この音符の音長[$len]は使用できません. サウンド エンジンが特殊用途で使用します");
                        continue;
                    }
                }
            }
        }
    }


    // ---------------------------------------------------------------- 検索サブ関数
    // MARK: searchChannelSource_()
    /** このチャンネルがテーブルにあるか探します
     * @param $track_tab checkTracks() 参照
     * @return int/null = 配列のインデックス/失敗
     */
    private static function searchChannelSource_(int $ch, array $track_tab): ?int
    {
        foreach ($track_tab as $idx => &$elem) {
            if ($elem['ch'] == $ch) {
                return $idx;
            }
        }
        return null;
    }


    // MARK: searchChannelInScore_()
    /** このチャンネルが楽譜にあるか探します
     * @return あれば track_nr, なければ null
     */
    private static function searchChannelInScore_(int $ch, \nwk\sound\Score &$r_score): ?int
    {
        foreach ($r_score->tracks as $track_nr => &$r_track) {
            if ($r_track->ch == $ch) {
                return $track_nr;
            }
        }
        return null;
    }


}