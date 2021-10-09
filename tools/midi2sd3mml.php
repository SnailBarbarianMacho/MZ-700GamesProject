<?php

declare(strict_types = 1);
/**
 * おざなり MIDI → sd3Play() 形式の MML に変換. C ソースの形で出力します
 * 使い方は, Usage: 行を参照してください
 *
 * - FORMAT 1 のみ対応
 * - ノード以外の殆どのイベントは解析しません(スキップします)
 * - midi ファイル名の末端が '_barX' ならば, X 小節単位に分割します
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

$inMidiFile = $argv[1];
$outCFile   = $argv[2];

// midi ファイル名の末端が '_barX' ならば, X 小節単位に分割します
$nrBars = 100000; // 1変数当たりの小節数(デフォルトは大きい値)
$matches = [];
if (preg_match('/_bar([0-9]+)$/', pathinfo($inMidiFile, PATHINFO_FILENAME), $matches) === 1) {
    $nrBars = (int)$matches[1];
    if ($nrBars < 1) {
        fwrite(STDERR, "Invalid value nr_bars\n");
        exit(1);
    }
}

// ファイル存在チェック
if (file_exists($inMidiFile) === false) {
    fwrite(STDERR, "File not found[$inMidiFile]\n");
    exit(1);
}

// MIDI ファイル読込
$midiData = file_get_contents($inMidiFile);
if ($midiData === false) {
    fwrite(STDERR, "File read error[$inMidiFile]\n");
    exit(1);
}

// unpackで作られる配列は 1 からなので, arrat_merge() で 0 からにします
$reader = new MidiReader(array_merge(unpack('C*', $midiData)));

// ヘッダ チャンク読込
$midiHeaderInfo = readMidiHeader($reader);
if ($midiHeaderInfo === false) {
    fwrite(STDERR, "MIDI header chunk error[$inMidiFile]\n");
    exit(1);
}

// トラック チャンク読込
$timeDiv = $midiHeaderInfo['timeDivision']; // 時間単位 = 四分音符 / $timeDiv
for ($i = 0; $i < $midiHeaderInfo['nrTracks']; $i++) {
    debugEcho('Track[' . $i + 1 . "]\n");
    $midiTrackInfo = readMidiTrack($reader, $timeDiv);
    if ($midiTrackInfo === false) {
        fwrite(STDERR, "MIDI track chunk error[$inMidiFile]\n");
        exit(1);
    }
}

$outStr = outHeader($reader, $argv[0], $timeDiv, $nrBars);
$mmlss = $reader->getMml();
if ($nrBars == 1) {
    $outStr .= outMml1($mmlss);
} else {
    $outStr .= outMmlN($mmlss, $nrBars);
}
file_put_contents($outCFile, $outStr);


// --------------------------------
/** ヘッダ解析
 * @param MidiReader $reader MIDI バイトデータの配列リーダー
 * @return false|array 成功したら MIDI 情報, 失敗したら false を返します
 */
function readMidiHeader(MidiReader $reader): false|array
{
    //echo(var_export($midiData, true). "\n");

    // チャンク タイプ(MThd)のチェック
    $chunkType = $reader->read4byte();
    if ($chunkType !== 0x4d546864) {
        $reader->printError("Invalid header chunk type:[" . sprintf('0x%08x', $chunkType) . "]", -4);
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
    $nrTracks = $reader->read2byte();

    // 時間分割
    $timeDivision = $reader->read2byte();
    if ($timeDivision >= 0x8000) {
        $reader->printError("Invalid header chunk. Time Division(Absolute time)is not supported", -2);
        return false;
    }

    $info['nrTracks'] = $nrTracks;
    $info['timeDivision'] = $timeDivision; // 四分音符の分解能. 主に 480
    // echo(var_export($midiHeaderInfo, true). "\n");
    return $info;
}

// --------------------------------
/** トラック解析
 * @param MidiReader $reader MIDI バイトデータの配列リーダー
 * @return false|array 成功したら MIDI 情報, 失敗したら false を返します
 */
function readMidiTrack(MidiReader $reader, int $timeDivision): false|array
{
    $idx = 14;

    // チャンク タイプ(MTrk)のチェック
    $chunkType = $reader->read4byte();
    if ($chunkType !== 0x4d54726b) {
        $reader->printError("Invalid track chunk type:[" . sprintf('0x%08x', $chunkType) . "]", -4);
        return false;
    }

    // データ長
    $len = $reader->read4byte();
    $info = [];

    while (true) {
        $deltaTime = $reader->readDeltaTime();
        $event     = $reader->read1byte();
        if ($deltaTime === false) {
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
                $ret = readNote($reader, $deltaTime, $event & 0x0f, false,  $timeDivision);
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
                $ret = readNote($reader, $deltaTime, $event & 0x0f, true, $timeDivision);
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
                $ret = readControlChange($reader, $deltaTime, $event & 0x0f);
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
            $ret = readProgramChange($reader, $deltaTime, $event & 0x0f);
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
            $ret = readPitchWheelChange($reader, $deltaTime, $event & 0x0f);
            if ($ret == false) {
                return false;
            }
        }
        break;

        case 0xf0: // SysEx
        case 0xf7: // SysEx fall througn
            {
                $ret = skipSysExEvent($reader, $deltaTime);
                if ($ret == false) {
                    return false;
                }
            }
            break;
        case 0xff: // Meta Event
            {
                $ret = skipMetaEvent($reader, $deltaTime, $timeDivision);
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
function readNote(MidiReader $reader, int $deltaTime, int $ch, bool $bOn, int $timeDivision): bool
{
    $index    = $reader->getIndex() - 1;
    $noteNr   = $reader->read1byte();
    $velocity = $reader->read1byte();
    if ($noteNr === false) {
        $reader->printError("EOF(Note number)", -1);
        return false;
    }
    if ($velocity === false) {
        $reader->printError("EOF(Note velocity)");
        return false;
    }
    $noteName =  NOTE_STR_TAB[$noteNr];
    if (($noteNr < 31) || (67 < $noteNr)) {  // G1 ～ G4 まで
        $reader->printError("Invalid Note number[$noteNr($noteName)]. The range of notes is 31(G1) to 67(G4).", -1);
        return false;
    }
    if (0x80 <= $velocity) {
        $reader->printError("Invalid Note velocity[$velocity]", -1);
        return false;
    }

    if ($bOn) {
        $reader->noteOn($noteName, $timeDivision);
    } else {
        $reader->noteOff($noteName, $timeDivision, false);
    }

    debugNoteEcho($reader->makeInfoHeadStr() . 'Note ' . sprintf('%3s', $noteName) . ($bOn ? ' On ' : ' Off') . " velocity[$velocity]\n");
    return true;
}

// --------------------------------
/**
 * チャンネル メッセージ(コントロール チェンジ)
 * @param int $ch チャンネル番号
 */
function readControlChange(MidiReader $reader, int $deltaTime, int $ch): bool
{
    $index = $reader->getIndex() - 1;
    $ctrlNr = $reader->read1byte();
    $data   = $reader->read1byte();
    if ($ctrlNr === false) {
        $reader->printError("EOF(Control number)");
        return false;
    }
    if ($data === false) {
        $reader->printError("EOF(Control data)");
        return false;
    }
    if (0x80 <= $ctrlNr) {
        $reader->printError("Invalid Control number[$ctrlNr]", -1);
        return false;
    }
    if (0x80 <= $data) {
        $reader->printError("Invalid Control data[$data]", -1);
        return false;
    }

    debugEcho($reader->makeInfoHeadStr() . "Control Change nr[$ctrlNr] data[$data]\n");
    return true;
}


// --------------------------------
/**
 * チャンネル メッセージ(プログラム チェンジ)
 * @param int $ch チャンネル番号
 */
function readProgramChange(MidiReader $reader, int $deltaTime, int $ch): bool
{
    $index = $reader->getIndex() - 1;
    $progNr = $reader->read1byte();
    if ($progNr === false) {
        $reader->printError("EOF(Program number)");
        return false;
    }
    if (0x80 <= $progNr) {
        $reader->printError("Invalid Program number[$progNr]", -1);
        return false;
    }
    debugEcho($reader->makeInfoHeadStr() . "Program Change nr[$progNr]\n");
    return true;
}

// --------------------------------
/**
 * チャンネル メッセージ(ピッチ ホイール チェンジ)
 * @param int $ch チャンネル番号
 */
function readPitchWheelChange(MidiReader $reader, int $deltaTime, int $ch): bool
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
function skipMetaEvent(MidiReader $reader, int $deltaTime, int $timeDivision): bool|int
{
    $index = $reader->getIndex() - 1;
    $type = $reader->read1byte();
    $len  = $reader->read1byte();
    if ($type === false) {
        $reader->printError("EOF(Meta Event Type)");
        return false;
    }
    if ($len === false) {
        $reader->printError("EOF(Mata Length)");
        return false;
    }
    $eventName = '';
    $eventInfo = '';
    $bInvalid = false;

    switch ($type) {
    default:
        $reader->printError("Invalid Meta event type[$type]", -2);
        return false;
    case 0x01:  // コメント イベント
        $eventName = "Text";
        break;
    case 0x02:
        $eventName = "Copyright";
        break;
    case 0x03:
        $eventName = "Sequence/Track name";
        break;
    case 0x04:
        $eventName = "Instrument name";
        break;
    case 0x05:
        $eventName = "Lyrics";
        break;
    case 0x06:
        $eventName = "Marker";
        break;
    case 0x07:
        $eventName = "Queue Point";
        break;
    case 0x20:
        $eventName = "Channel Prefix";
        if ($len !== 1) { $bInvalid = true; }
        break;
    case 0x21:
        $eventName = "Post Prefix";
        if ($len !== 1) { $bInvalid = true; }
        break;
    case 0x2f:
        $eventName = "End of Track";
        if ($len !== 0) { $bInvalid = true; }
        break;
    case 0x51:
        $eventName = "Set Tempo";
        if ($len != 3) { $bInvalid = true; }
        break;
    case 0x54:
        $eventName = "SMPTE Offset";
        if ($len != 5) { $bInvalid = true; }
        break;
    case 0x58:
        $eventName = "Time Signature";
        if ($len != 4) { $bInvalid = true; break; }
        {
            $numerator = $reader->read1byte();      // 分子(1 小節内の音符数)
            $fraction  = 2 ** $reader->read1byte();  // 分母(音符の種類. 3 なら, 2 ^ 3 = 8 分音符)
            $metronome = $reader->read1byte(); // メトロノーム間隔(無視)
            $midiClock = $reader->read1byte(); // MIDI クロック中の 32分音符の数(通常8)(無視)
            if ($numerator === false) {
                $reader->printError("EOF(Time Sigunature numerator)");
                return false;
            }
            if ($fraction === false) {
                $reader->printError("EOF(Time Sigunature fraction)");
                return false;
            }
            $eventInfo = "$numerator/$fraction";
            $len = 0;
            $reader->setBarTime((int)((4 / $fraction) * $numerator * $timeDivision));
        }
        break;
    case 0x59:
        $eventName = "Key Signature";
        if ($len != 2) { $bInvalid = true; }
        break;
    case 0x7f:
        $eventName = "Sequencer Specific Meta Event";
        break;

    } // switch ($eventType)

    if ($bInvalid) {
        $reader->printError("Invalid $eventName", -1);
        return false;
    }

    $reader->relativeSeek($len);
    debugEcho($reader->makeInfoHeadStr() . "Meta Event[" . sprintf("0x%02x", $type) . "($eventName)] len[$len] $eventInfo\n");
    return ($type == 0x2f) ? -1 : true;
}

// --------------------------------
/**
 * エクスクルーシブ イベント
 * すべて無視します
 * @return bool 成功/失敗
 */
function skipSysExEvent($reader, int $deltaTime)
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
function outHeader(MidiReader $reader, string $appName, int $timeDiv, int $nrBars): string
{
    $lenHist = $reader->getLengthHistgram();
    $ret  = "/* This file is made by **** $appName ****.  DO NOT MODIFY! */\n";
    $ret .= "//#define L 8 // 四分音符の長さ\n";

    foreach($lenHist as $key => $val) {
        $ret .= sprintf('#define L%-7s', $key) . "((u32)L * $key / " . $timeDiv . ")    // L * " . ($key / $timeDiv) . "\n";
    }

    $ret .= "// 1小節の長さは, L" . $reader->getBarTime() .  " 相当です\n";
    $ret .= "// 1変数当たりの小節数: $nrBars\n";

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
    $mmlss_noComment = $mmlss;
    foreach ($mmlss_noComment as &$mmls) {
        foreach ($mmls as &$mml) {
            $mml = preg_replace('/\/\*.*?\*\//s', '', $mml);
            $mml = preg_replace('/[ \t\r\n　]/s', '', $mml);
        }
    }

    // 同じ文字列かを比較. 超手抜き
    foreach ($mmlss_noComment as $trackNr1 => &$mmls1) {
        foreach ($mmls1 as $barNr1 => &$mml1) {
            foreach ($mmlss_noComment as $trackNr2 => &$mmls2) {
                foreach ($mmls2 as $barNr2 => &$mml2) {
                    if (($trackNr1 === $trackNr2) && ($barNr1 === $barNr2)) { continue; }
                    if (strcmp($mml1, $mml2) === 0) {
                        //echo('mml' . $trackNr1 . '_' . $barNr1 . '[' . $mml1 . '] and mml' . $trackNr2 . '_' . $barNr2 . '[' . $mml2 . "] are same.\n");
                        $mml2 = 'mml' . $trackNr1 . '_' . $barNr1;
                        $mmlss[$trackNr2][$barNr2] = $mml2;
                    }
                }
            }
        }
    }

    // 出力
    $ret = '';
    foreach ($mmlss as $trackNr => $mmls) {
        $ret .= "\n// track $trackNr\n";
        foreach ($mmls as $barNr => $mml) {
            $varName = 'mml' . $trackNr . "_" . $barNr;
            if (str_starts_with($mml, 'mml')) {
                $ret .= '#define ' . $varName . ' ' . $mml . "\n";
            } else {
                $ret .= 'static const u8 ' . $varName . '[] = { ' . $mml . "0 }; \n";
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
function outMmlN(array $mmlss, int $nrBars): string
{
    $mmlss2 = [[]];

    // 小節を結合します
    foreach ($mmlss as $trackNr => $mmls) {
        $str = '';
        $ct  = 0;
        foreach ($mmls as $barNr => $mml) {
            $str .= '    ' . $mml . "\n";
            $ct++;
            if ($nrBars <= $ct) {
                $ct = 0;
                $mmlss2[$trackNr][] = $str;
                $str = '';
            }
        }
        if ($str !== '') {
            $mmlss2[$trackNr][] = $str;
        }
    }

    // データ比較用のコメント無し文字列を作成
    $mmlss2_noComment = $mmlss2;
    foreach ($mmlss2_noComment as &$mmls) {
        foreach ($mmls as &$mml) {
            $mml = preg_replace('/\/\*.*?\*\//s', '', $mml);
            $mml = preg_replace('/[ \t\r\n　]/s', '', $mml);
        }
    }

    //echo(var_export($mmlss2_noComment, true));

    // 同じ文字列かを比較. 超手抜き
    foreach ($mmlss2_noComment as $trackNr1 => &$mmls1) {
        foreach ($mmls1 as $barNr1 => &$mml1) {
            foreach ($mmlss2_noComment as $trackNr2 => &$mmls2) {
                foreach ($mmls2 as $barNr2 => &$mml2) {
                    if (($trackNr1 === $trackNr2) && ($barNr1 === $barNr2)) { continue; }
                    if (strcmp($mml1, $mml2) === 0) {
                        //echo('mml' . $trackNr1 . '_' . $barNr1 . '[' . $mml1 . '] and mml' . $trackNr2 . '_' . $barNr2 . '[' . $mml2 . "] are same.\n");
                        $mml2 = 'mml' . $trackNr1 . '_' . $barNr1;
                        $mmlss2[$trackNr2][$barNr2] = $mml2;
                    }
                }
            }
        }
    }

    // 出力
    $ret = '';
    foreach ($mmlss2 as $trackNr => $mmls) {
        $ret .= "\n// track $trackNr\n";
        foreach ($mmls as $barNr => $mml) {
            $varName = 'mml' . $trackNr . "_" . $barNr;
            if (str_starts_with($mml, 'mml')) {
                $ret .= '#define ' . $varName . ' ' . $mml . "\n";
            } else {
                $ret .= 'static const u8 ' . $varName . "[] = {\n" . $mml . "    0 };\n";
            }
        }
    }
    return $ret;
}

// -------------------------------- MIDI データを読んで, MML を貯えこむクラス
class MidiReader {
    private array $data    = [];
    private int   $index   = 0;
    private int   $time    = 0;     // 絶対時間
    private int   $noteTime = 0;    // 最初のノートが出現した時間
    private int   $barTime = 0;     // 1 小節分の時間(0:小節時間は計算しない)
    private array $tracks  = [];    // 各トラックの状況. 内容はコンストラクタ参照
    private array $lenHist = [];    // 音長ヒストグラム [長さ => 1]

    public function __construct(array $data) {
        $this->data    = $data;
        $this->index   = 0;
        $this->time    = 0;
        $this->noteTime = 0;
        $this->barTime = 0;
        $this->tracks  = [
            // 使用中フラグ, ノート名, 開始時間, mml 文字列
            [ 'bInUse' => false, 'noteName' => '',  'time' => 0, 'mmls' => [ '' ], 'barTime' => 0, 'barNr' => 0 ],
            [ 'bInUse' => false, 'noteName' => '',  'time' => 0, 'mmls' => [ '' ], 'barTime' => 0, 'barNr' => 0 ],
            [ 'bInUse' => false, 'noteName' => '',  'time' => 0, 'mmls' => [ '' ], 'barTime' => 0, 'barNr' => 0 ],
        ];
        $this->lenHist  = [];
    }

    // -------- ノード関連
    public function getTime(): int
    {
        return $this->time;
    }
    public function getBarTime(): int
    {
        return $this->barTime;
    }

    public function setBarTime(int $barTime): void
    {
        $this->barTime = $barTime;
        $this->barTimeAdd = 0;
    }

    public function noteOn(string $noteName, int $timeDivision): void
    {
        if ($this->noteTime === 0) {
            $this->noteTime = $this->time;
            foreach ($this->tracks as &$track) {
                $track['time'] = $this->time;
            }
        }
        $this->checkTime($timeDivision);

        // 今鳴ってる同じ音を止める. もしそのような音が無くても警告しない
        $this->noteOff($noteName, $timeDivision, true);
        // 空いてるトラックに入れる. 全部ふさがってたら警告します
        // 残りの空きトラックも同じ音を入れる
        $bInUse = true;
        foreach ($this->tracks as $trackNr => &$track) {
            if ($track['bInUse'] !== false) {               // 使用中
                continue;
            }
            if ($track['noteName'] !== '') {                // そのトラックで, 何か音が鳴ってたら OFF します
                $this->trackNoteOff($trackNr, $track);
            }
            if ($track['time'] !== $this->time) {           // 無音部分があります !
                $this->printError('No-sound gap detected! node[' . $noteName . '] track[' . $trackNr . '] end time[' . $track['time'] . '] start time[' . $this->time . ']');
                exit(1);
            }
            $track['bInUse']   = $bInUse;                   // 有効になるのは最初のトラックだけ
            $track['noteName'] = $noteName;
            $track['time']     = $this->time;
            $bInUse = false;
        }
        // トラックに入りきれなかった音は警告
        if ($bInUse) {
            $this->printWarn('Over track! note[' . $noteName . ']');
        }
    }

    public function noteOff(string $noteName, int $timeDivision, bool $bIgnoreWarn): void
    {
        // 今鳴ってるのと同じノートの音を全部止めます
        $bOff = false;
        $this->checkTime($timeDivision);

        foreach ($this->tracks as $trackNr => &$track) {
            if ($track['noteName'] === $noteName) {
                $this->trackNoteOff($trackNr, $track);
                $bOff = true;
            }
        }

        // 存在しないノードがあった場合は警告
        if (($bIgnoreWarn === false) && ($bOff === false)) {
            $this->printWarn('Note[' . $noteName . '] is not turned ON');
        }
    }

    private function trackNoteOff(int $trackNr, array &$track): void
    {
        {
            $len  = $this->time - $track['time'];
            if ($len == 0) { return; }  // 長さ 0 の音はないだろうと思いますが, 念のため
            $mmlLastIndex = count($track['mmls']) - 1;
            $track['mmls'][$mmlLastIndex] .= 'SD3_' . $track['noteName'] . ', L' . $len. ', ';
            //echo('time[' . $this->time . '] ノートOFF ['. $track['noteName'] . '] len[' . $len . "]\n");
            //if ($trackNr == 0) echo('time[' . $this->time . '] ノートOFF index['. $mmlLastIndex . ']=[' . $track['mmls'][$mmlLastIndex] . "]\n");

            $track['noteName'] = '';
            $track['bInUse']   = false;
            $track['barTime'] += $len;
            $track['time']    += $len; // 終了時間を記録しておく
            $this->lenHist[$len] = 1;
        }

        // 小節単位で改行
        if ($this->barTime !== 0) {
            while ($this->barTime <= $track['barTime']) {
                $mmlLastIndex = count($track['mmls']) - 1;
                $track['barTime'] -= $this->barTime;
                $track['mmls'][$mmlLastIndex] .= '/* 小節' . $track['barNr'] ;
                if ($track['barTime'] !== 0) {
                    $track['mmls'][$mmlLastIndex] .= ' ※最後の音符が, [' . $track['barTime'] . '] だけ次の小節にはみ出してます';
                }
                $track['mmls'][$mmlLastIndex] .= ' */';
                //if ($trackNr == 0) echo('['. $track['mmls'][$mmlLastIndex] . "]\n");
                $track['mmls'][] = '';// 次の小節を追加
                $track['barNr']++;
            }
        }
    }

    public function getMml(): array
    {
        $ret = [];
        foreach ($this->tracks as $trackNr => $track) {
            $ret[$trackNr] = [];
            foreach ($track['mmls'] as $mml) {
                if ($mml === '') { break; }
                $ret[$trackNr][] = $mml;
            }
            //echo (var_export($ret[count($ret)-1], true) . "\n");
        }
        return $ret;
    }

    public function getLengthHistgram(): array
    {
        ksort($this->lenHist);
        return $this->lenHist;
    }

    /** $this->time が, $timeDivision の 1/4, 1/3 の倍数でなければエラー終了 */
    public function checkTime($timeDivision): void
    {
        if ($this->noteTime != 0) {
            $td1_4 = $timeDivision / 4;
            $td1_3 = $timeDivision / 3;
            $t = $this->time - $this->noteTime;
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
    public function makeInfoHeadStr(int $indexOffset = 0): string
    {

        $ret = sprintf('addr[0x%08x', $this->index - $indexOffset) . '] time[' . sprintf('%6d', $this->time) . ']';
        if ($this->barTime !== 0) {
            $nTime = $this->time - $this->noteTime;
            $bar = (int)($nTime / $this->barTime);
            $ret .= '(bar[' . sprintf('%2d', $bar) . '] + time[' . sprintf('%4d', $nTime - $bar * $this->barTime) . '])';
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
