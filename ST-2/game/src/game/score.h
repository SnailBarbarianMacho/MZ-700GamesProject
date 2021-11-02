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

extern u16  _score;
extern u8   _scoreLevel;
extern bool _bScoreEnabled;
extern u16  _scoreNrContinues;
extern u16  _scoreNrMisses;
#if DEBUG
extern const u8* _scoreStepStr;
#endif

// ---------------------------------------------------------------- マクロ
#define SCORE_BONUS_SHIP_1 500  //< 最初の残機が増える得点
#define SCORE_BONUS_SHIP   1000 //< 以降残機が増える得点

// ---------------------------------------------------------------- 初期化, メイン
/** スコア等の初期化 */
void scoreInit() __z88dk_fastcall;
/** スコア等を描画します */
void scoreMain() __z88dk_fastcall;

// ---------------------------------------------------------------- 制御
/** スコア等の描画を停止します. 次のフレームでは許可されます */
inline void scoreSetDisabled() { _bScoreEnabled = false; }

// ---------------------------------------------------------------- スタート, コンティニュー
/** スコアを 0 点に初期化します. アトラクト モード時は初期化しません */
void scoreGameStart()__z88dk_fastcall;
/** スコアを 0 点に初期化します */
void scoreContinue()__z88dk_fastcall;
/** コンティニュー回数 */
inline u16 scoreGetNrContinues() { return _scoreNrContinues; }
/** ミス回数 */
inline u16 scoreGetNrMisses() { return _scoreNrMisses; }

// ---------------------------------------------------------------- スコア, ハイ スコア
/** スコアを返します */
inline u16 scoreGet() { return _score; }

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
inline u8 scoreGetLevel() { return _scoreLevel; }
/** レベルをリセットします */
inline void scoreResetLevel() { _scoreLevel = 1; }


// ---------------------------------------------------------------- デバッグ
#if DEBUG
/** 現在のシーンを表示します */
inline void scoreSetStepString(const u8* const str) { _scoreStepStr = str; }
#endif

#endif // SCORE_H_INCLUDED
