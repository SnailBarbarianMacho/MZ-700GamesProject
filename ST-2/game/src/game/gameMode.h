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
extern u8   _gameMode;
extern u8   _gameHard;
extern bool _bGameCaravan;
extern bool _bGameIncLeft;
extern u16  _gameCaravanTimer;

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
 * @param gameMode
 */
void gameSetMode(const u8 gameMode) __z88dk_fastcall;

/** ゲームモードを返します */
inline u8 gameGetMode() { return _gameMode; }
/** 難易度がハードなら 0 以外を返します. アイテム増加数になります */
inline u8 gameHard() { return _gameHard; }
/** キャラバンモードかを返します */
inline bool gameIsCaravan() { return _bGameCaravan; }
/** 自機がスコアで増えるかどうかを返します */
inline bool gameIsIncLeft() { return _bGameIncLeft; }
/** キャラバン タイマーを返します */
inline u16 gameGetCaravanTimer() { return _gameCaravanTimer; }
/** キャラバン タイマーをデクリメントします */
u16 gameDecCaravanTimer();

#endif // GAME_MODE_H_INCLUDED
