/**
 * スコア, ハイスコア, レベル, 残機, ゲームモード+ルール
 * - 名前空間 SCORE_ および score
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef SCORE_H_INCLUDED
#define SCORE_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- private 変数. 直接触らない
extern u16  score_;
extern u8   score_level_;
extern bool b_score_enabled_;
extern u16  score_nr_continues_;
extern u16  score_nr_misses_;
#if DEBUG
extern const u8* score_step_str_;
#endif

// ---------------------------------------------------------------- マクロ
#define SCORE_BONUS_SHIP_1 500  //< 最初の残機が増える得点
#define SCORE_BONUS_SHIP   1000 //< 以降残機が増える得点

// ---------------------------------------------------------------- 初期化, メイン
/** スコア等の初期化 */
void scoreInit(void) __z88dk_fastcall;
/** スコア等を描画します */
void scoreMain(void) __z88dk_fastcall;

// ---------------------------------------------------------------- 制御
/** スコア等の描画を停止します. 次のフレームでは許可されます */
inline void scoreSetDisabled(void) { b_score_enabled_ = false; }

// ---------------------------------------------------------------- スタート, コンティニュー
/** スコアを 0 点に初期化します. アトラクト モード時は初期化しません */
void scoreGameStart(void)__z88dk_fastcall;
/** スコアを 0 点に初期化します */
void scoreContinue(void)__z88dk_fastcall;
/** コンティニュー回数 */
inline u16 scoreGetNrContinues(void) { return score_nr_continues_; }
/** ミス回数 */
inline u16 scoreGetNrMisses(void) { return score_nr_misses_; }

// ---------------------------------------------------------------- スコア, ハイ スコア
/** スコアを返します */
inline u16 scoreGet(void) { return score_; }

/** スコアを加算します. アトラクト モード時は加算しません
 * @return 残機が増えたなら true
 */
bool scoreAdd(const u16 score)__z88dk_fastcall;

/** スコアをハイ スコアに反映します
 * @return ハイ スコア更新ならば true
 */
bool scoreReflectHiScore(void)__z88dk_fastcall;

// ---------------------------------------------------------------- 残機
/** 残機を減らします
 * @return 0 の時に呼ぶと true. アトラクトモードの時は, 減算しないで常に false
 */
bool scoreDecrementLeft(void)__z88dk_fastcall;

// ---------------------------------------------------------------- レベル
/** サブ レベルを増加します
 * サブ レベルが 4 * 4 * 7 になったらレベル アップ
 * @param sub_level
 */
void scoreAddSubLevel(const u8 sub_level)__z88dk_fastcall;
/** レベルを返します */
inline u8 scoreGetLevel(void) { return score_level_; }
/** レベルをリセットします */
inline void scoreResetLevel(void) { score_level_ = 1; }


// ---------------------------------------------------------------- デバッグ
#if DEBUG
/** 現在のシーンを表示します */
inline void scoreSetStepString(const u8* const str) { score_step_str_ = str; }
#endif

#endif // SCORE_H_INCLUDED
