<?php
/** 音楽データ 楽譜マージ ユーティリティ
 *
 * @author Snail Barbarian Macho (NWK) 2024.10.18
 */

declare(strict_types = 1);
namespace nwk\sound;

require_once(__DIR__ . '/../utils/error.class.php');
require_once 'score.class.php';
require_once 'midi.class.php';


// ---------------------------------------------------------------- マージ用のワークや音符
// MARK: MergeTrackWork
class MergeTrackWork {  // マージ処理に使うトラック毎のワークをまとめたもの
    public int    $track_nr;            // トラック番号
    public int    $ch;                  // チャンネル
    public int    $note_nr     = 0;     // 並べ替え中の現在のノート番号
    public string $curr_slide  = '';    // 現在のスライド(速度)(Lead のみ, ''=スライド無)
    public string $curr_env    = 'GS1'; // 現在のエンベロープ(速度)(Lead のみ)
    public function __construct(int $track_nr, int $ch) {
        $this->track_nr = $track_nr;
        $this->ch       = $ch;
    }
};

// MARK: MergedNote
class MergedNote {   // マージ用音符データ
    public MergeTrackWork   $r_track_work;  // トラック毎のワークを指す
    public \nwk\sound\Note  $r_note;        // 音符を指す
    public array    $strs       = [];       // 出力文字列の配列 (1つの音符が複数の命令になることがある)
    public array    $repeats    = [];       // リピート回数の配列. 添字が若い程, 外側のループ
    public int      $nr_endps   = 0;        // リピート終了の数
    public int      $len        = 0;        // 音長(変換後)
    public string   $hash       = '';       // 比較演算を高速化するためのハッシュ値

    public function __construct(MergeTrackWork &$r_track_work, \nwk\sound\Note &$r_note, int $len) {
        $this->r_track_work = $r_track_work;
        $this->r_note       = $r_note;
        $this->len          = $len;
    }

    public function calcHash(): void
    {
        $ch    = $this->r_track_work->ch;
        $scale = $this->r_note->scale;
        $tone  = $this->r_note->tone;
        $len   = $this->r_note->len;
        $str = "$ch-$scale-$tone-$len";
        foreach ($this->repeats as $repeat) {
            $str .= "-$repeat";
        }
        $nr_endps = $this->nr_endps;
        $str .= "-$nr_endps";

        $this->hash = md5($str, true);
    }

    public function equals(MergedNote &$r_merged_note): bool
    {
        if ($this->hash !== $r_merged_note->hash) {
            return false;
        }
        return
            $this->r_track_work == $r_merged_note->r_track_work &&      // 同じトラック ワークを指している
            $this->r_note->scale === $r_merged_note->r_note->scale &&
            $this->r_note->tone === $r_merged_note->r_note->tone &&
            // $this->r_note->time === $r_merged_note->r_note->time &&  // 時間は比較対象でない
            $this->r_note->len === $r_merged_note->r_note->len &&
            // $this->r_note->velo === $r_merged_note->r_note->velo &&  // 音量も比較対象でない
            $this->repeats === $r_merged_note->repeats &&
            $this->nr_endps === $r_merged_note->nr_endps;
    }
};


class ScoreMergeUtils {
    // MARK: merge()
    /** 全トラックの音符を再生順に1列にマージします
     * - この時点で,
     *   - トラック配列は0から並んでること. 各トラックのチャンネル値正しいこと
     *   - 1トラック当たりの音源数は1であること
     *   - 音符は時間順に並んでること
     * @param $bar_time       1小節の時間
     * @return array [1列にマージしたMergedNoteの配列, MergeTrackWorkの配列] のタプル
     */
    public static function merge(\nwk\sound\MidiReader $midi_reader, \nwk\sound\Score &$r_score, int $bar_time): array
    {
        //foreach ($r_score->tracks as $track_nr => &$r_track) { echo("track#$track_nr ch[$r_track->ch]: sources:[" . count($r_track->sources) . "]\n"); }// 現在のトラックや音源

        // -------- 初期化
        $midi_bar_time = $midi_reader->getBarTime();
        $merged_notes = [];
        $track_works  = [];
        foreach ($r_score->tracks as $track_nr => &$r_track) {
            $track_works[] = new \nwk\sound\MergeTrackWork($track_nr, $r_track->ch);
        }

        // -------- マージ
        while (1) {
            // 各トラックの notes のうち, note_nr 以降にある最小の time を持つ音符と, そのトラック番号を調査
            $time_min          = PHP_INT_MAX;
            $time_min_track_nr = 0;
            foreach ($r_score->tracks as $track_nr => &$r_track) {
                $r_track_work = &$track_works[$track_nr];
                $note_nr      = $r_track_work->note_nr;
                $r_notes      = &$r_track->sources[0]->notes;
                if (!isset($r_notes[$note_nr])) { continue; }// このトラックの音符は全て $merged_notes に並べました
                $r_note       = &$r_notes[$note_nr];
                if ($time_min > $r_note->time) {
                    $time_min = $r_note->time;
                    $time_min_track_nr = $track_nr;
                }
                //if ($time_min < 100) { echo("トラック[$track_nr] [$note_nr]番目の音符[$r_note->data_scale] 時間[$r_note->time]\n"); }
            }
            if ($time_min === PHP_INT_MAX) { break; } // 全てのトラックの音符を $marged_notes に並べました

            $r_track_work = &$track_works[$time_min_track_nr];
            $note_nr      = $r_track_work->note_nr;
            $r_note       = &$r_score->tracks[$time_min_track_nr]->sources[0]->notes[$note_nr];
            $ch           = $r_score->tracks[$time_min_track_nr]->ch;

            //if ($time_min < 100) { echo("時間[$time_min] トラック[$time_min_track_nr](ch$ch)の $note_nr 番目の音符[$r_note->data_scale]を追加\n"); }
            $merged_notes[] = new \nwk\sound\MergedNote($r_track_work, $r_note, $r_note->len * $bar_time / $midi_bar_time);
            $r_track_work->note_nr++;
        }

        return [$merged_notes, $track_works];
    }

    /** リピート圧縮します
     * @param $r_merged_notes [in][out]マージした MergedNote の配列
     * @param $max_repeat_times 最大リピート回数
     * @param $max_repeat_level 最大リピート レベル
     * @return array [リピートの最大反復回数, リピートの最大ネスト] のタプル
     */
    public static function compressRepeat(array &$r_merged_notes, int $top_ch, int $max_repeat_times, int $max_repeat_level, \nwk\utils\Error $error): array
    {
        if (count($r_merged_notes) === 0) {
            $error->error("データがありません");
            return [0, 0];
        }

        // -------- 「一番最初の音符のチャンネル(top_ch)と同じ音符(top_note)」がいくつあるかを調査
        $ch = $r_merged_notes[0]->r_track_work->ch;
        if ($top_ch !== $ch) {
            $error->error("データの最初の音符が $top_ch ではありません");
            return [0, 0];
        }
        $nr_top_notes = 0;
        foreach ($r_merged_notes as &$r_note) {
            if ($r_note->r_track_work->ch == $top_ch) { $nr_top_notes++; }
            $r_note->calcHash();
        }
        //echo("top_ch:$top_ch top_notes:$nr_top_notes merged_notes:" . count($r_merged_notes) . "\n");

        // -「"A"(=top_note) で始り, 以降が "A" 以外の文字列」を「単語(Word)」と呼ぶことにします
        //   複数のワードをまとめて「連語(Collocation)」と呼ぶことにします
        //   たとえば, "ABCAB" は 2 単語連語
        // - "ABCABABCABABCD" と並んでる場合,
        //   最初の 2 単語連語 "ABCAB" を認識し,
        //   この 2 単語連語が次の文字以降何回連続するかを検索
        //   連続しないなら次の 2 単語連語 "ABABC" を認識

        for ($w = 1; $w < (int)($nr_top_notes / 2); $w++) { // 連語における単語数
        //for ($w = 2; $w <= 3; $w++) { // 連語における単語数
            $mode = 0;
            $it   = 0;
            $rep  = 0;
            $collo_start  = 0; // 連語開始位置
            $nr_words     = 0; // 連語の単語数
            $nr_notes     = 0; // 連語の音符数
            $next         = 0; // 次の連語捜査開始位置
            $nr_merged_notes = count($r_merged_notes);

            for ($i = 0; $i < $nr_merged_notes; $i++) {  // 音符インデックス
            //for ($i = 0; $i < 100; $i++) {  // 音符インデックス
                switch ($mode) {
                    case 0: // top_note を探す
                        $ch = $r_merged_notes[$i]->r_track_work->ch;
                        //echo("mode0 [$i]=ch$ch");
                        if ($ch === $top_ch) {
                            //echo(" top_note 検出");
                            $mode = 1;
                            $collo_start  = $i;
                            $nr_words     = 0;
                            $nr_notes     = 0;
                        }
                        //echo("\n");
                        break;
                    case 1: // 連語の最後の位置を検出
                        $ch = $r_merged_notes[$i]->r_track_work->ch;
                        //echo("mode1 [$i]=ch$ch 語:$nr_words");
                        $nr_notes++;
                        if ($ch === $top_ch) {
                            $nr_words++;
                            if ($nr_words === $w) {
                                $mode = 2;
                                $it   = 0;
                                $rep  = 1;
                                $next = $collo_start;   // 次回の検索開始位置は top_note の次
                                $i--;
                                //echo("  $w" . "語連語検出 音符数:[$collo_start" ."～$i]=" . $i+1-$collo_start . " next:$next");
                            }
                        }
                        //echo("\n");
                        break;
                    case 2: // 連語内の単語比較
                        if ($r_merged_notes[$collo_start + $it]->equals($r_merged_notes[$i])) {
                            //echo("mode2[$i] $it"."個めの音符を比較中/$nr_notes");
                            $it++;
                            if ($it == $nr_notes) {     // 1連語一致したので削除
                                for ($j = 0; $j < $nr_notes; $j++) {
                                    unset($r_merged_notes[$i - $j]);
                                }
                                $next = $i;             // 次回の検索開始位置は削除した連語の次
                                $rep++;
                                //echo("--連語一致($rep" . "個め) 重複する音符は削除:[" . $i-$nr_notes+1 . "～$i] next:$next");
                                $it = 0;
                            }
                        } else { // 一致しなければそこまで
                            if (2 <= $rep) {
                                array_unshift($r_merged_notes[$collo_start]->repeats, $rep);
                                $r_merged_notes[$collo_start + $nr_notes - 1]->nr_endps++;
                                //echo($r_merged_notes[$collo_start]->r_note->genInfo(1920)."から $w" . "連語($nr_notes" . "音符) x $rep" . " 反復分を圧縮\n");
                            }
                            //if ($w == 10 && 6 <= $it) { echo($r_merged_notes[$collo_start]->r_note->genInfo(1920)."から $w" . "連語($nr_notes" . "音符) $rep" . "復目" . " $it 音符目で不一致 ch:" . $r_merged_notes[$i]->r_track_work->ch . " " . $r_merged_notes[$collo_start + $it]->r_note->scale . " != " . $r_merged_notes[$i]->r_note->scale . "\n"); }
                            //echo("mode2終了 [$i] rep:$rep 次の走査は:$next+1から");
                            $i = $next;
                            $mode = 0;
                        }
                        //echo("\n");
                        break;
                } // switch($mode)
            } // for ($i)
            $r_merged_notes = array_values($r_merged_notes);
            if ($mode == 2 && 2 <= $rep) {
                $r_merged_notes[$collo_start]->repeats[] = $rep;
                $r_merged_notes[$collo_start + $nr_notes - 1]->nr_endps++;
                //echo("末端:".$r_merged_notes[$collo_start]->r_note->genInfo(1920)."から $w" . "連語($nr_notes" . "音符) x $rep" . " 反復分を圧縮\n");
            }
        } // for ($w)
        //echo("圧縮後のmerged_notes:" . count($r_merged_notes) . "\n");

        // 最大レベルと反復回数をチェック
        $repeat_times = 0;
        $repeat_level = 0;
        $lev = 0;
        foreach($r_merged_notes as &$r_notes) {
            foreach ($r_notes->repeats as $repeat) {
                $repeat_times = max($repeat_times, $repeat);
            }
            $lev += count($r_notes->repeats);
            $repeat_level = max($lev, $repeat_level);
            $lev -= $r_notes->nr_endps;
        }
        if ($lev !== 0) {
            $error->error("コンバータのバグ:リピートが閉じてません: $lev");
        }
        if ($max_repeat_times < $repeat_times) {
            $error->error("リピート回数が多すぎます: $max_repeat_times < $repeat_times");
        }
        if ($max_repeat_level < $repeat_level) {
            $error->error("リピート レベルが大きすぎます: $max_repeat_level < $repeat_level");
        }

        return [$repeat_times, $repeat_level];
    }


    // MARK: genHistgramString()
    /** ヒストグラム文字列作成
     * @param $r_merged_notes マージしたMergedNoteの配列
     * @param $track_tab サポートするトラックの形式テーブル. ScoreUtils::checkTrackTabs() を参照してください
     */
    public static function genHistgramString(array &$r_merged_notes, array $track_tab): string
    {
        $hist_ch_scale = [];// 各チャンネルの音階ヒストグラム
        $hist_len      = [];// 音長ヒストグラム

        // -------- 集計
        foreach ($r_merged_notes as &$r_note) {
            // 音長ヒストグラム
            $len = $r_note->len;
            if (!isset($hist_len[$len])) { $hist_len[$len] = 0; }
            $hist_len[$len]++;

            // 各チャンネルの音階ヒストグラム
            // ":" で区切られた音階は分解し, 音源数だけ採用する
            $ch = $r_note->r_track_work->ch;
            if (!isset($hist_ch_scale[$ch])) { $hist_ch_scale[$ch] = []; }

            $scales = explode(':', $r_note->r_note->data_scale);
            $nr_sources = min(count($scales), $track_tab[$ch]['nr_sources']);
            for ($i = 0; $i < $nr_sources; $i++) {
                $scale = $scales[$i];
                if (!isset($hist_ch_scale[$ch][$scale])) { $hist_ch_scale[$ch][$scale] = 0; }
                $hist_ch_scale[$ch][$scale]++;
            }
        }
        // print_r($hist_ch_scale);

        // -------- 文字列化
        $str = '';
        $sum2 = 0;
        foreach ($hist_ch_scale as $ch => &$r_hist_scale) {
            ksort($r_hist_scale);
            $sum = array_sum($r_hist_scale);
            $name = $track_tab[$ch]['name'];
            $str .= "// $name 音階ヒストグラム(計$sum):\n";
            foreach($r_hist_scale as $scale => $val) {
                $str .= "//   " . str_pad($scale, 4, ' ') . "x $val\n";
            }
            $sum2 += $sum;
        }
        $str .= "// 音符 計$sum2\n";

        ksort($hist_len);
        $sum = array_sum($hist_len);
        $str .= "// 音長ヒストグラム(計$sum):\n";
        foreach($hist_len as $len => $val) {
            $str .= "//   $len x $val\n";
        }

        //echo("$str\n");
        return $str;
    }


    // MARK: genInfoString()
    /** その他情報文字列の作成 */
    public static function genInfoString(int $max_repeat_times, int $max_repeat_level, int $bytes): string
    {
        $str  = "// 最大リピート回数: $max_repeat_times\n";
        $str .= "// 最大リピート レベル: $max_repeat_level\n";
        $str .= "// $bytes bytes\n\n";
        return $str;
    }


    // MARK: genFormattedSourceString()
    /** 整形済ソース文字列作成
     * @param $r_merged_notes マージした MergedNote の配列
     * @param $track_tab サポートするトラックの形式テーブル. ScoreUtils::checkTrackTabs() を参照してください
     */
    public static function genFormattedSourceString(
        string $var_name,
        \nwk\sound\MidiReader $midi_reader, array &$r_merged_notes, int $bar_time,
        array $track_tab, int $nr): string
    {
        $max_str_lens = []; // キーは, チャンネル
        $spaces       = []; // キーは, チャンネル

        // --------(1) 各チャンネルの文字列を調べて, 最大文字長を記録します
        foreach($r_merged_notes as &$r_merged_note) {
            $ch = $r_merged_note->r_track_work->ch;
            foreach($r_merged_note->strs as $str) {
                if (!isset($max_str_lens[$ch])) { $max_str_lens[$ch] = 0; }
                $max_str_lens[$ch] = max($max_str_lens[$ch], strlen($str));
            }
        }

        // --------(2) 出力文字列に ', ' を追加し, 最大文字長に揃えます
        foreach ($max_str_lens as &$r_max_str_len) {
            $r_max_str_len += 2;
        }
        foreach($r_merged_notes as &$r_merged_note) {
            $ch = $r_merged_note->r_track_work->ch;
            foreach($r_merged_note->strs as &$r_str) {
                $r_str = str_pad($r_str . ', ', $max_str_lens[$ch], ' ');
            }
        }

        // --------(3) 空白文字の作成と, 配列先頭部の出力
        $out = "//----------------------------------------------------------------♩\n//  ";
        foreach ($max_str_lens as $ch => $max_str_len) {
            $spaces[$ch] = str_pad('', $max_str_len, ' ');
            $out .= str_pad($track_tab[$ch]['name'], $max_str_len, ' ');
        }
        $out .= "\nstatic u8 const $var_name" . $nr. "[] = {";

        // -------- (5) 配列に適時改行を入れて結合.
        //foreach($spaces as $key => $space) { echo($key . "=>" . strlen($space). "\n"); } 空白文字数を調べる
        $curr_bar = -1;
        reset($spaces);
        $curr_ch = key($spaces);
        $midi_bar_time = $midi_reader->getBarTime();

        foreach($r_merged_notes as &$r_merged_note) {
            $ch  = $r_merged_note->r_track_work->ch;

            // 小節が切り替わったら, 改行
            $bar = (int)($r_merged_note->r_note->time / $midi_bar_time);
            if ($curr_bar !== $bar) {
                $curr_bar = $bar;
                // 小節番号は, BAR_OFFSET からではありません
                $out .= "\n    // -------- 小節" . ($bar + \nwk\sound\MidiReader::BAR_OFFSET) . "(t=" . ($bar_time * $bar) . ")";
                end($spaces);
                $curr_ch = key($spaces);
            }

            foreach($r_merged_note->strs as $str) {
                $space = '';
                do {
                    if (next($spaces) === false) {
                        $space .= "\n    "; // 改行とインデント
                        reset($spaces);
                    }
                    $curr_ch = key($spaces);

                    if ($r_merged_note->r_track_work->ch !== $curr_ch) {// このチャンネルの音符は無いのでスペースでスキップ
                        $space .= $spaces[$curr_ch];
                    }

                } while ($ch !== $curr_ch);
                $out .= $space . $str;
            }
        }

        return $out;
    }


}