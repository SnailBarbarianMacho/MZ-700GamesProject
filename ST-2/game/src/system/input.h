/**
 * 入力
 * - 名前空間 INPUT_ または input
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include "../../../../src-common/common.h"


// ---------------------------------------------------------------- private 変数. 直接触らない
extern u8 input_;            // キー入力 + ジョイスティック入力データ
extern u8 input_trg_;        // キー入力 + ジョイスティック入力 OFF->ONトリガ データ
extern u8 input_joy_;        // ジョイスティック入力データ
extern u8 input_joy_mode_;   // ジョイスティック検出モード兼カウンタ
extern u8 input_mz1x03_insensitivity_;    // MZ-1X03 の感度の鈍さ (1敏感～4鈍い)
#define INPUT_MZ1X03_INSENSITIVITY_MIN 1  // MZ-1X03 の感度の鈍さ (敏感)
#define INPUT_MZ1X03_INSENSITIVITY_MAX 4  // MZ-1X03 の感度の鈍さ (鈍い)

// ---------------------------------------------------------------- マクロ
// inputGet(), inputGetTrigger() 戻値のビット マスク. AM7J(..UDRLBA) 互換です
#define INPUT_MASK_A 0x01   // A ボタン
#define INPUT_MASK_B 0x02   // B ボタン
#define INPUT_MASK_L 0x04   // 左
#define INPUT_MASK_R 0x08   // 右
#define INPUT_MASK_D 0x10   // 下
#define INPUT_MASK_U 0x20   // 上
#define INPUT_MASK_PLAY 0x40  // Play/Pause/Back
#define INPUT_MASK_SEL  0x80  // Select/GameOver
#define INPUT_MASK_NC   0x80  // ジョイスティックの場合, 未検出フラグになります
#define INPUT_SHIFT_NC  0x7   // ジョイスティックの場合, 未検出フラグになります

// input_joy_mode_ の取る値
#define INPUT_JOY_MODE_AM7J_DETECTING       0   // AM7J   検出中
#define INPUT_JOY_MODE_AM7J_DETECTED        10  // AM7J   検出
#define INPUT_JOY_MODE_AM7J_UNDETECTING     11  // AM7J   検出したがデータが来ない
#define INPUT_JOY_MODE_MZ1X03_DETECTING     20  // MZ1X03 検出中
#define INPUT_JOY_MODE_MZ1X03_DETECTED      30  // MZ1X03 検出
#define INPUT_JOY_MODE_MZ1X03_UNDETECTING   31  // MZ1X03 検出したがデータが来ない
#define INPUT_JOY_MODE_MAX                  40

// ---------------------------------------------------------------- システム
/** 入力の初期化を行います */
void inputInit() __z88dk_fastcall;
/** 入力生データを更新します. システムのほか, ポーズ時も使用 */
void inputMain() __z88dk_fastcall;

// ---------------------------------------------------------------- 入力
/** 入力生データを入手します. INPUT_MASK_XXXX の bitwise or */
inline u8 inputGet() { return input_; }
/** 入力 OFF->ON データを入手します. INPUT_MASK_XXXX の bitwise or */
inline u8 inputGetTrigger() { return input_trg_; }

/** ジョイスティックで検出した値を返します. 未検出の場合は 0x00 */
inline u8 inputGetJoy() { return input_joy_; }

/** ジョイスティック検出モード兼カウンタを返します */
inline u8 inputGetJoyMode() { return input_joy_mode_; }

// ---------------------------------------------------------------- MZ-1X13
/** MZ-1X03 ボタンを読んで, VBLANK を待ち, 接続をテストして, 感度の鈍さの分だけ待って, 軸読み取り1をします
 * - 1) MZ-1X03 ボタンABのリード
 * - 2) /VBLK == L になるまで待つ
 * - 3) 100 T-states 待つ
 * - 4) 50  T-states の間, ボタンABが 'L' でなければ非接続
 * - 5) 130 T-states 待つ
 * - 6) 834 * (4 - mz11x03_insensitivity) だけ待つ
 * - 7) 軸読み取り 1 する
 * */
void inputMZ1X03ButtonVSyncAxis1(const u8 mz1x03_insensitivity) __z88dk_fastcall __naked;
/** MZ-1X03 軸読み取り2をして, 結果を返します */
void inputMZ1X03Axis2() __z88dk_fastcall __naked;

/** MZ-1X03 感度の鈍さ(1敏感～4鈍い)を返します */
inline u8 inputGetMZ1X03Insensitivity() { return input_mz1x03_insensitivity_; }
/** MZ-1X03 感度の鈍さ(1敏感～4鈍い) を -1 します */
inline void inputDecMZ1X03Insensitivity()
{
    input_mz1x03_insensitivity_ --;
    if (input_mz1x03_insensitivity_ == 0) {
        input_mz1x03_insensitivity_ = INPUT_MZ1X03_INSENSITIVITY_MAX;
    }
}


#endif // INPUT_H_INCLUDED