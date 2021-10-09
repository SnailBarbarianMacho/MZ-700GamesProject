/**
 * スコア テーブル ステップ
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef STEP_SCORE_TABLE_H_INCLUDED
#define STEP_SCORE_TABLE_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- 初期化, メイン
void stepScoreTableInit();
void stepScoreTableMain(u16 stepCounter);


// ---------------------------------------------------------------- デモ ループ カウンタ
/** デモ ループ カウンタを返します. 0～ */
u8 stepScoreTableGetLoopCt();

#endif  // STEP_SCORE_TABLE_H_INCLUDED
