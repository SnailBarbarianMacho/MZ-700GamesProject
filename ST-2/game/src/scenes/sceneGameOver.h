/**
 * ゲーム オーバー シーン
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef SCENE_GAME_OVER_H_INCLUDED
#define SCENE_GAME_OVER_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- 初期化, メイン
void sceneGameOverInit();
void sceneGameOverInitWithoutReflectHiScore();
void sceneGameOverMain(u16 sceneCounter);

#endif  // SCENE_GAME_OVER_H_INCLUDED
