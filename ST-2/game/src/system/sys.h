/**
 * システム
 * - 名前空間 SYS_ または sys
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef SYS_H_INCLUDED
#define SYS_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- システム
/** システムの初期化を行います
 * - 引数は @see sysSetStep() を参照してください
 */
void sysInit(void (*initFunc)(), void (*mainFunc)(u16));

/** 毎フレームの最初に呼んでください */
void sysMain(void) __z88dk_fastcall;

/** 毎フレーム + 1 されるシステム カウンタを返します */
u8 sysGetCounter() __z88dk_fastcall;

// ---------------------------------------------------------------- ゲーム モード
/** ゲーム モードをセットします. 初期値:false */
void sysSetGameMode(const bool bGameMode) __z88dk_fastcall;
/** 現在ゲームモードかセットします */
bool sysIsGameMode() __z88dk_fastcall;

// ---------------------------------------------------------------- ステップ
/** ステップを設定します
 * - サウンドはオフ (sdSetEnabled(false)) となります
 * - stepMain 系関数内からのみ呼んでください.
 *   オブジェクトからは呼んでは行けません(無限ループになります)
 * @param initFunc ステップ初期化関数
 * @param mainFunc 毎フレーム呼ばれる関数.
 * - 引数はステップカウンタで, 0 になったら次のステップへ移るようにしてください
 */
void sysSetStep(void (*initFunc)(), void (*mainFunc)(u16));
/**
 * ステップ カウンタを設定します.
 * 通常 sysSetStep() 内の initFunc() の中で呼ばれます
 */
void sysSetStepCounter(const u16 counter) __z88dk_fastcall;

#endif // SYS_H_INCLUDED