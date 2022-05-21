/**
 * サウンド
 * - 名前空間 SOUND_ または sound
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef SOUND_H_INCLUDED
#define SOUND_H_INCLUDED

#include "../src-common/common.h"

// -------------------------------- 周波数
#define SOUND_FREQ_C2   65.406
#define SOUND_FREQ_CS2  69.296
#define SOUND_FREQ_D2   73.416
#define SOUND_FREQ_DS2  77.782
#define SOUND_FREQ_E2  	82.407
#define SOUND_FREQ_F2   87.307
#define SOUND_FREQ_FS2  92.499
#define SOUND_FREQ_G2   97.999
#define SOUND_FREQ_GS2  103.826
#define SOUND_FREQ_A2   110.000
#define SOUND_FREQ_AS2  116.541
#define SOUND_FREQ_B2   123.471

#define SOUND_FREQ_C3   130.813
#define SOUND_FREQ_CS3  138.591
#define SOUND_FREQ_D3   146.832
#define SOUND_FREQ_DS3  155.563
#define SOUND_FREQ_E3   164.814
#define SOUND_FREQ_F3   174.614
#define SOUND_FREQ_FS3  184.997
#define SOUND_FREQ_G3   195.998
#define SOUND_FREQ_GS3  207.652
#define SOUND_FREQ_A3   220.000
#define SOUND_FREQ_AS3  233.082
#define SOUND_FREQ_B3   246.942

#define SOUND_FREQ_C4   261.626
#define SOUND_FREQ_CS4  277.183
#define SOUND_FREQ_D4   293.665
#define SOUND_FREQ_DS4  311.127
#define SOUND_FREQ_E4   329.628
#define SOUND_FREQ_F4   349.228
#define SOUND_FREQ_FS4  369.994
#define SOUND_FREQ_G4   391.995
#define SOUND_FREQ_GS4  415.305
#define SOUND_FREQ_A4   440.0
#define SOUND_FREQ_AS4  466.164
#define SOUND_FREQ_B4   493.883

#define SOUND_FREQ_C5   523.251
#define SOUND_FREQ_CS5  554.365
#define SOUND_FREQ_D5   587.33
#define SOUND_FREQ_DS5  622.254
#define SOUND_FREQ_E5   659.225
#define SOUND_FREQ_F5   698.456
#define SOUND_FREQ_FS5  739.989
#define SOUND_FREQ_G5   783.991
#define SOUND_FREQ_GS5  830.609
#define SOUND_FREQ_A5   880.0
#define SOUND_FREQ_AS5  932.328
#define SOUND_FREQ_B5   987.767

#define SOUND_FREQ_C6   1046.502
#define SOUND_FREQ_CS6  1108.731
#define SOUND_FREQ_D6   1174.659
#define SOUND_FREQ_DS6  1244.508
#define SOUND_FREQ_E6   1318.51
#define SOUND_FREQ_F6   1396.913
#define SOUND_FREQ_FS6  1479.978
#define SOUND_FREQ_G6   1567.982
#define SOUND_FREQ_GS6  1661.219
#define SOUND_FREQ_A6   1760.0
#define SOUND_FREQ_AS6  1864.655
#define SOUND_FREQ_B6   1975.533
#define SOUND_FREQ_C7   2093.005

// -------------------------------- sd3Play() 三重和音の音程
#define SD3_SAMPLE_DURATION (274.0 / 3579545.0)    // 三重和音 1 ループ内での時間
#define SD3_LT(freq)  (u8)(1.0f / freq / SD3_SAMPLE_DURATION + 0.0f)// 内部使用. ループ回数

#define SD3_C2    SD3_LT(SOUND_FREQ_C2)
#define SD3_CS2   SD3_LT(SOUND_FREQ_CS2)
#define SD3_D2    SD3_LT(SOUND_FREQ_D2)
#define SD3_DS2   SD3_LT(SOUND_FREQ_DS2)
#define SD3_E2    SD3_LT(SOUND_FREQ_E2)
#define SD3_F2    SD3_LT(SOUND_FREQ_F2)
#define SD3_FS2   SD3_LT(SOUND_FREQ_FS2)
#define SD3_G2    SD3_LT(SOUND_FREQ_G2)
#define SD3_GS2   SD3_LT(SOUND_FREQ_GS2)
#define SD3_A2    SD3_LT(SOUND_FREQ_A2)
#define SD3_AS2   SD3_LT(SOUND_FREQ_AS2)
#define SD3_B2    SD3_LT(SOUND_FREQ_B2)

#define SD3_C3    SD3_LT(SOUND_FREQ_C3)
#define SD3_CS3   SD3_LT(SOUND_FREQ_CS3)
#define SD3_D3    SD3_LT(SOUND_FREQ_D3)
#define SD3_DS3   SD3_LT(SOUND_FREQ_DS3)
#define SD3_E3    SD3_LT(SOUND_FREQ_E3)
#define SD3_F3    SD3_LT(SOUND_FREQ_F3)
#define SD3_FS3   SD3_LT(SOUND_FREQ_FS3)
#define SD3_G3    SD3_LT(SOUND_FREQ_G3)
#define SD3_GS3   SD3_LT(SOUND_FREQ_GS3)
#define SD3_A3    SD3_LT(SOUND_FREQ_A3)
#define SD3_AS3   SD3_LT(SOUND_FREQ_AS3)
#define SD3_B3    SD3_LT(SOUND_FREQ_B3)

#define SD3_C4    SD3_LT(SOUND_FREQ_C4)
#define SD3_CS4   SD3_LT(SOUND_FREQ_CS4)
#define SD3_D4    SD3_LT(SOUND_FREQ_D4)
#define SD3_DS4   SD3_LT(SOUND_FREQ_DS4)
#define SD3_E4    SD3_LT(SOUND_FREQ_E4)
#define SD3_F4    SD3_LT(SOUND_FREQ_F4)
#define SD3_FS4   SD3_LT(SOUND_FREQ_FS4)
#define SD3_G4    SD3_LT(SOUND_FREQ_G4)
// ここ付近から先は音程が崩れ出すので実用的ではないです
#define SD3_GS4   SD3_LT(SOUND_FREQ_GS4)
#define SD3_A4    SD3_LT(SOUND_FREQ_A4)
#define SD3_AS4   SD3_LT(SOUND_FREQ_AS4)
#define SD3_B4    SD3_LT(SOUND_FREQ_B4)

// -------------------------------- システム
/** サウンド システムの初期化 */
void soundInit() __z88dk_fastcall;
/** サウンドの ON/OFF を制御します. 初期値は, false です.
 * シーンが変更されると自動で false になります
 */
void sdSetEnabled(const bool bEnabled) __z88dk_fastcall __naked;

// -------------------------------- 三重和音サウンド
/**
 * 波形合成による三重和音サウンドを鳴らします.
 * - 鳴ってる間は 他の作業はできません
 * - サウンド データは「音程, 長さ...」で指定します. 休符はありません.
 *   - 音程は SD3_XXX マクロで指定します. 休符はありません
 *   - 長さは 100 を入れると, 約 1 秒鳴ります
 *   - どれかのサウンド データの周波数に, (マクロでなく,) 0 を入れるとそこで終了します
 *     必ず 1 音必要です. いきなり 0 は出来ません.
 * @param mml0 サウンド データ0
 * @param mml1 サウンド データ1
 * @param mml2 サウンド データ2
 */
void sd3Play(const u8* mml0, const u8* mml1, const u8* mml2) __naked;


#endif // SOUND_H_INCLUDED
