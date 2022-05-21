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
extern u8 input_;
extern u8 input_trg_;

// ---------------------------------------------------------------- マクロ
// inputGet(), inputGetTrigger() 戻値のビット マスク. AM7J(..UDRLBA) 互換です
#define INPUT_MASK_A 0x01   // A ボタン
#define INPUT_MASK_B 0x02   // B ボタン
#define INPUT_MASK_L 0x04   // 左
#define INPUT_MASK_R 0x08   // 右
#define INPUT_MASK_D 0x10   // 下
#define INPUT_MASK_U 0x20   // 上
#define INPUT_MASK_P 0x40   // Play
#define INPUT_MASK_S 0x80   // Select

// ---------------------------------------------------------------- システム
/** 入力の初期化を行います */
void inputInit() __z88dk_fastcall;
/** 入力生データを更新します. システムのほか, ポーズ時も使用 */
void inputMain() __z88dk_fastcall __naked;

// ---------------------------------------------------------------- 入力
/** 入力生データを入手します. INPUT_MASK_XXXX の bitwise or */
inline u8 inputGet() { return input_; }
/** 入力 OFF->ON データを入手します. INPUT_MASK_XXXX の bitwise or */
inline u8 inputGetTrigger() { return input_trg_; }

#endif // INPUT_H_INCLUDED