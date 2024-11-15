<?php
/** エラーを表示して, エラー数を返す. それだけのクラス
 *
 * @author Snail Barbarian Macho (NWK) 2024.06.06
 */

declare(strict_types = 1);
namespace nwk\utils;

require_once(__DIR__ . '/../sound/score.class.php');


// MARK: Error
Class Error
{
    private string $filename_ = '';
    private int    $bar_time_ = 0;
    private int    $errors_ = 0;
    private int    $warns_ = 0;

    // -------------------------------- ファイル名, 小節の長さ, 警告数, エラー数
    // MARK: setFileName()
    public function setFilename(string $filename): void
    {
        $this->filename_ = $filename;
    }


    // MARK: setBarTime()
    public function setBarTime(int $bar_time): void
    {
        $this->bar_time_ = $bar_time;
    }


    // MARK: setNrWarns()
    public function getNrWarns(): int
    {
        return $this->warns_;
    }


    // MARK: getNrErrors()
    public function getNrErrors(): int
    {
        return $this->errors_;
    }


    // MARK: report()
    /** エラーレポートして, 必要に応じて exit(1) します */
    public function report(): void
    {
        if ($this->errors_ !== 0) {
            fwrite(STDERR, "$this->errors_ error(s), $this->warns_ warns(s). aborted.\n");
            exit(1);
        }
        if ($this->warns_ !== 0) {
            fwrite(STDERR, "$this->warns_ warns(s).\n");
        }
    }


    // -------------------------------- 一般用
    // MARK: warn()
    /** 警告を表示します
     * @param $msg     エラーメッセージ
     * @param $src     エラーとなったソース(無ければ'')
     */
    public function warn(string $msg, string $src = ''): void
    {
        $this->errorLine_(-1, $msg, $src, 'WARN:');
        $this->warns_++;
    }


    // MARK: error()
    /** エラー表示します. 引数は warn() 参照 */
    public function error(string $msg, string $src = ''): void
    {
        $this->errorLine_(-1, $msg, $src, '');
        $this->errors_++;
    }


    // -------------------------------- 行番号付き
    // MARK: warnLine()
    /** 警告を表示します(行番号付)
     * @param $line_nr 行番号(無ければ-1)
     * @param $msg     エラーメッセージ
     * @param $src     エラーとなったソース(無ければ'')
     */
    public function warnLine(int $line_nr, string $msg, string $src = ''): void
    {
        $this->errorLine_($line_nr, $msg, $src, 'WARN:');
        $this->warns_++;
    }


    // MARK: errorLine()
    /** エラー表示します(行番号付). 引数は warnLine() 参照 */
    public function errorLine(int $line_nr, string $msg, string $src = ''): void
    {
        $this->errorLine_($line_nr, $msg, $src, '');
        $this->errors_++;
    }


    // MARK: errorLine_()
    private function errorLine_(int $line_nr, string $msg, string $src, string $prefix): void
    {
        $e = $this->errors_ + $this->warns_;
        if ($e == 20) {
            fwrite(STDERR, "...(Too many errors or warnings)\n");
            return;
        }
        if (20 < $e) {
            return;
        }

        // ソースコードの一部表示
        $str = str_replace("\n", '\n', $src);
        if (50 < strlen($str)) {
            $str = substr($str, 0, 50) . '...';
        }

        $out = '';
        if ($this->filename_ !== '') { $out .= $this->filename_ . ':'; }
        if ($line_nr !== -1)         { $out .= ($line_nr + 1) . ':'; }
        $out .= " $prefix$msg";
        if ($src !== '')             { $out .= ":[$str]"; }
        fwrite(STDERR, "$out\n");
    }


    // -------------------------------- 楽譜用
    // MARK: warnScore()
    /** 警告表示します(楽譜用)
     * @param $ch           チャンネル番号(無ければ-1)
     * @param $source_nr    音源番号(無ければ-1)
     * @param $note         音符名(無ければ空文字)
     * @param $time         時間(音符が無ければ表示しない)
     * @param $msg          エラーメッセージ
     */
    public function warnScore(int $ch, int $source_nr, ?\nwk\sound\Note $note, string $msg): void
    {
        $this->errorScore_($ch, $source_nr, $note, $msg, 'WARN:');
        $this->warns_++;
    }


    // MARK: errorScore()
    /** エラー表示します(楽譜用). 引数は, warnScore() 参照 */
    public function errorScore(int $ch, int $source_nr, ?\nwk\sound\Note $note, string $msg): void
    {
        $this->errorScore_($ch, $source_nr, $note, $msg, '');
        $this->errors_++;
    }


    // MARK: errorScore_()
    private function errorScore_(int $ch, int $source_nr, ?\nwk\sound\Note $note, string $msg, string $prefix): void
    {
        $e = $this->errors_ + $this->warns_;
        if ($e == 20) {
            fwrite(STDERR, "...(Too many errors or warnings)\n");
            return;
        }
        if (20 < $e) {
            return;
        }

        $out = '';
        $b   = false;

        if ($this->filename_ !== '') { $out .= $this->filename_ . ':'; }
        if ($ch              !== -1) { $out .= " ch[" . ($ch + 1) . "]";              $b = true; }
        if ($source_nr       !== -1) { $out .= " 音源[$source_nr]";     $b = true; }
        if ($note            !== null) {
            $out .= $note->genInfo($this->bar_time_);
            $b = true;
        }
        if ($b === true) { $out .= ':'; }
        $out .= " $prefix$msg";
        fwrite(STDERR, "$out\n");
    }
}