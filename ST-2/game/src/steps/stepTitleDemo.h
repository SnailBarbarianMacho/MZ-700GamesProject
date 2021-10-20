/**
 * タイトル デモ ステップ
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef STEP_TITLE_DEMO_H_INCLUDED
#define STEP_TITLE_DEMO_H_INCLUDED

#include "../../../../src-common/common.h"

// -------------------------------- 文字列
extern const u8 stepTitleDemoStrNwkPresents[];

// ---------------------------------------------------------------- 初期化, メイン
void stepTitleDemoInit();
void stepTitleDemoMain(u16 stepCounter);

// ---------------------------------------------------------------- タイトル表示
void stepTitleDrawTitleS(     const u8* drawAddr);
void stepTitleDrawTitleT(     const u8* drawAddr);
void stepTitleDrawTitleHyphen(const u8* drawAddr);
void stepTitleDrawTitle2(     const u8* drawAddr);

#endif  // STEP_TITLE_H_INCLUDED
