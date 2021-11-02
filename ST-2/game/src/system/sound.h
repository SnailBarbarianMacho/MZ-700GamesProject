/**
 * サウンド
 * - 名前空間 SD_, SD1_, SD3_ または sd
 * - 階層図<pre>
 * +-------------+-----------+-----------+
 * | sound       |           |           |
 * | sequencer   | sd1Play() |           |
 * +-------------+-----------+           |
 * | sdMake()                | sd3Play() |
 * +-------------------------+-----------+
 * |          8253 ch 0                  |
 * +-------------------------------------+</pre>
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef SOUND_INCLUDED
#define SOUND_H_INCLUDED

#include "../../../../src-common/common.h"
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

// ---------------------------------------------------------------- マクロ(sd1Play() 用音程)
#define SD1_R       2// 休符

#define SD1_C2      4
#define SD1_CS2     6
#define SD1_D2      8
#define SD1_DS2     10
#define SD1_E2      12
#define SD1_F2      14
#define SD1_FS2     16
#define SD1_G2      18
#define SD1_GS2     20
#define SD1_A2      22
#define SD1_AS2     24
#define SD1_B2      26

#define SD1_C3      28
#define SD1_CS3     30
#define SD1_D3      32
#define SD1_DS3     34
#define SD1_E3      36
#define SD1_F3      38
#define SD1_FS3     40
#define SD1_G3      42
#define SD1_GS3     44
#define SD1_A3      46
#define SD1_AS3     48
#define SD1_B3      50

#define SD1_C4      52
#define SD1_CS4     54
#define SD1_D4      56
#define SD1_DS4     58
#define SD1_E4      60
#define SD1_F4      62
#define SD1_FS4     64
#define SD1_G4      66
#define SD1_GS4     68
#define SD1_A4      70
#define SD1_AS4     72
#define SD1_B4      74

// ---------------------------------------------------------------- マクロ(sd3Play() 用音程)
#define SD3_SAMPLE_DURATION (286.0f / 3579545.0f)    // 三重和音 1 ループ内での時間
#define SD3_LT(freq)  (u8)(1.0f / freq / SD3_SAMPLE_DURATION + 0.0f)// 内部使用. ループ回数

// これより低い音は 8bit 値を越えるのでダメ
#define SD3_G1  SD3_LT(SD_FREQ_G1)
#define SD3_GS1 SD3_LT(SD_FREQ_GS1)
#define SD3_A1  SD3_LT(SD_FREQ_A1)
#define SD3_AS1 SD3_LT(SD_FREQ_AS1)
#define SD3_B1  SD3_LT(SD_FREQ_B1)

#define SD3_C2  SD3_LT(SD_FREQ_C2)
#define SD3_CS2 SD3_LT(SD_FREQ_CS2)
#define SD3_D2  SD3_LT(SD_FREQ_D2)
#define SD3_DS2 SD3_LT(SD_FREQ_DS2)
#define SD3_E2  SD3_LT(SD_FREQ_E2)
#define SD3_F2  SD3_LT(SD_FREQ_F2)
#define SD3_FS2 SD3_LT(SD_FREQ_FS2)
#define SD3_G2  SD3_LT(SD_FREQ_G2)
#define SD3_GS2 SD3_LT(SD_FREQ_GS2)
#define SD3_A2  SD3_LT(SD_FREQ_A2)
#define SD3_AS2 SD3_LT(SD_FREQ_AS2)
#define SD3_B2  SD3_LT(SD_FREQ_B2)

#define SD3_C3  SD3_LT(SD_FREQ_C3)
#define SD3_CS3 SD3_LT(SD_FREQ_CS3)
#define SD3_D3  SD3_LT(SD_FREQ_D3)
#define SD3_DS3 SD3_LT(SD_FREQ_DS3)
#define SD3_E3  SD3_LT(SD_FREQ_E3)
#define SD3_F3  SD3_LT(SD_FREQ_F3)
#define SD3_FS3 SD3_LT(SD_FREQ_FS3)
#define SD3_G3  SD3_LT(SD_FREQ_G3)
#define SD3_GS3 SD3_LT(SD_FREQ_GS3)
#define SD3_A3  SD3_LT(SD_FREQ_A3)
#define SD3_AS3 SD3_LT(SD_FREQ_AS3)
#define SD3_B3  SD3_LT(SD_FREQ_B3)

#define SD3_C4  SD3_LT(SD_FREQ_C4)
#define SD3_CS4 SD3_LT(SD_FREQ_CS4)
#define SD3_D4  SD3_LT(SD_FREQ_D4)
#define SD3_DS4 SD3_LT(SD_FREQ_DS4)
#define SD3_E4  SD3_LT(SD_FREQ_E4)
#define SD3_F4  SD3_LT(SD_FREQ_F4)
#define SD3_FS4 SD3_LT(SD_FREQ_FS4)
#define SD3_G4  SD3_LT(SD_FREQ_G4)
// ここ付近から先は音程が崩れ出すので実用的ではないです
#define SD3_GS4 SD3_LT(SD_FREQ_GS4)
#define SD3_A4  SD3_LT(SD_FREQ_A4)
#define SD3_AS4 SD3_LT(SD_FREQ_AS4)
#define SD3_B4  SD3_LT(SD_FREQ_B4)

// ---------------------------------------------------------------- マクロ
#define SD_SE_PRIORITY_0 0  // 最も高い SE 優先順位
#define SD_SE_PRIORITY_1 1
#define SD_SE_PRIORITY_2 2
#define SD_SE_PRIORITY_3 3  // 最も低い SE 優先順位

// ---------------------------------------------------------------- private 変数. 直接触らない
extern u16  (*_bgmMain)(u16);
extern u16  _bgmCt;
extern u8   _sePri;

// ---------------------------------------------------------------- システム
/** サウンド システムの初期化 */
void sdInit();
/** サウンド システムのメイン(BGM) */
void sdBgmMain();
/** サウンド システムのメイン(SE) */
void sdSeMain();

/** サウンドの ON/OFF を制御します. 初期値は, false です.
 * - false だと全ての音声が鳴りません
 * - シーンが変更されると自動で false になります
 */
void sdSetEnabled(const bool bEnabled) __z88dk_fastcall __naked;

// ---------------------------------------------------------------- サウンド シーケンサ
/** BGM を鳴らします.
 * - シーンが変更されると自動で SD_BGM_NONE になります
 * @param bgm BGM_XXXX を指定します.
 */
void sdPlayBgm(const u8 bgm) __z88dk_fastcall;

/** SE を鳴らします.
 * @param bgm SE_XXXX を指定します.
 */
void sdPlaySe(const u8 se) __z88dk_fastcall;

/** 現在の SE(効果音) プライオリティを返します */
inline u8 sdGetSePriority() { return _sePri; }

// ---------------------------------------------------------------- 音を鳴らす
/**
 * サウンドの周波数を設定します. 初期値は, 不定です
 * @param interval 音程
 * - 優先順位は, 値が小さいほど優先が高くなります
 * - オールゼロ(音程 0x0000, 優先順位 0) にすると, 最も優先が低くなります
 * - 音程は,「894886.25 / 周波数」で求まります. SD_MAKE_INTERVAL() マクロが便利です
 * - 参考: 各オクターブの「ラ」の音:<pre>
 *     低音   55Hz 0x3f8f
 *           110Hz 0x1fc7
 *           220Hz 0x0fe4
 *           440Hz 0x07f2
 *           880Hz 0x03f9
 *     高音 1760Hz 0x01fc</pre>
 */
void sdMake(const u16 interval) __z88dk_fastcall __naked;

/** sdMake() の音程の代わりに音階を使用するバージョンです
 * @param scale 音階 SD1_XXXX マクロを使ってください. 偶数です
 */
void sd1Play(const u8 scale) __z88dk_fastcall __naked;
/**
 * 波形合成による三重和音サウンドを鳴らします.
 * - 鳴ってる間は 他の作業はできません
 * - サウンド データは「音程, 長さ...」で指定します. 休符はありません.
 *   - 音程は SD3_XXXX マクロで指定します. 休符はありません
 *   - 長さは 100 を入れると, 約 1 秒鳴ります
 *   - どれかのサウンド データの周波数に, (マクロでなく,) 0 を入れるとそこで終了します
 *     必ず 1 音必要です. いきなり 0 は出来ません.
 * @param mml0 サウンド データ0
 * @param mml1 サウンド データ1
 * @param mml2 サウンド データ2
 * @param bCancelEnabled true にすると F1 キーを押しっぱなしでキャンセルします.
 *  - キャンセル時は, F1 キーを離さないと関数は終了しません
 * @return false/true = キャンセルされた/通常終了
 */
bool sd3Play(const u8* mml0, const u8* mml1, const u8* mml2, const bool bCancelEnabled) __naked;


#endif // SOUND_H_INCLUDED
