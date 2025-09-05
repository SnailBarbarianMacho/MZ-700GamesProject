<?php
/** Wav ファイル関係クラス
 *
 * @author Snail Barbarian Macho (NWK) 2025.06.16
 */

declare(strict_types = 1);
namespace nwk\sound;
//require_once(__DIR__ . '/../utils/error.class.php');


// -------------------------------- Wav
// MARK: Wav
/**
 * Wav データを読み書き加工するクラス
 * - リニアPCMのみ
 * - 今のところ 1チャンネルのみ
 * - 今のところ 8/16bit のみ
 */
class Wav {
    const MAX_SAMPLES = 65536 * 256 * 2;
    const ERRMSG_MAX_SAMPLES = 'サンプル数の上限は' . self::MAX_SAMPLES . ' 個に制限してます';

    private string  $data_str_;
    private int     $channels_;
    private int     $sampling_freq_;
    private int     $sample_bits_;
    private string  $err_msg_ = '';

    // ---------------------------------------------------------------- コンストラクタ/クローン
    // MARK: __construct()
    /** コンストラクタ */
    public function __construct(
        int $channels = 1, int $sampling_freq = 44100, int $sample_bits = 16, string $data_str = '')
    {
        $this->channels_      = $channels;
        $this->sampling_freq_ = $sampling_freq;
        $this->sample_bits_   = $sample_bits;

        if (!$this->isFormatSupported($channels, $sampling_freq, $sample_bits) ||
            !$this->checkNrSamples($data_str)) {
            throw new \Exception($this->err_msg_);
        }

        $this->data_str_ = $data_str;
    }


    // MARK: __clone()
    /** クローン */
    public function __clone()
    {
        if (!$this->isFormatSupported($this->channels_, $this->sampling_freq_, $this->sample_bits_) ||
            !$this->checkNrSamples($this->data_str_)) {
            throw new \Exception($this->err_msg_);
        }
    }


    // ---------------------------------------------------------------- wav データのロード/セーブ/インポート/エクスポート
    // MARK: load()
    /** wav ファイルをロードします */
    public function load(string $filename): bool
    {
        $wav_str = file_get_contents($filename);
        if ($wav_str === false) {
            $this->err_msg_ = "$filename: ロードに失敗しました";
            return false;
        }
        return $this->import($wav_str);
    }


    // MARK: save()
    /** wav ファイルにセーブします */
    public function save(string $filename): bool
    {
        $wav_str = $this->export();
        $result = file_put_contents($filename, $wav_str);
        if ($result === false) {
            $this->err_msg_ = "$filename: セーブに失敗しました";
            return false;
        }
        return true;
    }


    // MARK: import()
    /** Wav 形式バイナリ データを取り込みます
     * @param $wav_str Wav 形式バイナリ文字列. file_get_contents() の戻値がそのまま使えます
     * @return true/false = 成功/失敗(クラスの中身は変化しません)
     */
    public function import(string $wav_str): bool
    {
        if (substr($wav_str, 0, 4) !== 'RIFF') {
            $this->err_msg_ = 'WAV ファイルではありません';
            return false;
        }

        if (substr($wav_str, 8, 4) !== 'WAVE') {
            $this->err_msg_ = 'WAVE チャンクがありません';
            return false;
        }

        if (substr($wav_str, 12, 4) !== 'fmt ') {
            $this->err_msg_ = 'fmt チャンクがありません';
            return false;
        }

        $d = unpack('v', substr($wav_str, 20, 2));
        if ($d[1] !== 0x001) {
            $this->err_msg_ = "リニア PCM フォーマットのみサポートしてます:$d[1]";
            return false;
        }

        $d = unpack('v', substr($wav_str, 22, 2));
        $channels = 0;
        if (!$this->isChannelsSupported($d[1])) {
            return false;
        }
        $channels = $d[1];

        $sampling_freq = 0;
        $d = unpack('v', substr($wav_str, 24, 4));
        if (!$this->isSamplingFrequencySupported($d[1])) {
            return false;
        }
        $sampling_freq = $d[1];

        $sample_bits = 0;
        $d = unpack('v', substr($wav_str, 34, 2));
        if (!$this->isSampleBitsSupported($d[1])) {
            return false;
        }
        $sample_bits = $d[1];

        if (substr($wav_str, 36, 4) !== 'data') {
            $this->err_msg_ = 'data チャンクがありません';
            return false;
        }

        $data_size = unpack('V', substr($wav_str, 40, 4))[1];
        $samples = $data_size / ($sample_bits / 8 * $channels);
        if (self::MAX_SAMPLES < $samples) {
            $this->err_msg_ = self::ERRMSG_MAX_SAMPLES . ":$size";
            return false;
        }

        $this->data_str_ = substr($wav_str, 44, $data_size);
        $data_len = strlen($this->data_str_);
        if ($data_len !== $data_size) {
            $this->err_msg_ = "WAV データ サイズ($data_len) が期待してた値($data_size)と異なります";
            return false;
        }

        $this->channels_      = $channels;
        $this->sampling_freq_ = $sampling_freq;
        $this->sample_bits_   = $sample_bits;

        return true;
    }


    // MARK: export()
    /** Wav 形式バイナリ データを出力します
     * @return string file_put_contents() にそのまま使えます
     */
    public function export(): string
    {
        $block_size     = $this->channels_ * $this->sample_bits_ / 8;
        $bytes_per_secs = $block_size * $this->sampling_freq_;
        $fmt = 1; // linear PCM

        $fmt_chunk = 'WAVEfmt ';
        $fmt_chunk .= pack("V", 16); // fmt chunk length
        $fmt_chunk .= pack("v", $fmt);
        $fmt_chunk .= pack("vVV", $this->channels_, $this->sampling_freq_, $bytes_per_secs);
        $fmt_chunk .= pack("vv", $block_size, $this->sample_bits_);

        $data_chunk = 'data' . pack('V', strlen($this->data_str_)) . $this->data_str_;
        $riff_len   = strlen($fmt_chunk) + strlen($data_chunk);

        return 'RIFF'.pack("V", $riff_len).$fmt_chunk.$data_chunk;
    }


    // ---------------------------------------------------------------- プロパティ
    // -------------------------------- チャンネル・サンプリング周波数・サンプル ビット
    // MARK: getChannels()
    /** チャンネル数を返します */
    public function getChannels(): int { return $this->channels_; }


    // MARK: getSamplingFrequency()
    /** サンプリング周波数を返します */
    public function getSamplingFrequency(): int { return $this->sampling_freq_; }


    // MARK: getSampleBits()
    /** 1 チャンネル 1 サンプル当たりのビット数を返します */
    public function getSampleBits(): int { return $this->sample_bits_; }


    // MARK: setChannels()
    /** チャンネル数を設定します */
    public function setChannels($channels): bool
    {
        if (!$this->isFormatSupported($channels, $this->sampling_freq_, $this->sample_bits_)) {
            return false;
        }
        $this->channels_ = $channels;
        return true;
    }


    // MARK: setSamplingFrequency()
    /** サンプリング周波数を設定します */
    public function setSamplingFrequency(int $sampling_freq): bool
    {
        if (!$this->isFormatSupported($this->channels_, $sampling_freq, $this->sample_bits_)) {
            return false;
        }
        $this->sampling_freq_ = $sampling_freq;
        return true;
    }


    // MARK: setSampleBits()
    /** 1 チャンネル 1 サンプル当たりのビット数を設定します */
    public function setSampleBits(int $sample_bits): bool
    {
        if (!$this->isFormatSupported($this->channels_, $this->sampling_freq_, $sample_bits)) {
            return false;
        }
        $this->sample_bits_ = $sample_bits;
        return true;
    }


    // -------------------------------- データ・サンプル数
    // MARK: getData()
    /**
     * データを返します
     * @returns データ配列のコピーとなるバイナリ文字列
     * - @see getSampleBits() === 16 ならば, s16, 8 ならば u8
     * - @see getChannels() === 2 ならば, データの並びは ch0,ch1,ch0,ch1... となります.
     *   なのでデータの大きさはチャンネルの倍数になります
     */
    public function getData(): string { return $this->data_str_; }


    // MARK: setData()
    /**
     * 生データをコピーして Wav にセットします
     * @param r_data_str データ バイナリ文字列
     * @returns 成功したら $this. 配列の大きさがおかしいなら null
     */
    public function setData(string &$r_data_str): ?\nwk\sound\Wav
    {
        if ($this->checkNrSamples($r_data_str) === false) {
            return null;
        };

        $this->data_str_ = $r_data_str;
        return $this;
    }


    // MARK: getNrSamples()
    /**
     * サンプリング数を返します.
     * @returns $see getNrChannels() の倍数になります
     */
    public function getNrSamples(): int
    {
        $sample_bytes = $this->sample_bits_ / 8;
        return strlen($this->data_str_) / $sample_bytes;
    }


    // MARK: checkNrSamples()
    /** 指定データのサンプル数が正しいかチェックします.
     * - エラー時はエラー メッセージを残します
     */
    public function checkNrSamples(string &$r_data_str): bool
    {
        $sample_bytes = $this->sample_bits_ / 8;
        $data_len = strlen($r_data_str);
        $samples = $data_len / $sample_bytes;

        if ($samples * $sample_bytes !== $data_len) {
            $this->err_msg_ = "データの大きさ($data_len)が sample_bits の倍数ではありません";
        }

        if ($this->channels_ == 0) {
            $this->err_msg_ = "チャンネル数の異常:$this->channels_";
            return false;
        }

        if ($samples % $this->channels_ !== 0) {
            $this->err_msg_ = "データのサンプル数はチャンネル数($this->channels_)の倍数でなければなりません:$samples";
            return false;
        }

        if (self::MAX_SAMPLES < $samples) {
            $this->err_msg_ = self::ERRMSG_MAX_SAMPLES . ":$samples";
            return false;
        }

        return true;
    }


    // -------------------------------- エラー, 情報
    // MARK: getLastErrorMsg()
    /** 最後のエラー メッセ―ジを返します. 改行は入ってません
     * - 一度呼び出すとエラー メッセージは空文字列になります
     */
    public function getLastErrorMsg(): string
    {
        $msg = $this->err_msg_;
        $this->err_msg_ = '';
        return $msg;
    }


    // MARK: getInfoString()
    /** このデータの情報を返します */
    public function getInfoString(): string
    {
        $sec = (float)$this->getNrSamples() / ($this->channels_ * $this->sampling_freq_);
        return $this->getNrSamples() . 'samples, ' .
            $this->channels_ . 'ch, ' .
            $this->sampling_freq_ . 'Hz, ' .
            $this->sample_bits_ . 'bits, ' .
            sprintf('%.3f', $sec) . 'sec';
    }

    // ---------------------------------------------------------------- 編集
    // MARK: append()
    /**
     * Wav を連結して, 新しい Wav を返します. ギャップを入れることができます
     * @return 同一チャンネル・サンプリング周波数・サンプル ビットでなければ null
     */
    public function append(\nwk\sound\Wav $wav, int $gap_samples = 0): ?\nwk\sound\Wav
    {
        // チェック
        if ($this->channels_      !== $wav->getChannels() ||
            $this->sampling_freq_ !== $wav->getSamplingFrequency() ||
            $this->sample_bits_   !== $wav->getSampleBits()) {
            $this->err_msg_ = "フォーマットが一致しません";
            return null;
        }

        $gap_str = str_pad('', $gap_samples * $this->channels_ * $this->sample_bits_ / 8, "\x00", STR_PAD_LEFT);
        $data_str = $this->data_str_ . $gap_str . $wav->getData();
        //echo(strlen($this->data_str_) . "+=" . strlen($gap_str) . "+" . strlen($data_str) . "\n");

        return new \nwk\sound\Wav($this->channels_, $this->sampling_freq_, $this->sample_bits_, $data_str);
    }


    // MARK: replace()
    /** データの一部を, 別データに置換した新しい Wav を返します. 元 Wav は変更されません
     * - substr_replace() 関数とよく似た挙動をします
     * @param $sample_offset 削除/置換対象となる位置
     * - 単位は1チャンネルならサンプル単位. 2チャンネルならサンプルペア単位
     * - 負の場合, データ配列末端からの位置になります
     * @param $samples 削除される大きさ
     * - 単位は1チャンネルならサンプル単位. 2チャンネルならサンプルペア単位
     * - 省略時は 末端までを意味します
     * - 負だと, $sample_offset - |$samples| 位置からになります
     * @param $replace_wav 置換される Wav データ. null なら置換しません
     * @return $replace_wav が同一チャンネル・サンプリング周波数・サンプル ビットでなければ null
     */
    public function replace(?\nwk\sound\Wav $replace_wav, int $sample_offset, int $samples = undefined): ?\nwk\sound\Wav
    {
        $data_str = '';
        if ($replace_wav !== null) {
            if ($this->channels_      !== $replace_wav->getChannels() ||
                $this->sampling_freq_ !== $replace_wav->getSamplingFrequency() ||
                $this->sample_bits_   !== $replace_wav->getSampleBits()) {
                $this->err_msg_ = "フォーマットが一致しません";
                return null;
            }
            $replace_str = $replace_wav->getData();
        }

        $sample_bytes = $this->sample_bits_ / 8;
        $ch           = $this->channels_;
        $unit         = $sample_bytes * $ch;

        $start = $sample_offset * $unit;
        if (defined($samples)) {
            $len = $samples     * $unit;
        }

        $data_str = substr_replace($this->data_str, $replace_str, $start, $len);
        return new \nwk\sound\Wav($this->channels_, $this->sampling_freq_, $this->sample_bits_, $data_str);
    }


    // MARK: fill()
    /** データの特定領域を値で埋めた, 新しい Wav を作成します. 元 Wav は変更されません
     * - 配列は ksort() され, array_value() で並べ直されるので, 添字は 0 からになります
     * @param $sample_offset fill したい開始サンプル位置
     * - 単位は1チャンネルならサンプル単位. 2チャンネルならサンプルペア単位
     * - 負の場合, データ配列の前に fill された値が挿入されます (array_fill() に似た挙動)
     * @param $samples       fill したい大きさ
     * - 単位は1チャンネルならサンプル単位. 2チャンネルならサンプルペア単位
     * - 負の場合, 埋める方向が逆になります (array_fill() には無い機能)
     * @return fill する場所がデータから離れてる場合は null
     * @example
     *   $wav2 = $wav->fill(-1, -10000);                  // $wav の前に, 10000 サンプルの無音を挿入した $Wav2 を生成します
     *   $wav3 = $wav->fill($wav->getNrSamples(), 10000); // $wav の後に, 10000 サンプルの無音を挿入しま $Wav3 を生成します
     */
    /* 2025.06.15 データが配列からバイナリ文字列になったので未完成
    public function fill(int $sample_offset, int $samples, int $value = 0x0000): ?\nwk\sound\Wav
    {
        if ($this->channels_ !== 1 || $this->sample_bits_ !== 16) {
            // YET 未実装
            $this->err_msg_ = "YET このフォーマットは非サポート";
            return null;
        }

        $data_samples = $this->getNrSamples();
        $data_str = $this->data_str_;

        if (0 < $samples) {
            if ($sample_offset + $samples < -1 || $data_samples < $sample_offset) {
                $this->err_msg_ = "fill の範囲[$sample_offset # $samples]がデータ範囲[0, $data_samples]から離れてます";
                return null;
            }
            $ch  = $this->channels_;
            $idx = $sample_offset * $ch;
            for ($i = $samples * $ch; $i > 0; $i--, $idx++) {
                $data_str[$idx] = $value;
            }
        } else {
            if ($sample_offset < -1 || $data_samples < $sample_offset + $samples) {
                $this->err_msg_ = "fill の範囲[$sample_offset # $samples]がデータ範囲[0, $data_samples]から離れてます";
                return null;
            }
            $ch  = $this->channels_;
            $idx = $sample_offset * $ch;
            for ($i = $samples * $ch; $i < 0; $i++, $idx--) {
                $data_str[$idx] = $value;
            }
        }
        ksort($data_str);
        $data_str = array_values($data_str);
        //foreach($data_str as $key => $val) { echo("$key=>$val "); } echo("\n");
        return new \nwk\sound\Wav($this->channels_, $this->sampling_freq_, $this->sample_bits_, $data_str);
    }*/


    // ---------------------------------------------------------------- データ加工
    // MARK: adjustVolume()
    /** 音量調整します. 1.0 = 等倍(何もしません) */
    public function adjustVolume(float $vol): void
    {
        if ($vol === 1.0) { return; }

        $str = ' ';
        if ($this->sample_bits_ == 8) {
            for ($i = strlen($this->data_str_) - 1; 0 <= $i; --$i) {
                $str = $this->data_str_[$i];
                $val = (int)((unpack('C', $str)[1] - 0x80) * $vol + 0x80);
                $val = max(0, $val);
                $val = min(255, $val);
                $this->data_str_[$i] = pack("C", $val);
            }
        } else {
            for ($i = strlen($this->data_str_) - 2; 0 <= $i; $i -= 2) {
                $str = substr($this->data_str_, $i, 2);
                $val = unpack('v', $str)[1];                //  0x8000～0xffff,  0x0000～0x7fff
                if (0x8000 <= $val) { $val -= 0x10000; }     // -0x8000～-0x0001, 0x0000～0x7fff
                $val = (int)($val * $vol);
                $val = max(-0x8000, $val);
                $val = min(0x7fff,  $val);
                if ($val < 0) { $val += 0x10000; }
                $str = pack("v", $val);
                $this->data_str_[$i    ] = $str[0];
                $this->data_str_[$i + 1] = $str[1];
            }
        }
    }


    // MARK: autoMaximize()
    /** 自動最大音量処理を施した, 新しい Wav を返します
     * @param scale 自動最大音量処理の後に掛ける値
     * @return エラー時は null
     */
    /* 2025.06.15 データが配列からバイナリ文字列になったので未完成
    public function autoMaximize(float $scale = 1.0): ?\nwk\sound\Wav
    {
        $data_str = $this->data_str_;

        if ($this->autoMaximize_($data_str, $scale) === false) {
            return null;
        };
        return new \nwk\sound\Wav($this->channels_, $this->sampling_freq_, $this->sample_bits_, $data_str);
    }*/


    // MARK: autoMaximize_()
    /** 自動最大音量処理 */
    /* 2025.06.15 データが配列からバイナリ文字列になったので未完成
    private function autoMaximize_(string &$r_data_str, float $scale): bool
    {
        if ($this->channels_ !== 1) {
            // YET 未実装
            $this->err_msg_ = "YET 1チャンネルしかサポートしてません:$this->channels_";
            return false;
        }
        if ($this->sample_bits_ !== 16) {
            // YET 未実装
            $this->err_msg_ = "YET 16bit しかサポートしてません:$this->channels_";
            return false;
        }

        $min = 1 << 16;
        $max = -$min;
        foreach($r_data_str as $val) {
            $min = min($val, $min);
            $max = max($val, $max);
        }
        $max = max(abs($min), abs($max));
        if ($max !== 0) {
            $ratio = (float)((1 << 16) / 2- 1) / $max * $scale;
            //echo("max:$max ratio:$ratio $r_data_str[10]\n");
            foreach($r_data_str as &$r_val) {
                $val = (int)((float)$r_val * $ratio);
                $val = min($val,   (1 << 16) / 2 - 1);
                $val = max($val, - (1 << 16) / 2);
                $r_val = $val;
            }
            //echo("$r_data_str[10]\n");
        }

        return true;
    }*/


    // MARK: degrade()
    /** 音声データを劣化させた, 新しい Wav を返します
     * - Wav データのサンプリング周波数やビット数は変更されません. 音声データのみ修正されます
     * - リサンプリング→自動最大音量処理→ビット数変更→ノイズ除去 の順に処理します
     * - 用途:
     *   - 低サンプリング周波数・低ビットでの聴こえ方のシミュレーション
     *   - ハードウェア再生できないサンプリング周波数やビット数で再生させたい
     * @param $resampling_freq  変更したいサンプリング周波数. $this のそれより低い値でなければなりません
     * @param $resample_method  'point' か 'average'
     * @param $b_auto_maximize  自動最大音量
     * @param $resample_bits    変更したいビット数. $this のそれより低い値でなければなりません. 0 なら変更なし
     * @param $b_noise_filtering 1リサンプリングだけのノイズを除去します
     * @return エラー時は null
     */
    /* 2025.06.15 データが配列からバイナリ文字列になったので未完成
    public function degrade(
        int  $resampling_freq, string $resample_method = 'point',
        bool $b_auto_maximize   = true,
        int  $resample_bits     = 0,
        bool $b_noise_filtering = true): ?\nwk\sound\Wav
    {
        if ($this->sampling_freq_< $resampling_freq || $resampling_freq <= 0) {
            $this->err_msg_ = "サンプリング周波数は $this->$sampling_freq_ 以下でなければなりません:$resampling_freq";
            return null;
        }
        if ($resample_bits === 0) {
            $resample_bits = $this->sample_bits_;
        }
        if ($this->sample_bits_< $resample_bits || $resample_bits <= 0) {
            $this->err_msg_ = "サンプル ビット数は $this->$sample_bits_ 以下でなければなりません:$resample_bits";
            return null;
        }
        if ($this->channels_ !== 1) {
            // YET 未実装
            $this->err_msg_ = "YET 1チャンネルしかサポートしてません:$this->channels_";
            return null;
        }
        if ($this->sample_bits_ !== 16) {
            // YET 未実装
            $this->err_msg_ = "YET 16bit しかサポートしてません:$this->channels_";
            return null;
        }

        $data_str = [];

        // -------- リサンプリング
        $dps  = []; // サンプリング位置
        $fadd = 0;
        $n    = 0;
        switch ($resample_method) {
            case 'point':   // ポイント サンプリング
                foreach($this->data_str_ as $idx => $val) {
                    $fadd += $resampling_freq;
                    if ($this->sampling_freq_ < $fadd) {
                        $dps[] = count($data_str);
                        for ($i = 0; $i < $n; $i++) { $data_str[] = $val; }
                        $fadd -= $this->sampling_freq_;
                        $n     = 0;
                    }
                    $n++;
                    //echo(count($data_str) . '/' . count($this->data_str_) . " n=$n fadd=$fadd\n");
                }
                $n = count($this->data_str_) - count($data_str);
                for ($i = 0; $i < $n; $i++) { $data_str[] = $val; }
            break;

            case 'average': // 平均値
                $sum = 0;
                foreach($this->data_str_ as $idx => $val) {
                    $fadd += $resampling_freq;
                    $sum  += $val;
                    if ($this->sampling_freq_ < $fadd) {
                        $v     = (int)round($sum / (float)$n);
                        $dps[] = count($data_str);
                        //echo(count($data_str) . "#$n = $v\n");
                        for ($i = 0; $i < $n; $i++) { $data_str[] = $v; }
                        $fadd -= $this->sampling_freq_;
                        $n     = 0;
                        $sum   = 0;
                    }
                    $n++;
                }
                $v = (int)round($sum / (float)$n);
                $n = count($this->data_str_) - count($data_str);
                for ($i = 0; $i < $n; $i++) { $data_str[] = $v; }
            break;

            default:
                $this->err_msg_ = "不正なリサンプリング方法です:$resampling_method";
                return null;
        }

        // -------- 自動最大音量処理
        if ($b_auto_maximize) {
            if ($this->autoMaximize_($data_str) === false) {
                return null;
            };
        }

        // -------- ビット
        $bit_mask = ~ ((1 << ($this->sample_bits_ - $resample_bits)) - 1);
        //echo(sprintf("%x\n", $bit_mask));
        if ($resample_bits !== 0) {
            foreach($data_str as &$r_val) {
                $r_val &= $bit_mask;
            }
        }

        //echo(count($this->data_str_) . ' ' . count($data_str) . "\n");
        // -------- ノイズ除去
        if ($b_noise_filtering) {
            for ($i = 1; $i < count($dps) - 1; $i++) {
                $v_1 = $data_str[$dps[$i - 1]];
                $v   = $data_str[$dps[$i]];
                $v1  = $data_str[$dps[$i + 1]];
                if (($v_1 < $v && $v1 < $v) ||
                    ($v_1 > $v && $v1 > $v)) {
                    $val = round(($v_1 + $v1) / 2) & $bit_mask;
                    for ($j = $dps[$i]; $j < $dps[$i + 1]; $j++) {
                        $data_str[$j] = $val;
                    }
                }
            }
        }

        return new \nwk\sound\Wav($this->channels_, $this->sampling_freq_, $this->sample_bits_, $data_str);
    }*/


    // MARK: resample()
    /** チャンネル・サンプリング周波数・ビット数を変換した新しい Wav を返します
     * @param $channels      チャンネル数. 0 なら $this と同じ
     * @param $sampling_freq サンプリング周波数. 0 なら $this と同じ
     * @param $sample_bits   ビット数. 0 なら $this と同じ
     * @returns 失敗したら null を返します
     */
    /* 2025.06.15 データが配列からバイナリ文字列になったので未完成
    public function resample(int $channels = 0, int $sampling_freq = 0, int $sample_bits = 0): ?\nwk\sound\Wav
    {
        $this->err_msg_ = '';

        if ($channels === 0) {
            $channels = $this->channels_;
        }
        if ($sampling_freq === 0) {
            $sampling_freq = $this->sampling_freq_;
        }
        if ($sample_bits === 0) {
            $sample_bits = $this->sample_bits_;
        }

        if ($this->channels_ !== $channels) {
            // YET 未実装
            $this->err_msg_ = "YET チャンネル変換機能は未実装です:$channels";
            return null;
        }
        if ($this->sample_bits_ !== $sample_bits) {
            // YET 未実装
            $this->err_msg_ = "YET サンプリング ビット数変換機能は未実装です:$sample_bits";
            return null;
        }

        $rate = $sampling_freq / $this->sampling_freq_;
        $size0 = $this->getNrSamples();
        $size1 = (int)round((float)($size0 * $rate));
        $rate = $size1 / $size0;                        // 比率再計算
        $data_str = [];
        //echo("rate:$rate size0:$size0 size1:$size1\n");

        // 補完処理
        for ($i = 0; $i < $size1; $i++) {
            // 拡縮後数直線上の i, i+1 に対応する
            // 元数直線上の位置 p, q   を求める
            $p = (float) $i      / $rate;
            $q = (float)($i + 1) / $rate;
            // p - q の「密度」を data_str[$i] に返す
            if (floor($p) === floor($q)) {
                $data_str[] = (float)$this->data_str_[(int)$p] / ($q - $p);
            } else {
                $ip = (int)floor($p);
                $iq = (int)floor($q);
                $weight = (float)$this->data_str_[$ip] * (ceil($p) - $p);
                for ($ip++; $ip < $iq; $ip++) {
                    $weight += $this->data_str_[$ip];
                }
                if (0.0001 < $q - floor($q)) {
                    $weight += (float)$this->data_str_[$ip] * ($q - floor($q));
                }
                $data_str[] = (int)($weight / ($q - $p));
            }
            //if ($i < 5 || ($size1 - 5 < $i)) echo("$i:$data_str[$i] $p - $q:" . $this->data_str_[(int)floor($p)]. "\n");
            //if (($i & 0x03) === 0) echo("$data_str[$i] ");
        }

        return new \nwk\sound\Wav($channels, $sampling_freq, $sample_bits, $data_str);
    }*/


    // ---------------------------------------------------------------- ユーティリティ
    // -------------------------------- チェック
    // MARK: isChannelsSupported()
    /** このチャンネル数をサポートしてるか判定します
     * - エラー時はエラー メッセージを残します
     */
    public function isChannelsSupported(int $channels): bool
    {
        if ($channels === 1) {
            return true;
        }
        $this->err_msg_ = "このチャンネル数はサポートしてません:$channels";
        return false;
    }


    // MARK: isSamplingFrequencySupported()
    /** このサンプリング周波数をサポートしてるか判定します
     * - エラー時はエラー メッセージを残します
     */
    public function isSamplingFrequencySupported(int $sampling_freq): bool
    {
        // 一般に 8000～192000Hz くらいなんだそうです
        if (4000 <= $sampling_freq && $sampling_freq <= 96000) {
            return true;
        }
        $this->err_msg_ = "このサンプリング周波数はサポートしてません:$sampling_freq";
        return false;
    }


    // MARK: isSampleBitsSupported()
    /** このビット数をサポートしてるか判定します
     * - エラー時はエラー メッセージを残します
     */
    public function isSampleBitsSupported(int $sample_bits): bool
    {
        if ($sample_bits === 16 || $sample_bits === 8) {
            return true;
        }
        $this->err_msg_ = "このサンプル ビット数はサポートしてません:$sample_bits";
        return false;
    }


    // MARK: isFormatSupported()
    /** チャンネル・サンプリング周波数・サンプル ビットがサポートされてるかチェックします */
    public function isFormatSupported(int $channels, int $sampling_freq, int $sample_bits): bool
    {
        return
            $this->isChannelsSupported($channels) &&
            $this->isSamplingFrequencySupported($sampling_freq) &&
            $this->isSampleBitsSupported($sample_bits);
    }


    // MARK: isSameFormat()
    /** チャンネル・サンプリング周波数・サンプル ビット が一致するかチェックします */
    public function isSameFormat(int $channels, int $sampling_freq, int $sample_bits): bool
    {
        return
            $this->channels_      === $channels &&
            $this->sampling_freq_ === $sampling_freq &&
            $this->sample_bits_   === $sample_bits;
    }
}
