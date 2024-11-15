<?php
/** 音楽データ保持クラス
 *
 * @author Snail Barbarian Macho (NWK) 2024.06.29
 */

declare(strict_types = 1);
namespace nwk\sound;

require_once 'midi.class.php';

// -------------------------------- 音楽データ保持クラス
// MARK: Score
/** 楽譜 */
class Score {
    public array $tracks;       // トラックの配列
    function __construct()
    {
        $this->tracks = [];
    }
};


// MARK: Track
/** トラック */
class Track {
    public int   $ch;           // チャンネル番号
    public array $sources;      // 音源の配列
    function __construct(int $ch)
    {
        $this->ch = $ch;
        $this->sources = [];
    }
};


// MARK: Source
/** 音源 */
class Source {
    public array  $notes;           // 音符の配列
    function __construct()
    {
        $this->notes = [];
    }
};


// MARK: Note
/** 音符 */
class Note {
    // 音階種類文字列
    public const SCALE_STR_TAB = [
        'C-1', 'C#-1', 'D-1', 'D#-1', 'E-1', 'F-1', 'F#-1', 'G-1', 'G#-1', 'A-1', 'A#-1', 'B-1',
        'C0',  'C#0',  'D0',  'D#0',  'E0',  'F0',  'F#0',  'G0',  'G#0',  'A0',  'A#0',  'B0',
        'C1',  'C#1',  'D1',  'D#1',  'E1',  'F1',  'F#1',  'G1',  'G#1',  'A1',  'A#1',  'B1',
        'C2',  'C#2',  'D2',  'D#2',  'E2',  'F2',  'F#2',  'G2',  'G#2',  'A2',  'A#2',  'B2',
        'C3',  'C#3',  'D3',  'D#3',  'E3',  'F3',  'F#3',  'G3',  'G#3',  'A3',  'A#3',  'B3',
        'C4',  'C#4',  'D4',  'D#4',  'E4',  'F4',  'F#4',  'G4',  'G#4',  'A4',  'A#4',  'B4',
        'C5',  'C#5',  'D5',  'D#5',  'E5',  'F5',  'F#5',  'G5',  'G#5',  'A5',  'A#5',  'B5',
        'C6',  'C#6',  'D6',  'D#6',  'E6',  'F6',  'F#6',  'G6',  'G#6',  'A6',  'A#6',  'B6',
        'C7',  'C#7',  'D7',  'D#7',  'E7',  'F7',  'F#7',  'G7',  'G#7',  'A7',  'A#7',  'B7',
        'C8',  'C#8',  'D8',  'D#8',  'E8',  'F8',  'F#8',  'G8',  'G#8',  'A8',  'A#8',  'B8',
        'C9',  'C#9',  'D9',  'D#9',  'E9',  'F9',  'F#9',  'G9',
    ];
    public const SCALE_R = 'R';

    // 音色
    public const TONE_PIANO1 = 0;
    public const TONE_PIANO2 = 1;
    public const TONE_PIANO3 = 2;
    public const TONE_PIANO4 = 3;
    public const TONE_E_PIANO1 = 4;
    public const TONE_E_PIANO2 = 5;
    public const TONE_HARPSICHORD = 6;
    public const TONE_CLAV = 7;

    public string $scale;           // 音階. 'C-1'～'G9' または 'R' (ドラムの場合は, ドラムの種類)
    public int    $tone;            // 音色. 0～128(ドラムの場合は, 0 固定)
    public int    $time;            // 開始時間. 数値 (デフォルトでは, 四分音符=480)
    public int    $len;             // 音長. 数値 (デフォルトでは, 四分音符=480)
    public int    $velo;            // 音量. [0, 100]
    // 以下は楽譜としては未使用. コンバーターで使用します(いずれは廃止したいーーー)
    public string $data_scale = ''; // データ形式に変換した音階

    function __construct(string $scale, int $tone, int $time, int $len, int $velo)
    {
        $this->scale = $scale;
        $this->tone  = $tone;
        $this->time  = $time;
        $this->len   = $len;
        $this->velo  = $velo;
    }

    /* 音符の情報文字列を作成します. エラー時に使用します */
    public function genInfo(int $bar_time): string
    {
        if ($bar_time == 0) {
            return " 時間[$this->time]:[$this->scale]";
        } else {
            $bar = sprintf('%6.3f', ($this->time / $bar_time) + \nwk\sound\MidiReader::BAR_OFFSET);
            return " 小節[$bar]:[$this->scale]";
        }
    }

};
