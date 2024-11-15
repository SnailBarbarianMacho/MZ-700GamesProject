/**
 * ゲーム オーバー シーン
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef SCENE_GAME_OVER_H_INCLUDED
#define SCENE_GAME_OVER_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- 初期化, メイン
void sceneGameOverInit(void);
void sceneGameOverInitWithoutReflectHiScore(void);
void sceneGameOverMain(u16 scene_ct);

#endif  // SCENE_GAME_OVER_H_INCLUDED
