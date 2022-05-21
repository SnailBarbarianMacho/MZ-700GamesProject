/**
 * タイトル シーン
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef SCENE_TITLE_H_INCLUDED
#define SCENE_TITLE_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- 初期化, メイン
void sceneTitleInit();
void sceneTitleMain(u16 scene_ct);

// ---------------------------------------------------------------- NWK Presents...
const u8* sceneTitleGetStrNwkPresents();

// ---------------------------------------------------------------- タイトル ロゴ表示
void sceneTitleDrawTitleS(     const u8* draw_addr);
void sceneTitleDrawTitleT(     const u8* draw_addr);
void sceneTitleDrawTitleHyphen(const u8* draw_addr);
void sceneTitleDrawTitle2(     const u8* draw_addr);


#endif  // SCENE_TITLE_H_INCLUDED
