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
extern u8 input_mz1x03_sensitivity_;    // MZ-1X03 の感度(0鈍い～3敏感)

#define INPUT_MZ1X03_SENSITIVITY_MIN 0  // MZ-1X03 の感度の鈍さ (敏感)
#define INPUT_MZ1X03_SENSITIVITY_MAX 3  // MZ-1X03 の感度の鈍さ (鈍い)

// ---------------------------------------------------------------- マクロ
// inputGet(), inputGetTrigger() 戻値のビット マスク. AM7J(..UDRLBA) 互換です
#define INPUT_MASK_A 0x01   // A ボタン
#define INPUT_MASK_B 0x02   // B ボタン
#define INPUT_MASK_L 0x04   // 左
#define INPUT_MASK_R 0x08   // 右
#define INPUT_MASK_D 0x10   // 下
#define INPUT_MASK_U 0x20   // 上
#define INPUT_MASK_START  0x40  // Start/Pause/Resume
#define INPUT_MASK_CANCEL 0x80  // Cancel/GameOver
#define INPUT_MASK_NC   0x80  // ジョイスティックの場合, 未検出フラグになります
#define INPUT_SHIFT_NC  0x7   // ジョイスティックの場合, 未検出フラグになります

// input_joy_mode_ の取る値
#if 1
#define INPUT_JOY_MODE_AM7J_DETECTING       0   // AM7J   を検出中
#define INPUT_JOY_MODE_AM7J_DETECTED        20  // AM7J   モード
#define INPUT_JOY_MODE_AM7J_UNDETECTING     21  // AM7J   モードだがデータ異常
#define INPUT_JOY_MODE_MZ1X03_DETECTING     40  // MZ1X03 を検出中
#define INPUT_JOY_MODE_MZ1X03_DETECTED      60  // MZ1X03 モード
#define INPUT_JOY_MODE_MZ1X03_UNDETECTING   61  // MZ1X03 モードだがデータ異常
#define INPUT_JOY_MODE_MAX                  80
#else   // デバッグ用
#define INPUT_JOY_MODE_AM7J_DETECTING       0   // AM7J   を検出中
#define INPUT_JOY_MODE_AM7J_DETECTED        50  // AM7J   モード
#define INPUT_JOY_MODE_AM7J_UNDETECTING     51  // AM7J   モードだがデータ異常
#define INPUT_JOY_MODE_MZ1X03_DETECTING     100 // MZ1X03 を検出中
#define INPUT_JOY_MODE_MZ1X03_DETECTED      150 // MZ1X03 モード
#define INPUT_JOY_MODE_MZ1X03_UNDETECTING   151 // MZ1X03 モードだがデータ異常
#define INPUT_JOY_MODE_MAX                  200
#endif

// ---------------------------------------------------------------- システム
/** 入力の初期化を行います */
void inputInit(void) __z88dk_fastcall;
/** 入力生データを更新します. システムのほか, ポーズ時も使用 */
void inputMain(void) __z88dk_fastcall;

// ---------------------------------------------------------------- 入力
/** 入力生データを入手します. INPUT_MASK_XXXX の bitwise or */
inline u8 inputGet(void) { return input_; }
/** 入力 OFF->ON データを入手します. INPUT_MASK_XXXX の bitwise or */
inline u8 inputGetTrigger(void) { return input_trg_; }

/** ジョイスティックで検出した値を返します. 未検出の場合は 0x00 */
inline u8 inputGetJoy(void) { return input_joy_; }

/** ジョイスティック検出モード兼カウンタを返します */
inline u8 inputGetJoyMode(void) { return input_joy_mode_; }

// ---------------------------------------------------------------- MZ-1X13
/** 強制 MZ-1X03 モードにします(このモードでは自動検出しません) */
void inputSetMZ1X03Enabled(bool const b) __z88dk_fastcall;
/** 強制 MZ-1X03 モードかどうかを返します */
bool inputIsMZ1X03Enabled(void);

/** MZ-1X03 ボタンを読んで, VBLANK を待ち, 接続をテストして, 感度の鈍さの分だけ待って, 軸読み取りaをします
 * - この関数が呼んでる時点で /VBLK == 'H' (ブランク外) でなければなりません ('L' だったら assert します)
 */
void inputMZ1X03ButtonVSyncAxis1(const u8 mz1x03_sensitivity) __z88dk_fastcall __naked;
/** MZ-1X03 軸読み取りbをして, 結果を返します */
void inputMZ1X03Axis2(void) __naked;


/** MZ-1X03 感度(0鈍い～3敏感)を返します */
inline u8 inputGetMZ1X03sensitivity(void) { return input_mz1x03_sensitivity_; }
/** MZ-1X03 感度(0鈍い～3敏感)を +1 します */
inline void inputIncMZ1X03sensitivity(void)
{
    input_mz1x03_sensitivity_++;
    if (input_mz1x03_sensitivity_ > INPUT_MZ1X03_SENSITIVITY_MAX) {
        input_mz1x03_sensitivity_ = 0;
    }
}


#endif // INPUT_H_INCLUDED