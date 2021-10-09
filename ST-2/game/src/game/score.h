/**
 * スコア, ハイスコア, レベル, 残機
 * - 名前空間 SCORE_ または score
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef SCORE_H_INCLUDED
#define SCORE_H_INCLUDED

#include "../../../../src-common/common.h"

#define SCORE_BONUS_SHIP_1 500    // 最初の残機が増える得点
#define SCORE_BONUS_SHIP   1000   // 以降残機が増える得点

// ---------------------------------------------------------------- 初期化, メイン
/** スコア等の初期化 */
void scoreInit(void) __z88dk_fastcall;
/** スコア等を描画します */
void scoreMain(void) __z88dk_fastcall;

// ---------------------------------------------------------------- 制御
/** スコア等の描画を停止します. 次のフレームでは許可されます */
void scoreSetDisabled()__z88dk_fastcall;

// ---------------------------------------------------------------- スタート, コンティニュー
/** スコアを 0 点に初期化します. アトラクト モード時は初期化しません */
void scoreGameStart()__z88dk_fastcall;
/** スコアを 0 点に初期化します */
void scoreContinue()__z88dk_fastcall;
/** コンティニュー回数 */
u16 scoreGetNrContinues()__z88dk_fastcall;

// ---------------------------------------------------------------- スコア, ハイ スコア
/** スコアを加算します. アトラクト モード時は加算しません
 * @return 残機が増えたなら true
 */
bool scoreAdd(const u16 score)__z88dk_fastcall;

/** スコアをハイ スコアに反映します
 * @return ハイ スコア更新ならば true
 */
bool scoreReflectHiScore()__z88dk_fastcall;

// ---------------------------------------------------------------- 残機
/** 残機を減らします
 * @return 0 の時に呼ぶと true. アトラクトモードの時は, 減算しないで常に false
 */
bool scoreDecrementLeft()__z88dk_fastcall;

// ---------------------------------------------------------------- レベル
/** サブ レベルを増加します
 * サブ レベルが 4 * 4 * 7 になったらレベル アップ
 * @param subLevel
 */
void scoreAddSubLevel(const u8 subLevel)__z88dk_fastcall;
/** レベルを返します */
u8 scoreGetLevel()__z88dk_fastcall;
/** レベルをリセットします */
void scoreResetLevel();

// ---------------------------------------------------------------- デバッグ
#if DEBUG
/** 現在のステップを表示します */
void scoreSetStepString(const u8* const str)__z88dk_fastcall;
#endif


#endif // SCORE_H_INCLUDED
