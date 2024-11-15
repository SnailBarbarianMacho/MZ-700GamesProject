/**
 * サウンド
 * - 名前空間 SD_, SD1_, SD4_, sd, BGM, SE
 * - 階層図<pre>
 * +-------------+-----------+-----------+
 * | sound       |           |           |
 * | sequencer   | sd1Play() |           |
 * +-------------+-----------+           |
 * | sdMake()                | sd4Play() |
 * +-------------------------+-----------+
 * |          8253 ch 0                  |
 * +-------------------------------------+</pre>
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef SOUND_H_INCLUDED
#define SOUND_H_INCLUDED

#include "../src-common/common.h"
#include "addr.h"


// ---------------------------------------------------------------- マクロ(周波数)
#define SD_FREQ_C1  32.703
#define SD_FREQ_CS1 34.647
#define SD_FREQ_D1  36.647
#define SD_FREQ_DS1 38.890
#define SD_FREQ_E1  41.203
#define SD_FREQ_F1  43.653
#define SD_FREQ_FS1 46.249
#define SD_FREQ_G1  48.999
#define SD_FREQ_GS1 51.913
#define SD_FREQ_A1  55.000
#define SD_FREQ_AS1 58.270
#define SD_FREQ_B1  61.735

#define SD_FREQ_C2  65.406
#define SD_FREQ_CS2 69.296
#define SD_FREQ_D2  73.416
#define SD_FREQ_DS2 77.782
#define SD_FREQ_E2  82.407
#define SD_FREQ_F2  87.307
#define SD_FREQ_FS2 92.499
#define SD_FREQ_G2  97.999
#define SD_FREQ_GS2 103.826
#define SD_FREQ_A2  110.000
#define SD_FREQ_AS2 116.541
#define SD_FREQ_B2  123.471

#define SD_FREQ_C3  130.813
#define SD_FREQ_CS3 138.591
#define SD_FREQ_D3  146.832
#define SD_FREQ_DS3 155.563
#define SD_FREQ_E3  164.814
#define SD_FREQ_F3  174.614
#define SD_FREQ_FS3 184.997
#define SD_FREQ_G3  195.998
#define SD_FREQ_GS3 207.652
#define SD_FREQ_A3  220.000
#define SD_FREQ_AS3 233.082
#define SD_FREQ_B3  246.942

#define SD_FREQ_C4  261.626
#define SD_FREQ_CS4 277.183
#define SD_FREQ_D4  293.665
#define SD_FREQ_DS4 311.127
#define SD_FREQ_E4  329.628
#define SD_FREQ_F4  349.228
#define SD_FREQ_FS4 369.994
#define SD_FREQ_G4  391.995
#define SD_FREQ_GS4 415.305
#define SD_FREQ_A4  440.0
#define SD_FREQ_AS4 466.164
#define SD_FREQ_B4  493.883

#define SD_FREQ_C5  523.251
#define SD_FREQ_CS5 554.365
#define SD_FREQ_D5  587.33
#define SD_FREQ_DS5 622.254
#define SD_FREQ_E5  659.225
#define SD_FREQ_F5  698.456
#define SD_FREQ_FS5 739.989
#define SD_FREQ_G5  783.991
#define SD_FREQ_GS5 830.609
#define SD_FREQ_A5  880.0
#define SD_FREQ_AS5 932.328
#define SD_FREQ_B5  987.767

#define SD_FREQ_C6  1046.502
#define SD_FREQ_CS6 1108.731
#define SD_FREQ_D6  1174.659
#define SD_FREQ_DS6 1244.508
#define SD_FREQ_E6  1318.51
#define SD_FREQ_F6  1396.913
#define SD_FREQ_FS6 1479.978
#define SD_FREQ_G6  1567.982
#define SD_FREQ_GS6 1661.219
#define SD_FREQ_A6  1760.0
#define SD_FREQ_AS6 1864.655
#define SD_FREQ_B6  1975.533
#define SD_FREQ_C7  2093.005

// ---------------------------------------------------------------- マクロ(sdMake() 用音程)
#define SD_MAKE_INTERVAL(freq) (894886.25 / (freq)) // sdMake() の引数を周波数から変換します


// ---------------------------------------------------------------- マクロ(SD4 - PWN方式4重和音)
/**
 * 機能:
 * - SD4スペック:
 *   - 最も内側の Beeper ループの周期は, 15700Hz
 *     - 1分間の四分音符の数♩: 60 * 15700 / (256 * 32) = 114.3 (テンポは倍くらいまでいける)
 * - 機能やデータの作り方は, MIDI コンバータ sd4_midi_conv.php を参照してください
 */

// -------- 音譜
/*
 * len 音長 ... [1, 2, ...  63, 0] → [4, 8, ... 252, 256]
 * wl  波長 ... [1, 2, ... 255, 0] → [1, 2, ... 255, 256]
 */

#define SD4_EXCEPT_LEN  200  // この「長さ」は特別なコマンドで使われます

/* Lead         L                  H                  L
 *                    +-+-+-+-+-+-+-+-+
 *   休符             |0 0|    len    |
 *                    +-+-+-+-+-+-+-+-+
 *                    +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ■               |1 1|    len    |  |       wl      |
 *                    +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *                    +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ◢               |0 1|    len    |  |       wl      |
 *                    +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *                    +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ◣               |1 0|    len    |  |       wl      |
 *                    +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 * Lead の特定の音長は命令になります. 音長値は, SD4_EXCEPT_LEN#SD4_EXCEPT_LEN_SZ で定義
 *                    +-+-+-+-+-+-+-+-+
 *   リピート         |0 0|  [e, e+7] |  1～8回,5ネスト迄
 *                    +-+-+-+-+-+-+-+-+
 *                    +-+-+-+-+-+-+-+-+
 *   リピート終了     |0 1|     e     |
 *                    +-+-+-+-+-+-+-+-+
 *                    +-+-+-+-+-+-+-+-+
 *   終了             |0 1|     e+1   |
 *                    +-+-+-+-+-+-+-+-+
 *                    +-+-+-+-+-+-+-+-+  ◢◣用
 *   エンベロープ速度 |1 0|  [e, e+3] |  e/e+1/e+2/e+3 = 1/2, 1/4, 1/8(デフォルト), 1/16
 *                    +-+-+-+-+-+-+-+-+
 *                    +-+-+-+-+-+-+-+-+  ■用. スライド時は, 波長カウンタのリセットや音量の再設定はしません
 *   スライド速度     |1 1|  [e, e+3] |  e/e+1/e+2 = スライド無(デフォルト), 1/1, 1/2, 1/3
 *                    +-+-+-+-+-+-+-+-+
 */
#define SD4_L_REST(len)         (0x00 | (((len) & 0xfc) >> 2))      // 休符
#define SD4_L_UP(len, wl)       (0x40 | (((len) & 0xfc) >> 2)), ((int)(wl) & 0xff)  // 音譜(エンベロープ=◢)
#define SD4_L_DW(len, wl)       (0x80 | (((len) & 0xfc) >> 2)), ((int)(wl) & 0xff)  // 音譜(エンベロープ=◣)
#define SD4_L_FL(len, wl)       (0xc0 | (((len) & 0xfc) >> 2)), ((int)(wl) & 0xff)  // 音譜(エンベロープ=■)
#define SD4_L_REP_2             (0x00 | ((SD4_EXCEPT_LEN + 0) >> 2))    // リピート2
#define SD4_L_REP_3             (0x00 | ((SD4_EXCEPT_LEN + 4) >> 2))    // リピート3
#define SD4_L_REP_4             (0x00 | ((SD4_EXCEPT_LEN + 8) >> 2))    // リピート4
#define SD4_L_REP_5             (0x00 | ((SD4_EXCEPT_LEN + 12) >> 2))   // リピート5
#define SD4_L_REP_6             (0x00 | ((SD4_EXCEPT_LEN + 16) >> 2))   // リピート6
#define SD4_L_REP_7             (0x00 | ((SD4_EXCEPT_LEN + 20) >> 2))   // リピート7
#define SD4_L_REP_8             (0x00 | ((SD4_EXCEPT_LEN + 24) >> 2))   // リピート8
#define SD4_L_ENDR              (0x40 | ((SD4_EXCEPT_LEN + 0) >> 2))    // リピート終了
#define SD4_L_END               (0x40 | ((SD4_EXCEPT_LEN + 4) >> 2))    // 終了
#define SD4_L_ENV_SPEED_R1      (0x80 | ((SD4_EXCEPT_LEN + 0) >> 2))    // エンベロープ速度(1/1)
#define SD4_L_ENV_SPEED_R2      (0x80 | ((SD4_EXCEPT_LEN + 4) >> 2))    // エンベロープ速度(1/2)
#define SD4_L_ENV_SPEED_R4      (0x80 | ((SD4_EXCEPT_LEN + 8) >> 2))    // エンベロープ速度(1/4)
#define SD4_L_ENV_SPEED_R8      (0x80 | ((SD4_EXCEPT_LEN + 12) >> 2))   // エンベロープ速度(1/8)
#define SD4_L_SLIDE_OFF         (0xc0 | ((SD4_EXCEPT_LEN + 0) >> 2))    // スライド無
#define SD4_L_SLIDE_SPEED_R1    (0xc0 | ((SD4_EXCEPT_LEN + 4) >> 2))    // スライド速度(1/1)
#define SD4_L_SLIDE_SPEED_R2    (0xc0 | ((SD4_EXCEPT_LEN + 8) >> 2))    // スライド速度(1/2)
#define SD4_L_SLIDE_SPEED_R4    (0xc0 | ((SD4_EXCEPT_LEN + 12) >> 2))   // スライド速度(1/4)

/* Chord2      L                  H
 *             +-+-+-+-+-+-+-+-+
 *   休符      |0 0|    len    |
 *             +-+-+-+-+-+-+-+-+
 *             +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ◢        |0 1|    len    |  |       wl0     |  |       wl1     |
 *             +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *             +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ◣        |1 0|    len    |  |       wl0     |  |       wl1     |
 *             +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *             +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ■        |1 1|    len    |  |       wl0     |  |       wl1     |
 *             +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 */
#define SD4_C2_REST(len)         SD4_L_REST(len)                      // 休符
#define SD4_C2_UP(len, wl0, wl1) SD4_L_UP(len, wl0), (wl1)            // 音譜(エンベロープ=◢)
#define SD4_C2_DW(len, wl0, wl1) SD4_L_DW(len, wl0), (wl1)            // 音譜(エンベロープ=◣)
#define SD4_C2_FL(len, wl0, wl1) (0xc0 | (((len) & 0xfc) >> 2)), ((int)(wl0) & 0xff), ((int)(wl1) & 0xff) // 音譜(エンベロープ=■)

/* Base         L                  H
 *             +-+-+-+-+-+-+-+-+
 *   休符      |0 0|    len    |
 *             +-+-+-+-+-+-+-+-+
 *             +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ◢        |0 1|    len    |  |       wl      |
 *             +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *             +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ◣        |1 0|    len    |  |       wl      |
 *             +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *             +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 *   ■        |1 1|    len    |  |       wl      |
 *             +-+-+-+-+-+-+-+-+  +-+-+-+-+-+-+-+-+
 */
#define SD4_B_REST(len)         SD4_L_REST(len)                       // 休符
#define SD4_B_UP(len, wl)       SD4_L_UP(len, wl)                     // 音譜(エンベロープ=◢)
#define SD4_B_DW(len, wl)       SD4_L_DW(len, wl)                     // 音譜(エンベロープ=◣)
#define SD4_B_FL(len, wl)       (0xc0 | (((len) & 0xfc) >> 2)), ((int)(wl) & 0xff)  // 音譜(エンベロープ=■)

/** Drum         L
 *              +-+-+-+-+-+-+-+-+
 *  休符+ドラム |   len     |d d|      d ... ドラム番号[1～3]. 0 は休符
 *              +-+-+-+-+-+-+-+-+
 */
#define SD4_D_REST(len)         (((len) & 0xfc) | 0x00)               // 休符
#define SD4_D_1(len)            (((len) & 0xfc) | 0x01)               // ドラム1
#define SD4_D_2(len)            (((len) & 0xfc) | 0x02)               // ドラム2
#define SD4_D_3(len)            (((len) & 0xfc) | 0x03)               // ドラム3

// -------- 波長(音譜の wl の値)
// C2 が出来るだけ大きな値になるように調整します(8で割り切れるくらい 0xf8 くらいが理想)
//#define SD4_WL(freq)  (((float)(0xf8) / (freq) * SD_FREQ_C2) + 0.5f)
#define SD4_WL(freq)  (((float)(0x100) / (freq) * (SD_FREQ_B1 + 0.15f)) + 0.5f)// 低音なので多少のオンチは目をつぶる

#define SD4_B1  0xff    // 残念. 0x00 にはできない

#define SD4_C2  SD4_WL(SD_FREQ_C2)
#define SD4_CS2 SD4_WL(SD_FREQ_CS2)
#define SD4_D2  SD4_WL(SD_FREQ_D2)
#define SD4_DS2 SD4_WL(SD_FREQ_DS2)
#define SD4_E2  SD4_WL(SD_FREQ_E2)
#define SD4_F2  SD4_WL(SD_FREQ_F2)
#define SD4_FS2 SD4_WL(SD_FREQ_FS2)
#define SD4_G2  SD4_WL(SD_FREQ_G2)
#define SD4_GS2 SD4_WL(SD_FREQ_GS2)
#define SD4_A2  SD4_WL(SD_FREQ_A2)
#define SD4_AS2 SD4_WL(SD_FREQ_AS2)
#define SD4_B2  SD4_WL(SD_FREQ_B2)

#define SD4_C3  SD4_WL(SD_FREQ_C3)
#define SD4_CS3 SD4_WL(SD_FREQ_CS3)
#define SD4_D3  SD4_WL(SD_FREQ_D3)
#define SD4_DS3 SD4_WL(SD_FREQ_DS3)
#define SD4_E3  SD4_WL(SD_FREQ_E3)
#define SD4_F3  SD4_WL(SD_FREQ_F3)
#define SD4_FS3 SD4_WL(SD_FREQ_FS3)
#define SD4_G3  SD4_WL(SD_FREQ_G3)
#define SD4_GS3 SD4_WL(SD_FREQ_GS3)
#define SD4_A3  SD4_WL(SD_FREQ_A3)
#define SD4_AS3 SD4_WL(SD_FREQ_AS3)
#define SD4_B3  SD4_WL(SD_FREQ_B3)

#define SD4_C4  SD4_WL(SD_FREQ_C4)
#define SD4_CS4 SD4_WL(SD_FREQ_CS4)
#define SD4_D4  SD4_WL(SD_FREQ_D4)
#define SD4_DS4 SD4_WL(SD_FREQ_DS4)
#define SD4_E4  SD4_WL(SD_FREQ_E4)
#define SD4_F4  SD4_WL(SD_FREQ_F4)
#define SD4_FS4 SD4_WL(SD_FREQ_FS4)
#define SD4_G4  SD4_WL(SD_FREQ_G4)
#define SD4_GS4 SD4_WL(SD_FREQ_GS4)
#define SD4_A4  SD4_WL(SD_FREQ_A4)
#define SD4_AS4 SD4_WL(SD_FREQ_AS4)
#define SD4_B4  SD4_WL(SD_FREQ_B4)

#define SD4_C5  SD4_WL(SD_FREQ_C5)
#define SD4_CS5 SD4_WL(SD_FREQ_CS5)
#define SD4_D5  SD4_WL(SD_FREQ_D5)
#define SD4_DS5 SD4_WL(SD_FREQ_DS5)
#define SD4_E5  SD4_WL(SD_FREQ_E5)
#define SD4_F5  SD4_WL(SD_FREQ_F5)
#define SD4_FS5 SD4_WL(SD_FREQ_FS5)
#define SD4_G5  SD4_WL(SD_FREQ_G5)
#define SD4_GS5 SD4_WL(SD_FREQ_GS5)
#define SD4_A5  SD4_WL(SD_FREQ_A5)
#define SD4_AS5 SD4_WL(SD_FREQ_AS5)
#define SD4_B5  SD4_WL(SD_FREQ_B5)

#define SD4_C6  SD4_WL(SD_FREQ_C6)

// -------- テーブル
#define SD4_SZ_DRUM_TAB  0x0100  // ドラム テーブル1個の大きさ

// ---------------------------------------------------------------- システム
/** サウンド システムの初期化 */
void sd4Init(void);

/**
 * SD6 - PWM方式4重和音サウンドを鳴らします
 * - 鳴ってる間は 他の作業はできません
 * - 音が鳴らない場合は, 減衰テーブルやサウンド テータが上位 RAM に存在してないか確認してください
 * @param param (キャンセル可能か<<8) | MML データ
 * - MML データは, 全 Beeper のデータを1本にまとめたものです.
 *   各 Beeper が適した音符を拾って鳴らします
 * - チャタリング防止のため, 演奏終了前は, F1/F2/F6 キーを離すまで待ちます
 * - キャンセル可能時は, F1/F2/F6 キーでキャンセルできます
 *   チャタリング防止のため, 演奏終了後は, F1/F2/F6 キーを離すまで待ちます
 * @return キャンセルした場合は KEY9_F1_MASK, KEY9_F2_MASK, KEY9_F4_MASK の bitwise or.
 *   キーを押さないで終了した場合は, 0 を返します
 */
u8 sd4play(u32 param) __z88dk_fastcall;

#endif // SOUND_H_INCLUDED
