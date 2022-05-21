/**
 * スコア テーブル シーン
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef SCENE_SCORE_TABLE_H_INCLUDED
#define SCENE_SCORE_TABLE_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- private 変数. 直接触らない
extern u8 scene_score_tab_loop_ct_;

// ---------------------------------------------------------------- 初期化, メイン
void sceneScoreTableInit();
void sceneScoreTableMain(u16 scene_ct);

// ---------------------------------------------------------------- デモ ループ カウンタ
/** デモ ループ カウンタを返します. 0～ */
inline u8 sceneScoreTableGetLoopCt() { return scene_score_tab_loop_ct_; }

#endif  // SCENE_SCORE_TABLE_H_INCLUDED
