/**
 * ゲーム モード選択 シーン
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef SCENE_GAME_MODE_H_INCLUDED
#define SCENE_GAME_MODE_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- 初期化, メイン
void sceneGameModeInit();
void sceneGameModeMain(u16 sceneCounter);

// ---------------------------------------------------------------- print
/** ゲームモード名(と説明)を描画します */
void sceneGamePrintGameMode(u8* const addr, const u8 gameMode, const bool bWithExplanation);

#endif  // SCENE_GAME_MODE_H_INCLUDED
