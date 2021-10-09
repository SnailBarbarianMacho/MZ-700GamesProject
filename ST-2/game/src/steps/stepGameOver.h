/**
 * ゲーム オーバー ステップ
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef STEP_GAME_OVER_H_INCLUDED
#define STEP_GAME_OVER_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- 初期化, メイン
void stepGameOverInit();
void stepGameOverInitWithoutReflectHiScore();
void stepGameOverMain(u16 stepCounter);

#endif  // STEP_GAME_OVER_H_INCLUDED
