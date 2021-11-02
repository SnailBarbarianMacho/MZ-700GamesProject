/**
 * スコア テーブル シーン
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef SCENE_SCORE_TABLE_H_INCLUDED
#define SCENE_SCORE_TABLE_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- private 変数. 直接触らない
extern u8 _sceneScoreTableLoopCt;

// ---------------------------------------------------------------- 初期化, メイン
void sceneScoreTableInit();
void sceneScoreTableMain(u16 sceneCounter);

// ---------------------------------------------------------------- デモ ループ カウンタ
/** デモ ループ カウンタを返します. 0～ */
inline u8 sceneScoreTableGetLoopCt() { return _sceneScoreTableLoopCt; }

#endif  // SCENE_SCORE_TABLE_H_INCLUDED
