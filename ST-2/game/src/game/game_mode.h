/**
 * ゲーム モード
 * - 名前空間 GAME_ および game
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef GAME_MODE_H_INCLUDED
#define GAME_MODE_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- private 変数. 直接触らない
extern u8   game_mode_;
extern u8   game_hard_;
extern bool b_game_caravan_;
extern bool b_game_inc_left_;
extern u16  game_caravan_timer_;

// ---------------------------------------------------------------- マクロ

// ゲームモード
#define GAME_MODE_EASY     0    //< イージー
#define GAME_MODE_NORMAL   1    //< ノーマル
#define GAME_MODE_HARD     2    //< ハード
#define GAME_MODE_SURVIVAL 3    //< サバイバル
#define GAME_MODE_CARAVAN  4    //< キャラバン
#define NR_GAME_MODES      5

#define GAME_FPS           32   // 正確ではないですが, 32 フレームで1秒としときます

// ---------------------------------------------------------------- Setter/Getter
/** ゲームモードをセットします
 * - 同時に, 難易度,スコア カラー, キャラバン タイマーが変わります
 * @param game_mode
 */
void gameSetMode(const u8 game_mode) __z88dk_fastcall;

/** ゲームモードを返します */
inline u8 gameGetMode() { return game_mode_; }
/** 難易度がハードなら 0 以外を返します. アイテム増加数になります */
inline u8 gameHard() { return game_hard_; }
/** キャラバンモードかを返します */
inline bool gameIsCaravan() { return b_game_caravan_; }
/** 自機がスコアで増えるかどうかを返します */
inline bool gameIsIncLeft() { return b_game_inc_left_; }
/** キャラバン タイマーを返します */
inline u16 gameGetCaravanTimer() { return game_caravan_timer_; }
/** キャラバン タイマーをデクリメントします */
u16 gameDecCaravanTimer();

#endif // GAME_MODE_H_INCLUDED
