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
extern u16  game_timer_;
extern u8   game_subtimer_;

// ---------------------------------------------------------------- マクロ

// ゲーム モード
#define GAME_MODE_EASY     0    //< イージー
#define GAME_MODE_NORMAL   1    //< ノーマル
#define GAME_MODE_HARD     2    //< ハード
#define GAME_MODE_SURVIVAL 3    //< サバイバル
#define GAME_MODE_MUBO     4    //< むぼう
#define GAME_MODE_CARAVAN  5    //< キャラバン
#define GAME_MODE_MZ1X03_INSENSITIVITY 6 //< MZ1X03 感度の鈍さ
#define NR_GAME_MODES      7

#define GAME_FPS           32   // 正確ではないですが, 32 フレームで1秒としときます

// ---------------------------------------------------------------- Setter/Getter
/** ゲーム モードをセットします
 * - 同時に, 難易度,スコア カラー, キャラバン タイマーが変わります
 * - 不正な値を入れても変なゲーム設定になるだけです
 * @param game_mode
 */
void gameSetMode(const u8 game_mode) __z88dk_fastcall;

/** ゲームモードを返します */
inline u8 gameGetMode(void) { return game_mode_; }
/** 難易度がハードなら 0 以外を返します. アイテム増加数になります */
inline u8 gameHard(void) { return game_hard_; }
/** キャラバンモードかを返します */
inline bool gameIsCaravan(void) { return b_game_caravan_; }
/** 自機がスコアで増えるかどうかを返します (サバイバル, むぼう, キャラバンなら false) */
inline bool gameCanIncLeft(void) { return b_game_inc_left_; }
/** キャラバン, サバイバル, むぼう ゲーム モードで使うタイマー値を返します */
inline u16 gameGetTimer(void) { return game_timer_; }
/** サバイバル, むぼう ゲーム モードで使うサブタイマー値を返します(0～31) */
inline u8 gameGetSubtimer(void) { return game_subtimer_; }
/** タイマーをデクリメントし, その値を返します. 必要に応じてチャイム鳴らします(キャラバンモード用) */
u16 gameDecTimer(void);
/** タイマーとサブタイマーをインクリメントします(サバイバル, むぼう用) */
void gameIncTimer(void);

#endif // GAME_MODE_H_INCLUDED
