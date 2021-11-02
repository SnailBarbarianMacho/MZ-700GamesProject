/**
 * タイトル デモ シーン
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef SCENE_TITLE_DEMO_H_INCLUDED
#define SCENE_TITLE_DEMO_H_INCLUDED

#include "../../../../src-common/common.h"

// -------------------------------- 文字列
extern const u8 sceneTitleDemoStrNwkPresents[];

// ---------------------------------------------------------------- 初期化, メイン
void sceneTitleDemoInit();
void sceneTitleDemoMain(u16 sceneCounter);

// ---------------------------------------------------------------- タイトル表示
void sceneTitleDrawTitleS(     const u8* drawAddr);
void sceneTitleDrawTitleT(     const u8* drawAddr);
void sceneTitleDrawTitleHyphen(const u8* drawAddr);
void sceneTitleDrawTitle2(     const u8* drawAddr);

#endif  // SCENE_TITLE_H_INCLUDED
