/**
 * エンディング シーン
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef SCENE_ENDING_H_INCLUDED
#define SCENE_ENDING_H_INCLUDED

#include "../../../../src-common/common.h"


// ---------------------------------------------------------------- 初期化, メイン
void sceneEndingInit();
void sceneEndingMain(u16 sceneCounter);

// ---------------------------------------------------------------- 統計結果表示
/** 「Finish」を表示します */
void sceneEndingDispFinish(const u8 atb) __z88dk_fastcall;
/** 「ゲーム結果」を表示します */
void sceneEndingDispGameResults(u8* const dispAddr) __z88dk_fastcall;
/** コンティニュー数を表示します */
void sceneEndingDispContinues(u8* const dispAddr) __z88dk_fastcall;
/** ミス回数を表示します */
void sceneEndingDispMisses(u8* const dispAddr) __z88dk_fastcall;
/** アイテム取得数を表示します */
void sceneEndingDispItems(u8* const dispAddr) __z88dk_fastcall;
/** 倒した敵の数を表示します */
void sceneEndingDispEnemies(u8* const dispAddr) __z88dk_fastcall;

#endif  // SCENE_ENDING_H_INCLUDED
