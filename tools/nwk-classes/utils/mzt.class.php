<?php
/** MZT ユーティリティ
 *
 * @author Snail Barbarian Macho (NWK) 2025.06.20
 */

declare(strict_types = 1);
namespace nwk\utils;


// MARK: Mzt
Class Mzt
{
    // ------------------------------------- パルス幅
    const LEN1H = 464;                      // '1' のHパルス幅(us)
    const LEN1L = 494;                      // '1' のLパルス幅(us)
    const LEN0H = 240;                      // '0' のHパルス幅(us)
    const LEN0L = 264;                      // '0' のLパルス幅(us)

    const SZ_INFO_BLOCK = 0x80;             // インフォメーション ブロックの大きさ
    const INFO_OFFSET_FILE_TYPE = 0x00;     // ファイル タイプ
    const INFO_OFFSET_NAME = 0x01;          // プログラム名. 最大16文字. 0x0d で埋めます
    const INFO_NAME_LEN = 16;               // プログラム名の長さ
    const INFO_OFFSET_SIZE = 0x12;          // データ ブロックの大きさ
    const INFO_OFFSET_LOAD_ADDR = 0x14;     // ロード アドレス
    const INFO_OFFSET_EXEC_ADDR = 0x16;     // 実行アドレス

    const GAP_LEN = 256;

    private string $mzt_str_;       // MZT バイナリ

    // Wavデータ生成用
    private int    $sample_bytes;
    private string $wav_str_;       // WAV バイナリ
    private float  $slen1h_;        // '1' のHパルス幅(サンプリング単位)
    private float  $slen1l_;        // '1' のLパルス幅(サンプリング単位)
    private float  $slen0h_;        // '0' のHパルス幅(サンプリング単位)
    private float  $slen0l_;        // '0' のLパルス幅(サンプリング単位)
    private bool   $b_estimate_;    // false/true = 書込モード/見積モード

    // ---------------------------------------------------------------- コンストラクタ/クローン
    // MARK: __construct()
    /** コンストラクタ
     * @param $r_data_str   データ バイナリ or MZT バイナリ
     * @param ?$name        プログラム名.    MZTバイナリなら, 省略してください
     *                      ※小文字はすべて大文字にして, 16文字に削られます
     * @param ?$load_addr   ロード アドレス. MZTバイナリなら, 省略してください
     * @param ?$exec_addr   実行アドレス.    MZTバイナリなら, 省略してください
     *
     */
    public function __construct(
        string &$r_data_str,
        ?string $name = null, ?int $load_addr = null, ?int $exec_addr = null)
    {
        // 引数チェック
        if (isset($name) && isset($load_addr) && isset($exec_addr)) {
            // $r_data_str は, データ バイナリ

            if (!preg_match("/^[A-Za-z0-9\-\s\.]+$/", $name)) {
                throw new \Exception('The program name contains invalid characters.');
            }

            // プログラム名. 小文字は大文字に, 16文字に揃えて末端を 0x0d で埋めて 17 バイトにします
            $name = substr(strtoupper($name), 0, self::INFO_NAME_LEN);
            for ($i = strlen($name); $i < 17; ++$i) {
                $name .= pack("C", 0x0d);
            }

            $size = strlen($r_data_str);

            $this->mzt_str_ = pack("C", 0x01);        // ファイル モード
            $this->mzt_str_ .= $name;
            $this->mzt_str_ .= pack("v", $size);
            $this->mzt_str_ .= pack("v", $load_addr);
            $this->mzt_str_ .= pack("v", $exec_addr);
            for ($i = 0; $i < 0x68; ++$i) {
                $this->mzt_str_ .= pack("C", 0x00);
            }
            $this->mzt_str_ .= $r_data_str;

        } else if (!isset($name) && !isset($load_addr) && !isset($exec_addr)) {
            // $r_data_str は, MZT バイナリ
            // MZT として正しいかチェック
            if (strlen($r_data_str) <= self::SZ_INFO_BLOCK) {
                throw new \Exception('Invalid mzt data (no header)');
            }

            $data_size = unpack('vsize', substr($r_data_str, self::INFO_OFFSET_SIZE, 2))['size'];
            if (strlen($r_data_str) !== self::SZ_INFO_BLOCK + $data_size) {
                throw new \Exception('Invalid mzt data (invalid data size)');
            }
            $this->mzt_str_ = $r_data_str;
        } else {
            throw new \Exception('Invalid constructor args');
        }
    }


    // MARK: __clone()
    /** クローン */
    public function __clone()
    {
    }


    // ---------------------------------------------------------------- 情報
    // MARK: information
    /** MZTバイナリ文字列のコピーを返します */
    public function getMzt(): string
    {
        return $this->mzt_str_;
    }

    /** データ バイナリ文字列のコピーを返します */
    public function getData(): string
    {
        return substr($this->mzt_str_, self::SZ_INFO_BLOCK);
    }

    /** プログラム名のコピーを返します */
    public function getName(): string
    {
        $str = substr($this->mzt_str_, self::INFO_OFFSET_NAME, self::INFO_NAME_LEN + 1);
        return explode("\x0d", $str)[0];
    }

    /** データ バイナリのサイズを返します */
    public function getDataSize(): int
    {
        return strlen($this->mzt_str_) - self::SZ_INFO_BLOCK;
    }

    /** ロードアドレスを返します */
    public function getLoadAddress(): int
    {
        return unpack('v', substr($this->mzt_str_, self::INFO_OFFSET_LOAD_ADDR, 2))[1];
    }

    /** 開始アドレスを返します */
    public function getExecAddress(): int
    {
        return unpack('v', substr($this->mzt_str_, self::INFO_OFFSET_EXEC_ADDR, 2))[1];
    }

    /** 情報文字列を返します
     * 例: 'name:BARYON size:0x0f43 load:0x1200 exec:0x1202'
     */
    public function getInfoString(): string
    {
        return sprintf("name:%s size:0x%04x load:0x%04x exec:0x%04x",
            $this->getName(), $this->getDataSize(), $this->getLoadAddress(), $this->getExecAddress());
    }


    // ---------------------------------------------------------------- 保存
    // MARK: save()
    /** ファイルに保存します. 成功したか失敗したかを返します */
    public function save(string $filename): bool
    {
        return file_put_contents($filename, $this->mzt_str_) !== false;
    }

    // ---------------------------------------------------------------- Wav データ作成
    /** WAV データを生成します. 1ch 固定. エラーなら null
     * @param b_dataonly インフォメーション ブロックは省略します
     * @param b_firstonly データ ブロックの2回目は省略します
     */
    public function genWavData(
        string $filename,
        int $sample_freq, int $sample_bits,
        bool $b_dataonly = false, bool $b_firstonly = false) : ?string
    {
        // -------------------------------- 引数チェック
        if ($sample_bits !== 8 && $sample_bits !== 16) {
            return null;
        }

        // -------------------------------- 初期化
        $this->sample_bytes = $sample_bits / 8;
        $mzt_len = strlen($this->mzt_str_);
        $this->slen1h_ = self::LEN1H / 1000000 * (float)$sample_freq;
        $this->slen1l_ = self::LEN1L / 1000000 * (float)$sample_freq;
        $this->slen0h_ = self::LEN0H / 1000000 * (float)$sample_freq;
        $this->slen0l_ = (float)self::LEN0L / 1000000 * (float)$sample_freq;
        $info_sum = $this->calcCheckSum_(0, self::SZ_INFO_BLOCK);
        $data_sum = $this->calcCheckSum_(self::SZ_INFO_BLOCK, $mzt_len - self::SZ_INFO_BLOCK);
        $wav_pos = 0.0;

        $this->b_estimate_ = true;          // 見積モード

        for ($i = 0; $i < 2; ++$i) {        // 1 周目は Wav データ サイズの見積, 2 周目は Wav データ書出
            $wav_pos = 0.0;
            // -------------------------------- インフォメーション ブロック
            if (!$b_dataonly) {
                $wav_pos = $this->wavWrite0_($wav_pos, 22000);
                $wav_pos = $this->wavWrite1_($wav_pos, 40);
                $wav_pos = $this->wavWrite0_($wav_pos, 40);
                $wav_pos = $this->wavWrite1_($wav_pos, 1);

                $wav_pos = $this->wavWriteMztBytes_($wav_pos, 0, self::SZ_INFO_BLOCK);
                $wav_pos = $this->wavWriteByte_($wav_pos, $info_sum >> 8); // big endian
                $wav_pos = $this->wavWriteByte_($wav_pos, $info_sum);
                $wav_pos = $this->wavWrite1_($wav_pos, 1);

                $wav_pos = $this->wavWrite0_($wav_pos, 256);

                $wav_pos = $this->wavWriteMztBytes_($wav_pos, 0, self::SZ_INFO_BLOCK);
                $wav_pos = $this->wavWriteByte_($wav_pos, $info_sum >> 8);
                $wav_pos = $this->wavWriteByte_($wav_pos, $info_sum);
                $wav_pos = $this->wavWrite1_($wav_pos, 1);

                $wav_pos = $this->wavWriteGap_($wav_pos, self::GAP_LEN);   // 短いギャップ
            }

            // -------------------------------- データ ブロック
            $wav_pos = $this->wavWrite0_($wav_pos, 11000);
            $wav_pos = $this->wavWrite1_($wav_pos, 20);
            $wav_pos = $this->wavWrite0_($wav_pos, 20);
            $wav_pos = $this->wavWrite1_($wav_pos, 1);

            $wav_pos = $this->wavWriteMztBytes_($wav_pos, self::SZ_INFO_BLOCK, $mzt_len - self::SZ_INFO_BLOCK);
            $wav_pos = $this->wavWriteByte_($wav_pos, $data_sum >> 8);
            $wav_pos = $this->wavWriteByte_($wav_pos, $data_sum);
            $wav_pos = $this->wavWrite1_($wav_pos, 1);
            if (!$b_firstonly) {
                $wav_pos = $this->wavWrite0_($wav_pos, 256);

                $wav_pos = $this->wavWriteMztBytes_($wav_pos, self::SZ_INFO_BLOCK, $mzt_len - self::SZ_INFO_BLOCK);
                $wav_pos = $this->wavWriteByte_($wav_pos, $data_sum >> 8);
                $wav_pos = $this->wavWriteByte_($wav_pos, $data_sum);
                $wav_pos = $this->wavWrite1_($wav_pos, 1);
            }

            if ($i == 0) {                          // 見積が終わったので, Wav データを確保
                $this->wav_str_ = '';
                $this->wav_str_[(int)(ceil($wav_pos)) * $this->sample_bytes - 1] = "\x00";
                $this->b_estimate_ = false;         // 書込モード
            }
        }

        //echo implode(' ', str_split(bin2hex(substr($wav_str, 0, 20)), 2)) . "\n";
        /*$nr_samples = strlen($this->wav_str_) / $this->sample_bytes; $sec = $nr_samples / $sample_freq;
        echo "時間: $sec sec, サンプル数:$nr_samples\n";
        /**/

        return $this->wav_str_;
    }


    /** チェックサム計算 */
    private function calcCheckSum_(int $offset, int $len): int
    {
        $sum = 0;
        $end = $offset + $len;

        for ($i = $offset; $i < $end; ++$i) {
            $byte = unpack('Cbyte', substr($this->mzt_str_, $i, 1))['byte'];
            if ($byte & 0x80) { ++$sum; }
            if ($byte & 0x40) { ++$sum; }
            if ($byte & 0x20) { ++$sum; }
            if ($byte & 0x10) { ++$sum; }
            if ($byte & 0x08) { ++$sum; }
            if ($byte & 0x04) { ++$sum; }
            if ($byte & 0x02) { ++$sum; }
            if ($byte & 0x01) { ++$sum; }
        }
        return $sum;
    }


    /** GAP を書き込みます. 長さは'1'と同じ. 更新した $wav_pos を返します */
    private function wavWriteGap_(float $wav_pos, int $len): float
    {
        if ($this->sample_bytes == 1) {
            for (; 0 < $len; $len --) {
                $wav_pos = $this->wavWrite_pcm8_($wav_pos, $this->slen1h_, "\x00");
                $wav_pos = $this->wavWrite_pcm8_($wav_pos, $this->slen1l_, "\x00");
            }
        } else {
            for (; 0 < $len; $len --) {
                $wav_pos = $this->wavWrite_pcm16_($wav_pos, $this->slen1h_, "\x00", "\x00");
                $wav_pos = $this->wavWrite_pcm16_($wav_pos, $this->slen1l_, "\x00", "\x00");
            }
        }
        return $wav_pos;
    }

    /** "1" を書き込みます. 更新した $wav_pos を返します */
    private function wavWrite1_(float $wav_pos, int $len): float
    {
        if ($this->sample_bytes == 1) {
            for (; 0 < $len; $len --) {
                $wav_pos = $this->wavWrite_pcm8_($wav_pos, $this->slen1h_, "\xff");
                $wav_pos = $this->wavWrite_pcm8_($wav_pos, $this->slen1l_, "\x00");
            }
        } else {
            for (; 0 < $len; $len --) {
                $wav_pos = $this->wavWrite_pcm16_($wav_pos, $this->slen1h_, "\x7f", "\xff");
                $wav_pos = $this->wavWrite_pcm16_($wav_pos, $this->slen1l_, "\x80", "\x00");
            }
        }
        return $wav_pos;
    }

    /** "0" を書き込みます. 更新した $wav_pos を返します */
    private function wavWrite0_(float $wav_pos, int $len): float
    {
        if ($this->sample_bytes == 1) {
            for (; 0 < $len; $len --) {
                $wav_pos = $this->wavWrite_pcm8_($wav_pos, $this->slen0h_, "\xff");
                $wav_pos = $this->wavWrite_pcm8_($wav_pos, $this->slen0l_, "\x00");
            }
        } else {
            for (; 0 < $len; $len --) {
                $wav_pos = $this->wavWrite_pcm16_($wav_pos, $this->slen0h_, "\x7f", "\xff");
                $wav_pos = $this->wavWrite_pcm16_($wav_pos, $this->slen0l_, "\x80", "\x00");
            }
        }
        return $wav_pos;
    }

    /** 1 バイト書き込みます. 更新した $wav_pos を返します */
    private function wavWriteByte_(float $wav_pos, int $val): float
    {
        $wav_pos = $this->wavWrite1_($wav_pos, 1);
        $wav_pos = ($val & 0x80) ? $this->wavWrite1_($wav_pos, 1) : $this->wavWrite0_($wav_pos, 1);
        $wav_pos = ($val & 0x40) ? $this->wavWrite1_($wav_pos, 1) : $this->wavWrite0_($wav_pos, 1);
        $wav_pos = ($val & 0x20) ? $this->wavWrite1_($wav_pos, 1) : $this->wavWrite0_($wav_pos, 1);
        $wav_pos = ($val & 0x10) ? $this->wavWrite1_($wav_pos, 1) : $this->wavWrite0_($wav_pos, 1);
        $wav_pos = ($val & 0x08) ? $this->wavWrite1_($wav_pos, 1) : $this->wavWrite0_($wav_pos, 1);
        $wav_pos = ($val & 0x04) ? $this->wavWrite1_($wav_pos, 1) : $this->wavWrite0_($wav_pos, 1);
        $wav_pos = ($val & 0x02) ? $this->wavWrite1_($wav_pos, 1) : $this->wavWrite0_($wav_pos, 1);
        $wav_pos = ($val & 0x01) ? $this->wavWrite1_($wav_pos, 1) : $this->wavWrite0_($wav_pos, 1);
        return $wav_pos;
    }

    /** $mzt_str の一部を書き込みます. 更新した $wav_pos を返します */
    private function wavWriteMztBytes_(float $wav_pos, int $start, int $len)
    {
        $end = $start + $len;
        for ($i = $start; $i < $end; ++$i) {
            $wav_pos = $this->wavWriteByte_($wav_pos, unpack('C', $this->mzt_str_[$i])[1]);
        }
        return $wav_pos;
    }


    /** $nr_samples 分だけ $val を書き込みます(8bit PCM版) */
    private function wavWrite_pcm8_(float $wav_pos, float $nr_samples, string $val): float
    {
        $pos = $wav_pos + $nr_samples;

        if (!$this->b_estimate_) {

            $start = (int)(round($wav_pos));
            $end   = (int)(round($pos));

            for ($i = $start; $i < $end; ++$i) {
                $this->wav_str_[$i] = $val;
            }
        }
        $wav_pos = $pos;
        return $wav_pos;
    }

    /** $nr_samples 分だけ $val を書き込みます(16bit PCM版) */
    private function wavWrite_pcm16_(float $wav_pos, float $nr_samples, string $val_h, string $val_l): float
    {
        $pos = $wav_pos + $nr_samples;

        if (!$this->b_estimate_) {

            $start = (int)(round($wav_pos)) * $this->sample_bytes;
            $end   = (int)(round($pos))     * $this->sample_bytes;

            for ($i = $start; $i < $end; ) {
                // 16bit PCM は Little endian
                $this->wav_str_[$i] = $val_l; ++$i;
                $this->wav_str_[$i] = $val_h; ++$i;
            }
        }
        $wav_pos = $pos;
        return $wav_pos;
    }



}