<?php

declare(strict_types = 1);
/**
 * おざなり MIDI → sd3Play() 形式の MML に変換. C ソースの形で出力します
 * 使い方は, Usage: 行を参照してください
 *
 * - MIDI FORMAT 1 のみ対応
 * - 同時発音は最大3音
 * - 音程は C2～B4 (3オクターブ)ですが C4 以降の高い音は聞きづらいかも
 * - ノード以外の殆どのイベントは解析しません(スキップします)
 * - midi ファイル名の末端が '_barX' ならば, X 小節単位に分割します
 *
 * -  ソースは, トラックに分解されたデータ(mml1_x, mml2_x, mml3_x, xは分割小節番号0～) です.
 * - 再生速度にマクロ L の定義が必要です. 出力されるソースを観てください.
 *
 * @author Snail Barbarian Macho (NWK) 2021.09.06
 */

// --------------------------------
const NOTE_STR_TAB = [
    'C-1', 'CS-1', 'D-1', 'DS-1', 'E-1', 'F-1', 'FS-1', 'G-1', 'GS-1', 'A-1', 'AS-1', 'B-1',
    'C0', 'CS0', 'D0', 'DS0', 'E0', 'F0', 'FS0', 'G0', 'GS0', 'A0', 'AS0', 'B0',
    'C1', 'CS1', 'D1', 'DS1', 'E1', 'F1', 'FS1', 'G1', 'GS1', 'A1', 'AS1', 'B1',
    'C2', 'CS2', 'D2', 'DS2', 'E2', 'F2', 'FS2', 'G2', 'GS2', 'A2', 'AS2', 'B2',
    'C3', 'CS3', 'D3', 'DS3', 'E3', 'F3', 'FS3', 'G3', 'GS3', 'A3', 'AS3', 'B3',
    'C4', 'CS4', 'D4', 'DS4', 'E4', 'F4', 'FS4', 'G4', 'GS4', 'A4', 'AS4', 'B4',
    'C5', 'CS5', 'D5', 'DS5', 'E5', 'F5', 'FS5', 'G5', 'GS5', 'A5', 'AS5', 'B5',
    'C6', 'CS6', 'D6', 'DS6', 'E6', 'F6', 'FS6', 'G6', 'GS6', 'A6', 'AS6', 'B6',
    'C7', 'CS7', 'D7', 'DS7', 'E7', 'F7', 'FS7', 'G7', 'GS7', 'A7', 'AS7', 'B7',
    'C8', 'CS8', 'D8', 'DS8', 'E8', 'F8', 'FS8', 'G8', 'GS8', 'A8', 'AS8', 'B8',
    'C9', 'CS9', 'D9', 'DS9', 'E9', 'F9', 'FS9', 'G9',
];
const DEBUG = 0;     // 1 にするとノートを除くイベントの詳細を出力します. 出力される詳細はコードでないのでコンパイル通りませんので注意
const DEBUG_NOTE = 0;// 1 にするとノート イベントの詳細を出力します.      出力される詳細はコードでないのでコンパイル通りませんので注意

// 引数チェック
if (count($argv) != 3) {
    fwrite(STDERR, 'Usage: php ' . $argv[0] . " source-midi.mdi out-data.h\n");
    exit(1);
}

$in_midi_file = $argv[1];
$out_c_file   = $argv[2];

// midi ファイル名の末端が '-barX' ならば, X 小節単位に分割します
$nr_bars = 100000; // 1変数当たりの小節数(デフォルトは大きい値)
$matches = [];
if (preg_match('/-bar([0-9]+)$/', pathinfo($in_midi_file, PATHINFO_FILENAME), $matches) === 1) {
    $nr_bars = (int)$matches[1];
    if ($nr_bars < 1) {
        fwrite(STDERR, "Invalid value nr_bars\n");
        exit(1);
    }
}

// ファイル存在チェック
if (file_exists($in_midi_file) === false) {
    fwrite(STDERR, "File not found[$in_midi_file]\n");
    exit(1);
}

// MIDI ファイル読込
$midi_data = file_get_contents($in_midi_file);
if ($midi_data === false) {
    fwrite(STDERR, "File read error[$in_midi_file]\n");
    exit(1);
}

// unpackで作られる配列は 1 からなので, arrat_merge() で 0 からにします
$reader = new MidiReader(array_merge(unpack('C*', $midi_data)));

// ヘッダ チャンク読込
$midi_header_info = readMidiHeader($reader);
if ($midi_header_info === false) {
    fwrite(STDERR, "MIDI header chunk error[$in_midi_file]\n");
    exit(1);
}

// トラック チャンク読込
$time_div = $midi_header_info['time_div']; // 時間単位 = 四分音符 / $time_div
for ($i = 0; $i < $midi_header_info['nr_tracks']; $i++) {
    debugEcho('Track[' . $i + 1 . "]\n");
    $midi_track_info = readMidiTrack($reader, $time_div);
    if ($midi_track_info === false) {
        fwrite(STDERR, "MIDI track chunk error[$in_midi_file]\n");
        exit(1);
    }
}

$out_str = outHeader($reader, $argv[0], $time_div, $nr_bars);
$mmlss = $reader->getMml();
if ($nr_bars == 1) {
    $out_str .= outMml1($mmlss);
} else {
    $out_str .= outMmlN($mmlss, $nr_bars);
}
file_put_contents($out_c_file, $out_str);


// --------------------------------
/** ヘッダ解析
 * @param MidiReader $reader MIDI バイトデータの配列リーダー
 * @return false|array 成功したら MIDI 情報, 失敗したら false を返します
 */
function readMidiHeader(MidiReader $reader): false|array
{
    //echo(var_export($midi_data, true). "\n");

    // チャンク タイプ(MThd)のチェック
    $chunk_type = $reader->read4byte();
    if ($chunk_type !== 0x4d546864) {
        $reader->printError("Invalid header chunk type:[" . sprintf('0x%08x', $chunk_type) . "]", -4);
        return false;
    }
    // データ長
    $len = $reader->read4byte();
    if ($len !== 0x00000006) {
        $reader->printError("Invalid header chunk length[$len]", -4);
        return false;
    }


    // FORMAT
    $format = $reader->read2byte();
    if ($format !== 0x0001) {
        $reader->printError("Invalid header chunk. Format[$format]. is not supported", -2);
        return false;
    }

    // トラック数
    $nr_tracks = $reader->read2byte();

    // 時間分割
    $time_div = $reader->read2byte();
    if ($time_div >= 0x8000) {
        $reader->printError("Invalid header chunk. Time Division(Absolute time)is not supported", -2);
        return false;
    }

    $info['nr_tracks'] = $nr_tracks;
    $info['time_div'] = $time_div; // 四分音符の分解能. 主に 480
    // echo(var_export($midi_header_info, true). "\n");
    return $info;
}

// --------------------------------
/** トラック解析
 * @param MidiReader $reader MIDI バイトデータの配列リーダー
 * @return false|array 成功したら MIDI 情報, 失敗したら false を返します
 */
function readMidiTrack(MidiReader $reader, int $time_div): false|array
{
    $idx = 14;

    // チャンク タイプ(MTrk)のチェック
    $chunk_type = $reader->read4byte();
    if ($chunk_type !== 0x4d54726b) {
        $reader->printError("Invalid track chunk type:[" . sprintf('0x%08x', $chunk_type) . "]", -4);
        return false;
    }

    // データ長
    $len = $reader->read4byte();
    $info = [];

    while (true) {
        $delta_time = $reader->readDeltaTime();
        $event     = $reader->read1byte();
        if ($delta_time === false) {
            $reader->printError("EOF(Meta Event Type)");
            return false;
        }
        if ($event === false) {
            $reader->printError("EOF(Mata Length)");
            return false;
        }

        switch ($event) {
        default:
            $reader->printError("Unknown event[" . sprintf('0x%02x', $event) . "]", -2);
            return false;

        case 0x80:// Note Off fall through
        case 0x81://          fall through
        case 0x82://          fall through
        case 0x83://          fall through
        case 0x84://          fall through
        case 0x85://          fall through
        case 0x86://          fall through
        case 0x87://          fall through
        case 0x88://          fall through
        case 0x89://          fall through
        case 0x8a://          fall through
        case 0x8b://          fall through
        case 0x8c://          fall through
        case 0x8d://          fall through
        case 0x8e://          fall through
        case 0x8f://          fall through
            {
                $ret = readNote($reader, $delta_time, $event & 0x0f, false,  $time_div);
                if ($ret == false) {
                    return false;
                }
            }
            break;

        case 0x90:// Note On  fall through
        case 0x91://          fall through
        case 0x92://          fall through
        case 0x93://          fall through
        case 0x94://          fall through
        case 0x95://          fall through
        case 0x96://          fall through
        case 0x97://          fall through
        case 0x98://          fall through
        case 0x99://          fall through
        case 0x9a://          fall through
        case 0x9b://          fall through
        case 0x9c://          fall through
        case 0x9d://          fall through
        case 0x9e://          fall through
        case 0x9f://
            {
                $ret = readNote($reader, $delta_time, $event & 0x0f, true, $time_div);
                if ($ret == false) {
                    return false;
                }
            }
            break;

        case 0xb0:// Control Change fall through
        case 0xb1://                fall through
        case 0xb2://                fall through
        case 0xb3://                fall through
        case 0xb4://                fall through
        case 0xb5://                fall through
        case 0xb6://                fall through
        case 0xb7://                fall through
        case 0xb8://                fall through
        case 0xb9://                fall through
        case 0xba://                fall through
        case 0xbb://                fall through
        case 0xbc://                fall through
        case 0xbd://                fall through
        case 0xbe://                fall through
        case 0xbf://
            {
                $ret = readControlChange($reader, $delta_time, $event & 0x0f);
                if ($ret == false) {
                    return false;
                }
            }
            break;

        case 0xc0:// Program Change fall through
        case 0xc1://                fall through
        case 0xc2://                fall through
        case 0xc3://                fall through
        case 0xc4://                fall through
        case 0xc5://                fall through
        case 0xc6://                fall through
        case 0xc7://                fall through
        case 0xc8://                fall through
        case 0xc9://                fall through
        case 0xca://                fall through
        case 0xcb://                fall through
        case 0xcc://                fall through
        case 0xcd://                fall through
        case 0xce://                fall through
        case 0xcf://
        {
            $ret = readProgramChange($reader, $delta_time, $event & 0x0f);
            if ($ret == false) {
                return false;
            }
        }
        break;

        case 0xe0:// Pitch Wheel Change fall through
        case 0xe1://                    fall through
        case 0xe2://                    fall through
        case 0xe3://                    fall through
        case 0xe4://                    fall through
        case 0xe5://                    fall through
        case 0xe6://                    fall through
        case 0xe7://                    fall through
        case 0xe8://                    fall through
        case 0xe9://                    fall through
        case 0xea://                    fall through
        case 0xeb://                    fall through
        case 0xec://                    fall through
        case 0xed://                    fall through
        case 0xee://                    fall through
        case 0xef://
        {
            $ret = readPitchWheelChange($reader, $delta_time, $event & 0x0f);
            if ($ret == false) {
                return false;
            }
        }
        break;

        case 0xf0: // SysEx
        case 0xf7: // SysEx fall througn
            {
                $ret = skipSysExEvent($reader, $delta_time);
                if ($ret == false) {
                    return false;
                }
            }
            break;
        case 0xff: // Meta Event
            {
                $ret = skipMetaEvent($reader, $delta_time, $time_div);
                if ($ret == false) {
                    return false;
                } else if ($ret === -1) {
                    return $info;
                }
            }
        } // switch ($event)
    }

    return false;
}

// --------------------------------
/**
 * チャンネル メッセージ(ノート)
 * @param int $ch チャンネル番号
 * @param bool $bOn true/false ON/OFF
 */
function readNote(MidiReader $reader, int $delta_time, int $ch, bool $bOn, int $time_div): bool
{
    $index    = $reader->getIndex() - 1;
    $note_nr  = $reader->read1byte();
    $velocity = $reader->read1byte();
    if ($note_nr === false) {
        $reader->printError("EOF(Note number)", -1);
        return false;
    }
    if ($velocity === false) {
        $reader->printError("EOF(Note velocity)");
        return false;
    }
    $note_name = NOTE_STR_TAB[$note_nr];
    if (($note_nr < 31) || (67 < $note_nr)) {  // G1 ～ G4 まで
        $reader->printError("Invalid Note number[$note_nr($note_name)]. The range of notes is 31(G1) to 67(G4).", -1);
        return false;
    }
    if (0x80 <= $velocity) {
        $reader->printError("Invalid Note velocity[$velocity]", -1);
        return false;
    }

    if ($bOn) {
        $reader->noteOn($note_name, $time_div);
    } else {
        $reader->noteOff($note_name, $time_div, false);
    }

    debugNoteEcho($reader->makeInfoHeadStr() . 'Note ' . sprintf('%3s', $note_name) . ($bOn ? ' On ' : ' Off') . " velocity[$velocity]\n");
    return true;
}

// --------------------------------
/**
 * チャンネル メッセージ(コントロール チェンジ)
 * @param int $ch チャンネル番号
 */
function readControlChange(MidiReader $reader, int $delta_time, int $ch): bool
{
    $index   = $reader->getIndex() - 1;
    $ctrl_nr = $reader->read1byte();
    $data    = $reader->read1byte();
    if ($ctrl_nr === false) {
        $reader->printError("EOF(Control number)");
        return false;
    }
    if ($data === false) {
        $reader->printError("EOF(Control data)");
        return false;
    }
    if (0x80 <= $ctrl_nr) {
        $reader->printError("Invalid Control number[$ctrl_nr]", -1);
        return false;
    }
    if (0x80 <= $data) {
        $reader->printError("Invalid Control data[$data]", -1);
        return false;
    }

    debugEcho($reader->makeInfoHeadStr() . "Control Change nr[$ctrl_nr] data[$data]\n");
    return true;
}


// --------------------------------
/**
 * チャンネル メッセージ(プログラム チェンジ)
 * @param int $ch チャンネル番号
 */
function readProgramChange(MidiReader $reader, int $delta_time, int $ch): bool
{
    $index   = $reader->getIndex() - 1;
    $prog_nr = $reader->read1byte();
    if ($prog_nr === false) {
        $reader->printError("EOF(Program number)");
        return false;
    }
    if (0x80 <= $prog_nr) {
        $reader->printError("Invalid Program number[$prog_nr]", -1);
        return false;
    }
    debugEcho($reader->makeInfoHeadStr() . "Program Change nr[$prog_nr]\n");
    return true;
}

// --------------------------------
/**
 * チャンネル メッセージ(ピッチ ホイール チェンジ)
 * @param int $ch チャンネル番号
 */
function readPitchWheelChange(MidiReader $reader, int $delta_time, int $ch): bool
{
    $index = $reader->getIndex() - 1;
    $data1 = $reader->read1byte();
    $data2 = $reader->read1byte();
    if ($data1 === false) {
        $reader->printError("EOF(Pitch Wheel data1)");
        return false;
    }
    if ($data2 === false) {
        $reader->printError("EOF(Pitch Wheel data2)");
        return false;
    }
    if (0x80 <= $data1) {
        $reader->printError("Invalid Piych Wheel data1[$data1]", -1);
        return false;
    }
    if (0x80 <= $data2) {
        $reader->printError("Invalid Piych Wheel data1[$data2]", -1);
        return false;
    }
    $data = (($data1 << 7) | $data2) - 8192;

    debugEcho($reader->makeInfoHeadStr() . "Pitch Wheel data[$data]\n");
    return true;
}


// --------------------------------
/**
 * メタ イベント
 * Time Signature 以外は全て無視します
 * @return bool|int bool の場合は成功/失敗. トラック チャンクの終了を検出したら -1 を返す
 */
function skipMetaEvent(MidiReader $reader, int $delta_time, int $time_div): bool|int
{
    $index = $reader->getIndex() - 1;
    $type  = $reader->read1byte();
    $len   = $reader->read1byte();
    if ($type === false) {
        $reader->printError("EOF(Meta Event Type)");
        return false;
    }
    if ($len === false) {
        $reader->printError("EOF(Mata Length)");
        return false;
    }
    $event_name = '';
    $event_info = '';
    $b_invalid  = false;

    switch ($type) {
    default:
        $reader->printError("Invalid Meta event type[$type]", -2);
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
        if ($len != 3) { $b_invalid = true; }
        break;
    case 0x54:
        $event_name = "SMPTE Offset";
        if ($len != 5) { $b_invalid = true; }
        break;
    case 0x58:
        $event_name = "Time Signature";
        if ($len != 4) { $b_invalid = true; break; }
        {
            $numerator = $reader->read1byte();      // 分子(1 小節内の音符数)
            $fraction  = 2 ** $reader->read1byte();  // 分母(音符の種類. 3 なら, 2 ^ 3 = 8 分音符)
            $metronome = $reader->read1byte(); // メトロノーム間隔(無視)
            $midi_clk  = $reader->read1byte(); // MIDI クロック中の 32分音符の数(通常8)(無視)
            if ($numerator === false) {
                $reader->printError("EOF(Time Sigunature numerator)");
                return false;
            }
            if ($fraction === false) {
                $reader->printError("EOF(Time Sigunature fraction)");
                return false;
            }
            $event_info = "$numerator/$fraction";
            $len = 0;
            $reader->setBarTime((int)((4 / $fraction) * $numerator * $time_div));
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
        $reader->printError("Invalid $event_name", -1);
        return false;
    }

    $reader->relativeSeek($len);
    debugEcho($reader->makeInfoHeadStr() . "Meta Event[" . sprintf("0x%02x", $type) . "($event_name)] len[$len] $event_info\n");
    return ($type == 0x2f) ? -1 : true;
}

// --------------------------------
/**
 * エクスクルーシブ イベント
 * すべて無視します
 * @return bool 成功/失敗
 */
function skipSysExEvent($reader, int $delta_time)
{
    $index = $reader->getIndex();
    $len   = $reader->readVariableLengthNumber();
    if ($len === false) {
        $reader->printError("EOF(SysEx Length)", -1);
    }
    debugEcho($reader->makeInfoHeadStr() . "SysEx Event len[$len]\n");
    $reader->relativeSeek($len);
    return true;
}

// -------------------------------- デバッグ用 echo
function debugEcho(string $msg): void
{
    if (DEBUG) {
        echo($msg);
    }
}
function debugNoteEcho(string $msg): void
{
    if (DEBUG_NOTE) {
        echo($msg);
    }
}

// -------------------------------- 出力
function outHeader(MidiReader $reader, string $app_name, int $time_div, int $nr_bars): string
{
    $len_hist = $reader->getLengthHistgram();
    $ret  = "/* This file is made by **** $app_name ****.  DO NOT MODIFY! */\n";
    $ret .= "//#define L 8 // 四分音符の長さ\n";

    foreach($len_hist as $key => $val) {
        $ret .= sprintf('#define L%-7s', $key) . "((u32)L * $key / " . $time_div . ")    // L * " . ($key / $time_div) . "\n";
    }

    $ret .= "// 1小節の長さは, L" . $reader->getBarTime() .  " 相当です\n";
    $ret .= "// 1変数当たりの小節数: $nr_bars\n";

    return $ret;
}

/**
 * 1 変数 1 小節ならば, 1 行で,
 * mmlx_y = { ... }; の様に出力します
 * @param array $mmlss mml文字列[トラック数][小節数]
 */
function outMml1(array $mmlss): string
{
    // データ比較用のコメント無し文字列を作成
    $mmlss_no_comment = $mmlss;
    foreach ($mmlss_no_comment as &$mmls) {
        foreach ($mmls as &$mml) {
            $mml = preg_replace('/\/\*.*?\*\//s', '', $mml);
            $mml = preg_replace('/[ \t\r\n　]/s', '', $mml);
        }
    }

    // 同じ文字列かを比較. 超手抜き
    foreach ($mmlss_no_comment as $track_nr1 => &$mmls1) {
        foreach ($mmls1 as $bar_nr1 => &$mml1) {
            foreach ($mmlss_no_comment as $track_nr2 => &$mmls2) {
                foreach ($mmls2 as $bar_nr2 => &$mml2) {
                    if (($track_nr1 === $track_nr2) && ($bar_nr1 === $bar_nr2)) { continue; }
                    if (strcmp($mml1, $mml2) === 0) {
                        //echo('mml' . $track_nr1 . '_' . $bar_nr1 . '[' . $mml1 . '] and mml' . $track_nr2 . '_' . $bar_nr2 . '[' . $mml2 . "] are same.\n");
                        $mml2 = 'mml' . $track_nr1 . '_' . $bar_nr1;
                        $mmlss[$track_nr2][$bar_nr2] = $mml2;
                    }
                }
            }
        }
    }

    // 出力
    $ret = '';
    foreach ($mmlss as $track_nr => $mmls) {
        $ret .= "\n// track $track_nr\n";
        foreach ($mmls as $bar_nr => $mml) {
            $var_name = 'mml' . $track_nr . "_" . $bar_nr;
            if (str_starts_with($mml, 'mml')) {
                $ret .= '#define ' . $var_name . ' ' . $mml . "\n";
            } else {
                $ret .= 'static u8 const ' . $var_name . '[] = { ' . $mml . "0 }; \n";
            }
        }
    }
    return $ret;
}

/**
 * 1 変数 2 小節以上ならば複数行で,
 * mmlx_y = {
 *   ...
 *   ...
 * }; の様に出力します
 * @param array $mmlss mml文字列[トラック数][小節数]
 */
function outMmlN(array $mmlss, int $nr_bars): string
{
    $mmlss2 = [[]];

    // 小節を結合します
    foreach ($mmlss as $track_nr => $mmls) {
        $str = '';
        $ct  = 0;
        foreach ($mmls as $bar_nr => $mml) {
            $str .= '    ' . $mml . "\n";
            $ct++;
            if ($nr_bars <= $ct) {
                $ct = 0;
                $mmlss2[$track_nr][] = $str;
                $str = '';
            }
        }
        if ($str !== '') {
            $mmlss2[$track_nr][] = $str;
        }
    }

    // データ比較用のコメント無し文字列を作成
    $mmlss2_no_comment = $mmlss2;
    foreach ($mmlss2_no_comment as &$mmls) {
        foreach ($mmls as &$mml) {
            $mml = preg_replace('/\/\*.*?\*\//s', '', $mml);
            $mml = preg_replace('/[ \t\r\n　]/s', '', $mml);
        }
    }

    //echo(var_export($mmlss2_no_comment, true));

    // 同じ文字列かを比較. 超手抜き
    foreach ($mmlss2_no_comment as $track_nr1 => &$mmls1) {
        foreach ($mmls1 as $bar_nr1 => &$mml1) {
            foreach ($mmlss2_no_comment as $track_nr2 => &$mmls2) {
                foreach ($mmls2 as $bar_nr2 => &$mml2) {
                    if (($track_nr1 === $track_nr2) && ($bar_nr1 === $bar_nr2)) { continue; }
                    if (strcmp($mml1, $mml2) === 0) {
                        //echo('mml' . $track_nr1 . '_' . $bar_nr1 . '[' . $mml1 . '] and mml' . $track_nr2 . '_' . $bar_nr2 . '[' . $mml2 . "] are same.\n");
                        $mml2 = 'mml' . $track_nr1 . '_' . $bar_nr1;
                        $mmlss2[$track_nr2][$bar_nr2] = $mml2;
                    }
                }
            }
        }
    }

    // 出力
    $ret = '';
    foreach ($mmlss2 as $track_nr => $mmls) {
        $ret .= "\n// track $track_nr\n";
        foreach ($mmls as $bar_nr => $mml) {
            $var_name = 'mml' . $track_nr . "_" . $bar_nr;
            if (str_starts_with($mml, 'mml')) {
                $ret .= '#define ' . $var_name . ' ' . $mml . "\n";
            } else {
                $ret .= 'static u8 const ' . $var_name . "[] = {\n" . $mml . "    0 };\n";
            }
        }
    }
    return $ret;
}

// -------------------------------- MIDI データを読んで, MML を貯えこむクラス
class MidiReader {
    private array $data      = [];
    private int   $index     = 0;
    private int   $time      = 0;     // 絶対時間
    private int   $note_time = 0;     // 最初のノートが出現した時間
    private int   $bar_time  = 0;     // 1 小節分の時間(0:小節時間は計算しない)
    private array $tracks    = [];    // 各トラックの状況. 内容はコンストラクタ参照
    private array $len_hist  = [];    // 音長ヒストグラム [長さ => 1]

    public function __construct(array $data) {
        $this->data      = $data;
        $this->index     = 0;
        $this->time      = 0;
        $this->note_time = 0;
        $this->bar_time  = 0;
        $this->tracks    = [
            // 使用中フラグ, ノート名, 開始時間, mml 文字列
            [ 'b_in_use' => false, 'note_name' => '',  'time' => 0, 'mmls' => [ '' ], 'bar_time' => 0, 'bar_nr' => 0 ],
            [ 'b_in_use' => false, 'note_name' => '',  'time' => 0, 'mmls' => [ '' ], 'bar_time' => 0, 'bar_nr' => 0 ],
            [ 'b_in_use' => false, 'note_name' => '',  'time' => 0, 'mmls' => [ '' ], 'bar_time' => 0, 'bar_nr' => 0 ],
        ];
        $this->len_hist  = [];
    }

    // -------- ノード関連
    public function getTime(): int
    {
        return $this->time;
    }
    public function getBarTime(): int
    {
        return $this->bar_time;
    }

    public function setBarTime(int $bar_time): void
    {
        $this->bar_time = $bar_time;
        $this->bar_time_add = 0;
    }

    public function noteOn(string $note_name, int $time_div): void
    {
        if ($this->note_time === 0) {
            $this->note_time = $this->time;
            foreach ($this->tracks as &$track) {
                $track['time'] = $this->time;
            }
        }
        $this->checkTime($time_div);

        // 今鳴ってる同じ音を止める. もしそのような音が無くても警告しない
        $this->noteOff($note_name, $time_div, true);
        // 空いてるトラックに入れる. 全部ふさがってたら警告します
        // 残りの空きトラックも同じ音を入れる
        $b_in_use = true;
        foreach ($this->tracks as $track_nr => &$track) {
            if ($track['b_in_use'] !== false) {               // 使用中
                continue;
            }
            if ($track['note_name'] !== '') {                // そのトラックで, 何か音が鳴ってたら OFF します
                $this->trackNoteOff($track_nr, $track);
            }
            if ($track['time'] !== $this->time) {           // 無音部分があります !
                $this->printError('No-sound gap detected! node[' . $note_name . '] track[' . $track_nr . '] end time[' . $track['time'] . '] start time[' . $this->time . ']');
                exit(1);
            }
            $track['b_in_use']  = $b_in_use;                // 有効になるのは最初のトラックだけ
            $track['note_name'] = $note_name;
            $track['time']      = $this->time;
            $b_in_use = false;
        }
        // トラックに入りきれなかった音は警告
        if ($b_in_use) {
            $this->printWarn('Over track! note[' . $note_name . ']');
        }
    }

    public function noteOff(string $note_name, int $time_div, bool $bIgnoreWarn): void
    {
        // 今鳴ってるのと同じノートの音を全部止めます
        $bOff = false;
        $this->checkTime($time_div);

        foreach ($this->tracks as $track_nr => &$track) {
            if ($track['note_name'] === $note_name) {
                $this->trackNoteOff($track_nr, $track);
                $bOff = true;
            }
        }

        // 存在しないノードがあった場合は警告
        if (($bIgnoreWarn === false) && ($bOff === false)) {
            $this->printWarn('Note[' . $note_name . '] is not turned ON');
        }
    }

    private function trackNoteOff(int $track_nr, array &$track): void
    {
        {
            $len  = $this->time - $track['time'];
            if ($len == 0) { return; }  // 長さ 0 の音はないだろうと思いますが, 念のため
            $mml_last_idx = count($track['mmls']) - 1;

            // 波長→音長の順に出力
            $track['mmls'][$mml_last_idx] .= 'SD3_' . $track['note_name'] . ', L' . $len. ', ';

            // 音長→波長の順に出力
            //$track['mmls'][$mml_last_idx] .= 'L' . $len. ', SD3_' . $track['note_name'] . ', ';

            //echo('time[' . $this->time . '] ノートOFF ['. $track['note_name'] . '] len[' . $len . "]\n");
            //if ($track_nr == 0) echo('time[' . $this->time . '] ノートOFF index['. $mml_last_idx . ']=[' . $track['mmls'][$mml_last_idx] . "]\n");

            $track['note_name'] = '';
            $track['b_in_use']  = false;
            $track['bar_time'] += $len;
            $track['time']     += $len; // 終了時間を記録しておく
            $this->len_hist[$len] = 1;
        }

        // 小節単位で改行
        if ($this->bar_time !== 0) {
            while ($this->bar_time <= $track['bar_time']) {
                $mml_last_idx = count($track['mmls']) - 1;
                $track['bar_time'] -= $this->bar_time;
                $track['mmls'][$mml_last_idx] .= '/* 小節' . $track['bar_nr'] ;
                if ($track['bar_time'] !== 0) {
                    $track['mmls'][$mml_last_idx] .= ' ※最後の音符が, [' . $track['bar_time'] . '] だけ次の小節にはみ出してます';
                }
                $track['mmls'][$mml_last_idx] .= ' */';
                //if ($track_nr == 0) echo('['. $track['mmls'][$mml_last_idx] . "]\n");
                $track['mmls'][] = '';// 次の小節を追加
                $track['bar_nr']++;
            }
        }
    }

    public function getMml(): array
    {
        $ret = [];
        foreach ($this->tracks as $track_nr => $track) {
            $ret[$track_nr] = [];
            foreach ($track['mmls'] as $mml) {
                if ($mml === '') { break; }
                $ret[$track_nr][] = $mml;
            }
            //echo (var_export($ret[count($ret)-1], true) . "\n");
        }
        return $ret;
    }

    public function getLengthHistgram(): array
    {
        ksort($this->len_hist);
        return $this->len_hist;
    }

    /** $this->time が, $time_div の 1/4, 1/3 の倍数でなければエラー終了 */
    public function checkTime($time_div): void
    {
        if ($this->note_time != 0) {
            $td1_4 = $time_div / 4;
            $td1_3 = $time_div / 3;
            $t = $this->time - $this->note_time;
            if (((int)($t / $td1_4) * $td1_4 !== $t) &&
                ((int)($t / $td1_3) * $td1_3 !== $t)) {
                $this->printError('Invalid time[' . $t . ']. The time must be a multiple of ' . $td1_4 . ' or ' . $td1_3 . ".\n");
                exit(1);
            }
        }
    }

    // -------- データ アクセス関連
    public function getIndex(): int
    {
        return $this->index;
    }

    public function isEOF(): bool
    {
        return count($this->data) <= $this->index;
    }

    public function relativeSeek(int $rel): bool
    {
        $this->index += $rel;
        if ($this->index < 0) {
            $this->index = 0;
            return false;
        }
        if (count($this->data) <= $this->index) {
            return false;
        }
        return true;
    }


    public function read1byte(): bool|int
    {
        if (count($this->data) <= $this->index) {
            return false;
        }
        $ret = $this->data[$this->index];
        $this->index ++;
        return $ret;
    }

    public function read2byte(): bool|int
    {
        if (count($this->data) <= $this->index + 1) {
            return false;
        }
        $ret = ($this->data[$this->index] << 8) | $this->data[$this->index + 1];
        $this->index += 2;
        return $ret;
    }

    public function read4byte(): bool|int
    {
        if (count($this->data) <= $this->index + 3) {
            return false;
        }
        $ret = ($this->data[$this->index] << 24) | ($this->data[$this->index + 1] << 16) | ($this->data[$this->index + 2] << 8) | $this->data[$this->index + 3];
        $this->index += 4;
        return $ret;
    }

    public function readDeltaTime(): bool|int
    {
        $n = $this->readVariableLengthNumber();
        if ($n === false) { return false; }
        $this->time += $n;
        return $n;
    }

    // 可変長数字のリード
    public function readVariableLengthNumber(): bool|int
    {
        // - オリジナルの長さを下から 7bit 単位に区切る
        // - 各 7bit の最上位 bit に '1' を追加します. 但し, 最後のバイトは '0'
        // - 例: 元の値                                        10000 00000000 = 0x1000
        //   下から7bit に区切って,                          0100000  0000000
        //   各 7bit の最上位 bit に '1' か '0' を付加して  10100000 00000000 = 0xa000

        $ret = 0;
        $r0 = $this->read1byte();
        if ($r0 === false) { return false; }
        if ($r0 < 0x80) {
            $ret = $r0;
        } else {
            $r1 = $this->read1byte();
            if ($r1 === false) { return false; }
            if ($r1 < 0x80) {
                $ret = (($r0 & 0x7f) << 7) | $r1;
            } else {
                $r2 = $this->read1byte();
                if ($r2 === false) { return false; }
                if ($r2 < 0x80) {
                    $ret = (($r0 & 0x7f) << 14) | (($r1 & 0x7f) << 7) | $r2;
                } else {
                    $r3 = $this->read1byte();
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

    // -------- 情報関連
    /** バイト位置, 時間, 小節番号を含む情報文字列を返します */
    public function makeInfoHeadStr(int $idx_offset = 0): string
    {

        $ret = sprintf('addr[0x%08x', $this->index - $idx_offset) . '] time[' . sprintf('%6d', $this->time) . ']';
        if ($this->bar_time !== 0) {
            $note_time = $this->time - $this->note_time;
            $bar = (int)($note_time / $this->bar_time);
            $ret .= '(bar[' . sprintf('%2d', $bar) . '] + time[' . sprintf('%4d', $note_time - $bar * $this->bar_time) . '])';
        }
        $ret .= ': ';
        return $ret;
    }

    function printWarn(string $msg, int $inedxOffset = 0): void
    {
        fwrite(STDERR, 'WARN: ' . $this->makeInfoHeadStr() . "$msg\n");
    }

    function printError(string $msg, int $inedxOffset = 0): void
    {
        fwrite(STDERR, 'ERROR: ' . $this->makeInfoHeadStr() . "$msg\n");
    }
}
