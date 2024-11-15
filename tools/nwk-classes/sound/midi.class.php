<?php
/** Midi 関係クラス
 *
 * @author Snail Barbarian Macho (NWK) 2024.06.29
 */

declare(strict_types = 1);
namespace nwk\sound;

require_once 'score.class.php';

/** 現在のトラックの状態保持クラス */
class CurrentTrackState_ {
    public int  $time;              // 時間
    public int  $tone;              // 音色
    public array $source_states;    // 音源
    function __construct()
    {
        $this->time = 0;
        $this->tone = 0;
        $this->source_states = [];
    }
};

/** 現在の音源の状態保持クラス. Note の同名プロパティと同じです */
class CurrentSourceState_ {
    public string $scale;           // 音階
    public int    $tone;            // 音色
    public int    $time;            // 開始時間
    public int    $velo;            // 音量
    function __construct(string $scale, int $tone, int $time, int $velo)
    {
        $this->scale = $scale;
        $this->tone  = $tone;
        $this->time  = $time;
        $this->velo  = $velo;
    }
};


// -------------------------------- Midi リーダー
// MARK: MidiReader
/** バイナリ配列を MIDI データとみなし, MML に変換しやすい形式にするクラス
 * - 各種制限:
 *   - 1チャンネルは 1トラックしか割り当てできません
 *   - 長さはデルタ タイムの値がベースです. 時間単位に左右されません
 *   - その他イベント(ドラムの切り替え, ピッチベンド等...)は全て無視します
 * - 使い方:
 *   read() で読込&変換, getMML() でゲット.
 */
class MidiReader {
    public  const BAR_OFFSET    = 2;    // 小節番号は 2 から

    public  const CH_DRUM       = 9;    // ドラム チャンネル(midiツール上では1から数えるので ch10)

    private const DEBUG_NOTE_   = 0;    // 1 にするとノートとプログラム チェンジ イベントの詳細を出力します
    private const DEBUG_        = 0;    // 1 にすると上記以外のイベントの詳細を出力します

    private array $data_        = [];   // 入力バイナリデータ
    private int   $index_       = 0;    // データポインタ

    private int   $tempo_       = 0;    // 四分音符の時間(usec)
    private int   $time_resolution_ = 0;// 四分音符の分解能 例:480
    private int   $numerator_   = 0;    // 拍子の分子 例:4
    private int   $fraction_    = 0;    // 拍子の分母 例:4
    private int   $bar_time_    = 0;    // 1 小節分の時間(四分音符 = tempo_ 換算)例: 1920(=480 * 4)
    private int   $play_time    = 0;    // 演奏時間. 小節単位にアライメントされます

    private array $curr_track_states_;  // 現在のトラックの状態. CurrTrackState の配列
    private \nwk\sound\Score $score_;    // 楽譜

    // ---------------------------------------------------------------- 読み込み, プロパティ, データ
    // MARK: read()
    /** MIDI データを読込, 変換します
     * @param $data バイナリデータ
     * @return true/false = 成功/失敗
     */
    public function read(array $data): bool
    {
        $this->data_        = $data;
        $this->index_       = 0;

        $this->tempo_       = 0;
        $this->time_resolution_ = 0;
        $this->numerator_   = 0;
        $this->fraction_    = 0;
        $this->bar_time_    = 0;
        $this->total_play_time_ = 0;

        $this->curr_track_states_ = [];
        $this->score_       = new \nwk\sound\Score();

        // ヘッダ チャンク読込
        $nr_tracks = $this->readHeaderChunk_();
        if ($nr_tracks === false) {
            return false;
        }

        // トラック チャンク読込
        for ($track_nr = 0; $track_nr < $nr_tracks; $track_nr++) {
            $this->debugNoteEcho_("Track$track_nr\n");

            $this->curr_track_states_[] = new CurrentTrackState_();
            $this->score_->tracks[]     = new \nwk\sound\Track(-1);

            if ($this->readTrackChunk_($track_nr) === false) {
                return false;
            }
        }

        $this->postProcessTrackData_();

        return true;
    }


    // MARK: postProcessTrackData_()
    /** トラックデータの後加工 */
    private function postProcessTrackData_() : void
    {
        $r_tracks = &$this->score_->tracks;

        // -------- (1) 音源の無いトラックは削除して詰めます
        foreach ($r_tracks as $track_nr => &$r_track) {
            if (count($r_track->sources) === 0) {
                unset($this->curr_track_states_[$track_nr]);
                unset($r_tracks[$track_nr]);
            }
        }
        $this->curr_track_states_ = array_values($this->curr_track_states_);
        $r_tracks = array_values($r_tracks);

        // -------- (2) 一番最初のノートを探し, その時間を記録します
        $first_note_time = PHP_INT_MAX;
        foreach ($r_tracks as $track_nr => &$r_track) {
            foreach ($r_track->sources as $source_nr => &$r_source) {
                foreach ($r_source->notes as $note_nr => &$r_note) {
                    if ($r_note->scale !== \nwk\sound\Note::SCALE_R) {
                        $first_note_time = min($first_note_time, $r_note->time);
                        break;
                    }
                }
            }
        }

        // -------- (3) セットアップに使ってる小節を削ります. 再生時間を計算します
        foreach ($r_tracks as $track_nr => &$r_track) {
            foreach ($r_track->sources as $source_nr => &$r_source) {
                foreach ($r_source->notes as $note_nr => &$r_note) {
                    $r_note->time -= $first_note_time;
                    if ($r_note->time < 0) {
                        $r_note->len  -= $first_note_time;
                        $r_note->time = 0;
                    }
                    $this->total_play_time_ = max($this->total_play_time_, $r_note->time + $r_note->len);
                }
            }
        }
        // 再生時間を小節単位に揃える
        //$this->total_play_time_ = ((int)(($this->total_play_time_ + $this->bar_time_ - 1) / $this->bar_time_)) * $this->bar_time_;

        // -------- (4) 末端に休符を付加します
        foreach ($r_tracks as $track_nr => &$r_track) {
            foreach ($r_track->sources as $source_nr => &$r_source) {
                $start_time = $this->curr_track_states_[$track_nr]->source_states[$source_nr]->time;
                $r_source->notes[] = new \nwk\sound\Note(
                    \nwk\sound\Note::SCALE_R,
                    0,
                    $start_time - $first_note_time,
                    $this->total_play_time_ - ($start_time - $first_note_time),
                    0);
            }
        }

        // -------- (5) 長さ 0 の音符は削除します
        foreach ($r_tracks as $track_nr => &$r_track) {
            foreach ($r_track->sources as $source_nr => &$r_source) {
                foreach ($r_source->notes as $note_nr => &$r_note) {
                    if ($r_note->len === 0) {
                        unset($r_tracks[$track_nr]->sources[$source_nr]->notes[$note_nr]);
                    }
                }
                $r_source->notes = array_values($r_source->notes);
            }
        }
    }


    // MARK: getters
    /** テンポ(四分音符の時間 usec)を返します. 例:500000(つまり♩=120) */
    public function getTempo(): int { return $this->tempo_; }
    /** 時間の解像度(四分音符の長さ)を返します. 例:480 */
    public function getTimeResolution(): int { return $this->time_resolution_; }
    /** 拍子の分子 (1小節内の音符数)を返します. 6/8 拍子ならば 6 (=8分音符が6個) */
    public function getNumerator(): int { return $this->numerator_; }
    /** 拍子の分母 (音符の種類) を返します. 6/8 拍子ならば 8 (= 8分音符) */
    public function getFraction(): int { return $this->fraction_; }
    /** 1 小節の時間(四分音符 = getTempo()換算) を返します */
    public function getBarTime(): int { return $this->bar_time_; }
    /** 演奏時間を返します(四分音符 = getTempo()換算). 小節単位にアライメントされます */
    public function getTotalPlayTime(): int { return $this->total_play_time_; }
    /** 変換した楽譜データを返します */
    public function getScore(): \nwk\sound\Score { return $this->score_; }


    // MARK: genNoteLenHistogram()
    /** 音の長さのヒストグラムを作成します. 音の長さ順にソートされてます */
    public function genNoteLenHistogram(\nwk\sound\Score $score): array
    {
        $histo = [];
        foreach ($score->tracks as &$r_track) {
            foreach ($r_track->sources as &$r_source) {
                foreach ($r_source->notes as &$r_note) {
                    $len = $r_note->len;
                    if (isset($histo[$len])) {
                        $histo[$len]++;
                    } else {
                        $histo[$len] = 1;
                    }
                }
            }
        }
        ksort($histo);
        return $histo;
    }


    // MARK: genNoteScaleHistogram()
    /** 音階のヒストグラムを作成します. ドラムは除きます. 音階順にソートされてます */
    public function genNoteScaleHistogram(\nwk\sound\Score $score): array
    {
        $histo = [];
        foreach ($score->tracks as &$r_track) {
            if ($r_track->ch == self::CH_DRUM) { continue; }
            foreach ($r_track->sources as &$r_source) {
                foreach ($r_source->notes as &$r_note) {
                    $scale = $r_note->scale;
                    if (isset($histo[$scale])) {
                        $histo[$scale]++;
                    } else {
                        $histo[$scale] = 1;
                    }
                }
            }
        }
        uksort($histo, function($a, $b):int  {
            $ai = array_search($a, \nwk\sound\Note::SCALE_STR_TAB);
            $bi = array_search($b, \nwk\sound\Note::SCALE_STR_TAB);
            return $ai - $bi;
        });
        return $histo;
    }


    // ---------------------------------------------------------------- チャンク読込
    // MARK: readHeaderChunk_()
    /** ヘッダ チャンク読込
     * @return {false|int} 成功したらトラック数, 失敗したら false を返します
     */
    private function readHeaderChunk_(): false|int
    {
        //echo(var_export($midi_data, true). "\n");

        // チャンク タイプ(MThd)のチェック
        $chunk_type = $this->read4bytes_();
        if ($chunk_type !== 0x4d546864) {
            $this->printError_(null, "ヘッダ チャンク タイプの異常です:[" . sprintf('0x%08x', $chunk_type) . "]");
            return false;
        }

        // データ長
        $len = $this->read4bytes_();
        if ($len !== 0x00000006) {
            $this->printError_(null, "ヘッダ チャンクの長さの異常です:[$len]");
            return false;
        }

        // FORMAT
        $format = $this->read2bytes_();
        if ($format !== 0x0001) {
            $this->printError_(null, "この MIDI フォーマット [$format] はサポートしません");
            return false;
        }

        // トラック数
        $nr_tracks = $this->read2bytes_();

        // 四分音符の分解能
        $this->time_resolution_ = $this->read2bytes_();
        if ($this->time_resolution_ >= 0x8000) {
            $this->printError_(null, "この分解能値 [$this->time_resolution_] はサポートしません");
            return false;
        }

        return $nr_tracks;
    }


    // MARK: readTrackChunk_()
    /** トラック チャンク読込
     * @return true/false = 成功/失敗
     */
    private function readTrackChunk_(int $track_nr): bool
    {
        $idx = 14;

        // チャンク タイプ(MTrk)のチェック
        $chunk_type = $this->read4bytes_();
        if ($chunk_type !== 0x4d54726b) {
            $this->printError_($track_nr, "トラック チャンク タイプの異常です:[" . sprintf('0x%08x', $chunk_type) . "]");
            return false;
        }

        // データ長
        $len = $this->read4bytes_();

        while (true) {
            $delta_time = $this->readDeltaTime_($track_nr); // デルタ タイム = 直前のイベントからの時間. 時間単位=四分音符とした時間
            if ($delta_time === false) {
                $this->printError_($track_nr, "メタ イベント タイプの前にデータが終わりました");
                return false;
            }

            $event = $this->read1byte_();
            if ($event === false) {
                $this->printError_($track_nr, "メタ イベントの長さの前にデータが終わりました");
                return false;
            }
            $ch = $event & 0x0f;

            switch ($event) {
            default:
                $this->printError_($track_nr, "不明なイベントです[" . sprintf('0x%02x', $event) . "]");
                return false;

            case 0x80:// Note Off
            case 0x81:
            case 0x82:
            case 0x83:
            case 0x84:
            case 0x85:
            case 0x86:
            case 0x87:
            case 0x88:
            case 0x89:
            case 0x8a:
            case 0x8b:
            case 0x8c:
            case 0x8d:
            case 0x8e:
            case 0x8f:
                {
                    $ret = $this->readNote_($track_nr, $ch, $delta_time, false);
                    if ($ret == false) {
                        return false;
                    }
                }
                break;

            case 0x90:// Note On
            case 0x91:
            case 0x92:
            case 0x93:
            case 0x94:
            case 0x95:
            case 0x96:
            case 0x97:
            case 0x98:
            case 0x99:
            case 0x9a:
            case 0x9b:
            case 0x9c:
            case 0x9d:
            case 0x9e:
            case 0x9f:
                {
                    $ret = $this->readNote_($track_nr, $ch, $delta_time, true);
                    if ($ret == false) {
                        return false;
                    }
                }
                break;

            case 0xb0:// Control Change
            case 0xb1:
            case 0xb2:
            case 0xb3:
            case 0xb4:
            case 0xb5:
            case 0xb6:
            case 0xb7:
            case 0xb8:
            case 0xb9:
            case 0xba:
            case 0xbb:
            case 0xbc:
            case 0xbd:
            case 0xbe:
            case 0xbf:
                {
                    $ret = $this->readControlChange_($track_nr, $ch, $delta_time);
                    if ($ret == false) {
                        return false;
                    }
                }
                break;

            case 0xc0:// Program Change
            case 0xc1:
            case 0xc2:
            case 0xc3:
            case 0xc4:
            case 0xc5:
            case 0xc6:
            case 0xc7:
            case 0xc8:
            case 0xc9:
            case 0xca:
            case 0xcb:
            case 0xcc:
            case 0xcd:
            case 0xce:
            case 0xcf:
            {
                $ret = $this->readProgramChange_($track_nr, $ch, $delta_time);
                if ($ret == false) {
                    return false;
                }
            }
            break;

            case 0xe0:// Pitch Wheel Change
            case 0xe1:
            case 0xe2:
            case 0xe3:
            case 0xe4:
            case 0xe5:
            case 0xe6:
            case 0xe7:
            case 0xe8:
            case 0xe9:
            case 0xea:
            case 0xeb:
            case 0xec:
            case 0xed:
            case 0xee:
            case 0xef:
            {
                $ret = $this->readPitchWheelChange_($track_nr, $ch, $delta_time);
                if ($ret == false) {
                    return false;
                }
            }
            break;

            case 0xf0: // SysEx
            case 0xf7: // SysEx
                {
                    $ret = $this->skipSysExEvent_($track_nr, $delta_time);
                    if ($ret == false) {
                        return false;
                    }
                }
                break;
            case 0xff: // Meta Event
                {
                    $ret = $this->skipMetaEvent_($track_nr, $delta_time);
                    if ($ret == false) {
                        return false;
                    } else if ($ret === -1) {
                        return true;
                    }
                }
            } // switch ($event)
        }

        return false;
    }


    // ---------------------------------------------------------------- 各メッセージの処理
    // MARK: readNote_()
    /**
     * チャンネル メッセージ(ノート)の処理
     * @param $track_nr   トラック番号
     * @param $ch         チャンネル番号
     * @param $delta_time デルタ タイム
     * @param $b_on       true/false = Note On/Note Off
     * @return true/false = 成功/失敗
     */
    private function readNote_(int $track_nr, int $ch, int $delta_time, bool $b_on): bool
    {
        if ($this->bindTrackWithCh_($track_nr, $ch) === false) {
            return false;
        }

        $scale_nr = $this->read1byte_();
        $velo     = $this->read1byte_();

        if ($scale_nr === false) {
            $this->printError_($track_nr, "ノート音階の前にデータが終わりました");
            return false;
        }
        if ($velo === false) {
            $this->printError_($track_nr, "ノート音量の前にデータが終わりました");
            return false;
        }
        if (($scale_nr < 0) || (count(\nwk\sound\Note::SCALE_STR_TAB) <= $scale_nr)) {
            $this->printError_($track_nr, "ノート音階の異常です[$scale_nr]");
            return false;
        }
        $scale = \nwk\sound\Note::SCALE_STR_TAB[$scale_nr];
        if (0x80 <= $velo) {
            $this->printError_($track_nr, "ノート音階[$scale]音量の異常です[$velo]");
            return false;
        }

        if ($b_on) {
            if ($this->noteOn_($track_nr, $ch, $scale, $velo) === false) {
                return false;
            }
        } else {
            if ($this->noteOff_($track_nr, $ch, $scale) === false) {
                return false;
            }
        }

        if ($ch !== self::CH_DRUM) {
            $this->debugNoteEcho_($this->makeInfoHeadStr_($track_nr, $ch) . 'Note ' . sprintf('%3s', $scale) . ($b_on ? ' On ' : ' Off') . " velo[$velo]\n");
        } else {// ドラム
            $this->debugNoteEcho_($this->makeInfoHeadStr_($track_nr, $ch) . sprintf('%3s', $scale) . ($b_on ? ' On ' : ' Off') . " velo[$velo]\n");
        }
        return true;
    }


    // MARK: readControlChange_()
    /**
     * チャンネル メッセージ(コントロール チェンジ)の処理
     * - 本クラスでは無視します
     * @param $track_nr   トラック番号
     * @param $ch         チャンネル番号
     * @param $delta_time デルタ タイム
     * @return true/false = 成功/失敗
     */
    private function readControlChange_(int $track_nr, int $ch, int $delta_time): bool
    {
        if ($this->bindTrackWithCh_($track_nr, $ch) === false) {
            return false;
        }

        $ctrl_nr = $this->read1byte_();
        $data    = $this->read1byte_();

        if ($ctrl_nr === false) {
            $this->printError_($track_nr, "コントロール番号の前にデータが終わりました");
            return false;
        }

        if ($data === false) {
            $this->printError_($track_nr, "コントロール データの前にデータが終わりました");
            return false;
        }
        if (0x80 <= $ctrl_nr) {
            $this->printError_($track_nr, "コントロール番号の異常です:[$ctrl_nr]");
            return false;
        }
        if (0x80 <= $data) {
            $this->printError_($track_nr, "コントロール データの異常です:[$data]");
            return false;
        }

        $this->debugEcho_($this->makeInfoHeadStr_($track_nr, $ch) . "Control Change[$ctrl_nr] data[$data]\n");
        return true;
    }


    // MARK: readProgramChange_()
    /**
     * チャンネル メッセージ(プログラム チェンジ = 音色)
     * @param $track_nr   トラック番号
     * @param $ch         チャンネル番号
     * @param $delta_time デルタ タイム
     * @return true/false = 成功/失敗
     */
    private function readProgramChange_(int $track_nr, int $ch, int $delta_time): bool
    {
        if ($this->bindTrackWithCh_($track_nr, $ch) === false) {
            return false;
        }

        $prog_nr = $this->read1byte_();

        if ($prog_nr === false) {
            $this->printError_($track_nr, "プログラム番号の前にデータが終わりました");
            return false;
        }

        //echo("Program Change track[$track_nr] tone[$prog_nr]\n");

        $this->curr_track_states_[$track_nr]->tone = $prog_nr;
        $this->debugNoteEcho_($this->makeInfoHeadStr_($track_nr, $ch) . "Program Change[$prog_nr]\n");
        return true;
    }


    // MARK: readPitchWheelChante_()
    /**
     * チャンネル メッセージ(ピッチ ホイール チェンジ)
     * - 本クラスでは無視します
     * @param $track_nr   トラック番号
     * @param $ch         チャンネル番号
     * @param $delta_time デルタ タイム
     * @return true/false = 成功/失敗
     */
    private function readPitchWheelChange_(int $track_nr, int $ch, int $delta_time): bool
    {
        if ($this->bindTrackWithCh_($track_nr, $ch) === false) {
            return false;
        }

        $data1 = $this->read1byte_();
        $data2 = $this->read1byte_();

        if ($data1 === false) {
            $this->printError_($track_nr, "ピッチ ホイール データ 1 の前にデータが終わりました");
            return false;
        }
        if ($data2 === false) {
            $this->printError_($track_nr, "ピッチ ホイール データ 2 の前にデータが終わりました");
            return false;
        }
        if (0x80 <= $data1) {
            $this->printError_($track_nr, "ピッチ ホイール データ 1 の異常です[$data1]");
            return false;
        }
        if (0x80 <= $data2) {
            $this->printError_($track_nr, "ピッチ ホイール データ 2 の異常です[$data2]");
            return false;
        }
        $data = (($data1 << 7) | $data2) - 8192;

        $this->debugEcho_($this->makeInfoHeadStr_($track_nr, $ch) . "Pitch Wheel data[$data]\n");
        return true;
    }


    // MARK: readMetaEvent_()
    /**
     * メタ イベント
     * - 本クラスでは, Tempo, Time Signature 以外は全て無視します
     * @param $track_nr   トラック番号
     * @param $delta_time デルタ タイム
     * @return true/false/-1 = 成功/失敗/トラック チャンクの終了検出
     */
    private function skipMetaEvent_(int $track_nr, int $delta_time): bool|int
    {
        $type  = $this->read1byte_();
        $len   = $this->read1byte_();
        $need_seek = true;
        if ($type === false) {
            $this->printError_($track_nr, "メタ イベント タイプの前にデータが終わりました");
            return false;
        }
        if ($len === false) {
            $this->printError_($track_nr, "メタ長さの前にデータが終わりました");
            return false;
        }
        $event_name = '';
        $event_info = '';
        $b_invalid  = false;

        switch ($type) {
        default:
            $this->printError_($track_nr, "メタ イベント タイプの異常です[$type]");
            return false;
        case 0x01:  // コメント イベント
            $event_name = "Text";
            break;
        case 0x02:
            $event_name = "Copyright";
            break;
        case 0x03:
            $event_name = "Sequence/Track name";
            break;
        case 0x04:
            $event_name = "Instrument name";
            break;
        case 0x05:
            $event_name = "Lyrics";
            break;
        case 0x06:
            $event_name = "Marker";
            break;
        case 0x07:
            $event_name = "Queue Point";
            break;
        case 0x20:
            $event_name = "Channel Prefix";
            if ($len !== 1) { $b_invalid = true; }
            break;
        case 0x21:
            $event_name = "Post Prefix";
            if ($len !== 1) { $b_invalid = true; }
            break;
        case 0x2f:
            $event_name = "End of Track";
            if ($len !== 0) { $b_invalid = true; }
            break;
        case 0x51:
            $event_name = "Set Tempo";
            if ($len != 3) { $b_invalid = true; break; }
            $this->tempo_ = $this->read3bytes_();
            $event_info = $this->tempo_;
            $need_seek = false;
            break;
        case 0x54:
            $event_name = "SMPTE Offset";
            if ($len != 5) { $b_invalid = true; }
            break;
        case 0x58:
            $event_name = "Time Signature";// n分のf拍子
            if ($len != 4) { $b_invalid = true; break; }
            {
                $this->numerator_ = $this->read1byte_();       // 拍子の分子
                $this->fraction_  = 2 ** $this->read1byte_();  // 拍子の分母(音符の種類. 3 なら, 2 ^ -3 = 1/8 音符)
                // 6/8 拍子なら, 分子が 6, 分母が -3 になります
                $metronome = $this->read1byte_();       // メトロノーム間隔(無視)
                $midi_clk  = $this->read1byte_();       // 四分音符中の 32分音符の数(通常8)(無視)
                if ($this->numerator_ === false) {
                    $this->printError_($track_nr, "タイム シグネチャ分子の前にデータが終わりました");
                    return false;
                }
                if ($this->fraction_ === false) {
                    $this->printError_($track_nr, "タイム シグネチャ分母の前にデータが終わりました");
                    return false;
                }
                $this->bar_time_ = (int)((4 / $this->fraction_) * $this->numerator_ * $this->time_resolution_);

                $event_info = '' . $this->numerator_ . '/' . $this->fraction_;
                $need_seek = false;
            }
            break;
        case 0x59:
            $event_name = "Key Signature";
            if ($len != 2) { $b_invalid = true; }
            break;
        case 0x7f:
            $event_name = "Sequencer Specific Meta Event";
            break;

        } // switch ($eventType)

        if ($b_invalid) {
            $this->printError_($track_nr, "Invalid $event_name", -1);
            return false;
        }
        if ($need_seek) {
            $this->relativeSeek_($len);
        }
        $this->debugEcho_($this->makeInfoHeadStr_($track_nr, null) . "Meta Event[" . sprintf("0x%02x", $type) . "($event_name)] len[$len] $event_info\n");
        return ($type == 0x2f) ? -1 : true;
    }


    // MARK: readSysExEvent_()
    /**
     * エクスクルーシブ イベント
     * - 本クラスでは無視します
     * @param $track_nr   トラック番号
     * @param $delta_time デルタ タイム
     * @return true/false = 成功/失敗
     */
    private function skipSysExEvent_(int $track_nr, int $delta_time)
    {
        $len = $this->readVariableLengthNumber_();
        if ($len === false) {
            $this->printError_($track_nr, "SysEx Length の前にデータが終わりました");
        }
        $this->debugEcho_($this->makeInfoHeadStr_($track_nr, null) . "SysEx Event len[$len]\n");
        $this->relativeSeek_($len);
        return true;
    }


    // MARK: bindTrackWithCh_()
    /** トラックとチャンネルを結びつけます
     * - 既にトラックが結び付けられてる場合は, 矛盾してないかチェックします
     * @return true/false = 成功/失敗
     */
    private function bindTrackWithCh_(int $track_nr, int $ch): bool
    {
        $r_tracks = $this->score_->tracks;

        if ($r_tracks[$track_nr]->ch === -1) {
            $r_tracks[$track_nr]->ch = $ch;
        } else if ($r_tracks[$track_nr]->ch != $ch) {
            $ch2 = $r_tracks[$track_nr]->ch;
            $this->printError("トラック[$track_nr]のチャンネル[$ch]が以前セットした値[$ch2]と異なります. チャンネルは1つのトラックしか指定できません\n");
            return false;
        }
        return true;
    }


    // ---------------------------------------------------------------- ノート詳細
    // MARK: noteOn_()
    /** ノート ON
     * @param $track_nr   トラック番号
     * @param $ch         チャンネル番号
     * @param $scale      音階
     * @param $velo       音量
     * @return true/false = 成功/失敗
     */
    private function noteOn_(int $track_nr, int $ch, string $scale, int $velo): bool
    {
        $source_nr = $this->searchSourceNr_($track_nr, $scale);
        $time = $this->curr_track_states_[$track_nr]->time;
        $curr_source_state = null;

        if ($source_nr !== -1) {
            // 同じ音階が鳴ってる音源があれば, それを止めます.
            $curr_source_state = $this->curr_track_states_[$track_nr]->source_states[$source_nr];
        } else {
            // 別の音ならば, 休符状態の音源を探します
            $source_nr = $this->searchSourceNr_($track_nr, \nwk\sound\Note::SCALE_R);
            if ($source_nr === -1) {
                // 休符状態の音源が見つからないならば, 音源を追加して, 演奏開始以来休符状態とします
                $source_nr = count($this->curr_track_states_[$track_nr]->source_states); // 追加した音源の番号
                $this->score_->tracks[$track_nr]->sources[] = new \nwk\sound\Source();
                $this->curr_track_states_[$track_nr]->source_states[] = new CurrentSourceState_(
                    \nwk\sound\Note::SCALE_R,
                    0,
                    0,
                    0);
            }
            $curr_source_state = $this->curr_track_states_[$track_nr]->source_states[$source_nr];
        }

        // 現在の状態を止めて楽譜を確定します
        if ($time !== $curr_source_state->time) {
            $this->score_->tracks[$track_nr]->sources[$source_nr]->notes[] = new \nwk\sound\Note(
                $curr_source_state->scale,
                $curr_source_state->tone,
                $curr_source_state->time,
                $time - $curr_source_state->time,
                $curr_source_state->velo);
        }

        //if ($track_nr===6){echo("$track_nr $source_nr $scale " . $this->curr_track_states_[$track_nr]->tone. "\n");}

        // 状態を記録します
        $this->curr_track_states_[$track_nr]->source_states[$source_nr] = new CurrentSourceState_(
            $scale,
            $this->curr_track_states_[$track_nr]->tone,
            $time,
            $velo);
        return true;
    }


    // MARK: noteOff_()
    /** ノート OFF
     * @param $track_nr   トラック番号
     * @param $ch         チャンネル番号
     * @param $scale      音階
     * @return true/false = 成功/失敗
     */
    private function noteOff_(int $track_nr, int $ch, string $scale): bool
    {
        // 今鳴ってるのと同じノートの音を止めます
        $source_nr = $this->searchSourceNr_($track_nr, $scale);
        $time      = $this->curr_track_states_[$track_nr]->time;

        if ($source_nr !== -1) {
            $curr_source_state = $this->curr_track_states_[$track_nr]->source_states[$source_nr];
            $this->score_->tracks[$track_nr]->sources[$source_nr]->notes[] = new \nwk\sound\Note(
                $curr_source_state->scale,
                $curr_source_state->tone,
                $curr_source_state->time,
                $time - $curr_source_state->time,
                $curr_source_state->velo);
        } else {
            $this->printWarn_($track_nr, "ON になってないノート(音階:[$scale])を, OFF にしようとしてます. 無視しますが, 音が重なっている可能性があります. この付近の楽譜を確認してください");
        }

        // 無音を鳴らします
        $this->curr_track_states_[$track_nr]->source_states[$source_nr] = new CurrentSourceState_(
            \nwk\sound\Note::SCALE_R,
            0,
            $time,
            0);

        return true;
    }


    // ---------------------------------------------------------------- データ読取
    // MARK: isEOF_()
    /** データ終了か判定します */
    private function isEOF_(): bool
    {
        return count($this->data_) <= $this->index_;
    }


    // MARK: relativeSeek_()
    /** 現在の位置を移動します */
    private function relativeSeek_(int $rel): bool
    {
        $this->index_ += $rel;
        if ($this->index_ < 0) {
            $this->index_ = 0;
            return false;
        }
        if (count($this->data_) <= $this->index_) {
            return false;
        }
        return true;
    }


    // MARK: read1byte_()
    /** 1 バイトを読込 */
    private function read1byte_(): bool|int
    {
        if (count($this->data_) <= $this->index_) {
            return false;
        }
        $ret = $this->data_[$this->index_];
        $this->index_ ++;
        return $ret;
    }


    // MARK: read2bytes_()
    /** 2 バイトを読込 */
    private function read2bytes_(): bool|int
    {
        if (count($this->data_) <= $this->index_ + 1) {
            return false;
        }
        $ret = ($this->data_[$this->index_] << 8) | $this->data_[$this->index_ + 1];
        $this->index_ += 2;
        return $ret;
    }


    // MARK: read3bytes_()
    /** 2 バイトを読込 */
    private function read3bytes_(): bool|int
    {
        if (count($this->data_) <= $this->index_ + 2) {
            return false;
        }
        $ret = ($this->data_[$this->index_] << 16) | ($this->data_[$this->index_ + 1] << 8) | $this->data_[$this->index_ + 2];
        $this->index_ += 3;
        return $ret;
    }


    // MARK: read4bytes_()
    /** 4 バイトを読込 */
    private function read4bytes_(): bool|int
    {
        if (count($this->data_) <= $this->index_ + 3) {
            return false;
        }
        $ret = ($this->data_[$this->index_] << 24) | ($this->data_[$this->index_ + 1] << 16) | ($this->data_[$this->index_ + 2] << 8) | $this->data_[$this->index_ + 3];
        $this->index_ += 4;
        return $ret;
    }


    // MARK: readDeltaTime_()
    /** デルタ タイムを読込 */
    private function readDeltaTime_(int $track_nr): bool|int
    {
        $len = $this->readVariableLengthNumber_();
        if ($len === false) { return false; }
        $this->curr_track_states_[$track_nr]->time += $len;
        return $len;
    }


    // MARK: readVariableLengthNumber_()
    /** 可変長数値を読込 */
    private function readVariableLengthNumber_(): bool|int
    {
        // - オリジナルの長さを下から 7bit 単位に区切る
        // - 各 7bit の最上位 bit に '1' を追加します. 但し, 最後のバイトは '0'
        // - 例: 元の値                                        10000 00000000 = 0x1000
        //   下から7bit に区切って,                          0100000  0000000
        //   各 7bit の最上位 bit に '1' か '0' を付加して  10100000 00000000 = 0xa000

        $ret = 0;
        $r0 = $this->read1byte_();
        if ($r0 === false) { return false; }
        if ($r0 < 0x80) {
            $ret = $r0;
        } else {
            $r1 = $this->read1byte_();
            if ($r1 === false) { return false; }
            if ($r1 < 0x80) {
                $ret = (($r0 & 0x7f) << 7) | $r1;
            } else {
                $r2 = $this->read1byte_();
                if ($r2 === false) { return false; }
                if ($r2 < 0x80) {
                    $ret = (($r0 & 0x7f) << 14) | (($r1 & 0x7f) << 7) | $r2;
                } else {
                    $r3 = $this->read1byte_();
                    if ($r3 === false) { return false; }
                    if ($r3 < 0x80) {
                        $ret = (($r0 & 0x7f) << 28) | (($r1 & 0x7f) << 14) | (($r2 & 0x7f) << 7) | $r3;
                    } else {
                        return false; // 4 バイト形式の先頭バイトは 0x80 未満
                    }
                }
            }
        }
        return $ret;
    }


    // ---------------------------------------------------------------- ユーティリティ
    // MARK: searchTrackNr_()
    /** データの中に, 指定したチャンネル番号のトラックを検索します.
     * @param  チャンネル番号(0～)
     * @return トラック番号. 無ければ -1
     */
    private function searchTrackNr_(int $ch) : int
    {
        foreach ($this->score_->tracks as $track_nr => &$r_track) {
            if ($r_track->ch === $ch) {
                return $track_nr;
            }
        }
        return -1;
    }


    // MARK: searchSourceNr_()
    /** 現在のトラックで, 指定した音階が鳴ってる音源番号を探します
     * @param $track_nr トラック番号(0～)
     * @param $scale 音階
     * @return 音源番号(0～), 無ければ -1
     */
    private function searchSourceNr_(int $track_nr, string $scale) : int
    {
        foreach ($this->curr_track_states_[$track_nr]->source_states as $source_nr => &$r_source_state) {
            if ($r_source_state->scale === $scale) {
                return $source_nr;
            }
        }
        return -1;
    }


    // ---------------------------------------------------------------- 表示, デバッグ
    // MARK: createScoreInfo()
    /** 楽譜の情報文字列を返します */
    public function createScoreInfo(): string
    {
        $tempo = $this->getTempo();
        $time_resolution = $this->getTimeResolution();
        $numerator = $this->getNumerator();
        $fraction = $this->getFraction();
        $bar_time = $this->getBarTime();
        $total_play_time = $this->getTotalPlayTime();
        $ret =  "// 四分音符の時間:$tempo usec(♩=". (1000000 / $tempo * 60) .")\n" .
                "// 四分音符の解像度:$time_resolution\n".
                "// $numerator/$fraction 拍子\n" .
                "// 1小節の時間:$bar_time\n" .
                "// 演奏時間:$total_play_time(" . ($total_play_time / $bar_time) . "小節)\n";

        $ret .= "// 音長ヒストグラム:\n";
        foreach ($this->genNoteLenHistogram($this->score_) as $len => $nr) {
            $ret .= "//  $len x $nr\n";
        }
        $ret .= "// 音階ヒストグラム:\n";
        foreach ($this->genNoteScaleHistogram($this->score_) as $scale => $nr) {
            $ret .= "//  $scale x $nr\n";
        }

        return $ret;
    }


    // MARK: createScoreDump()
    /** 楽譜の内容を表示します. $score を省略するとインスタンス内の情報を返します */
    public function debugScoreDump(\nwk\sound\Score $score = null): void
    {
        if (!$score) {
            $score = $this->score_;
        }
        echo($this->createScoreInfo());

        foreach ($score->tracks as $track_nr => &$r_track) {
            $ch = $r_track->ch;
            echo("Track[$track_nr]\n");
            foreach ($r_track->sources as $source_nr => &$r_source) {
                echo("  音源[$source_nr](ch[$ch])\n");
                $ch++;
                foreach ($r_source->notes as $note_nr => &$r_note) {
                    $scale   = substr($r_note->scale . '   ', 0, 4);
                    $tone = '        ';
                    if ($r_note->scale !== \nwk\sound\Note::SCALE_R) {
                        $tone  = '音色[' . substr(' '     . $r_note->tone, -2, 2) . ']';
                    }
                    $timelen = substr('     ' . $r_note->time, -6, 6) . '#' . substr('   ' . $r_note->len, -4, 4);
                    $velo    = substr('  '    . $r_note->velo, -3, 3);
                    echo("    $scale $tone 時間[$timelen] 音量[$velo]\n");
                }
            }
        }
    }


    // MARK: debugEcho_()
    private function debugEcho_(string $msg): void
    {
        if (self::DEBUG_) {
            echo($msg);
        }
    }


    // MARK: debugNoteEcho_()
    private function debugNoteEcho_(string $msg): void
    {
        if (self::DEBUG_NOTE_) {
            echo($msg);
        }
    }


    // MARK: makeInfoHeadStr_()
    /** バイト位置, 時間, 小節番号を含む情報文字列を返します */
    private function makeInfoHeadStr_(?int $track_nr, ?int $ch): string
    {
        $ret = sprintf('addr[0x%08x] ', $this->index_);
        if ($track_nr !== null && $this->bar_time_ != 0) {
            $time = $this->curr_track_states_[$track_nr]->time;
            $ret .= sprintf('time[%6d]', $time);
//            $note_time = $this->time_[$track_nr] - $this->note_time_;
//            $bar = (int)($note_time / $this->bar_time_);
            $ch = $ch === null ? '    ' : (' ch' . $ch);
//            $ret .= '(bar[' . sprintf('%2d', $bar) . '] + time[' . sprintf('%4d', $note_time - $bar * $this->bar_time_) . '])' . $ch;
            $ret .= sprintf('time[%4d]) %s', $time, $ch);
        } else {
            $ret .= sprintf('addr[0x%08x]             ', $this->index_);
        }
        $ret .= ': ';
        return $ret;
    }


    // MARK: printWarn_()
    private function printWarn_(?int $track_nr, string $msg): void
    {
        fwrite(STDERR, 'WARN: ' . $this->makeInfoHeadStr_($track_nr, null) . "$msg\n");
    }


    // MARK: printError_()
    private function printError_(?int $track_nr, string $msg): void
    {
        fwrite(STDERR, 'ERROR: ' . $this->makeInfoHeadStr_($track_nr, null) . "$msg\n");
    }
}
