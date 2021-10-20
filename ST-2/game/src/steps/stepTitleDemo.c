/**
 * タイトル デモ ステップ
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/input.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/math.h"
#include "../system/obj.h"
#include "../game/score.h"
#include "stepTitle.h"
#include "stepTitleDemo.h"

// -------------------------------- マクロ
#define STEP_CT 120

// -------------------------------- 文字列
const u8 stepTitleDemoStrNwkPresents[] = {
    CHAR_N, CHAR_W, CHAR_K, CHAR_SP,
    CHAR_P, CHAR_CAPS, CHAR_R, CHAR_E, CHAR_S, CHAR_E, CHAR_N, CHAR_T, CHAR_S, 0,
};

// ---------------------------------------------------------------- 初期化
void stepTitleDemoInit()
{
    objInit();
#if DEBUG
    static const u8 stepStr[] = {CHAR_T, CHAR_D, CHAR_E, CHAR_M, CHAR_O, 0};
    scoreSetStepString(stepStr);
#endif
    sysSetStepCounter(STEP_CT);
}

// ---------------------------------------------------------------- メイン
void stepTitleDemoMain(u16 stepCounter)
{
    s16 ct = ((s16)STEP_CT - stepCounter) / 2 - 20;
    printSetAddr((u8*)VVRAM_TEXT_ADDR(3, 2)); printStringWithLength(stepTitleDemoStrNwkPresents, ct);

    // -------- タイトル表示
    stepTitleDrawTitleS(     (u8*)VVRAM_TEXT_ADDR(clampS16(stepCounter - 10 +  7,  7, VRAM_WIDTH), 5));
    stepTitleDrawTitleT(     (u8*)VVRAM_TEXT_ADDR(clampS16(stepCounter - 10 + 14, 14, VRAM_WIDTH), 5));
    stepTitleDrawTitleHyphen((u8*)VVRAM_TEXT_ADDR(clampS16(stepCounter - 10 + 21, 21, VRAM_WIDTH), 8));
    stepTitleDrawTitle2(     (u8*)VVRAM_TEXT_ADDR(clampS16(stepCounter - 10 + 26, 26, VRAM_WIDTH), 5));
    if (stepCounter == 0)  {
        sysSetStep(stepTitleInit, stepTitleMain);
    }
}

// ---------------------------------------------------------------- タイトル表示
#include "../../cg/Title_S.h"
#include "../../cg/Title_T.h"
#include "../../cg/Title_Hyphen.h"
#include "../../cg/Title_2.h"

void stepTitleDrawTitleS(const u8* drawAddr)
{
    vVramDrawRectTransparent(drawAddr, sTitle_S,      W8H8(CG_TITLE_S_WIDTH, CG_TITLE_S_HEIGHT));
}
void stepTitleDrawTitleT(const u8* drawAddr)
{
    vVramDrawRectTransparent(drawAddr, sTitle_T,      W8H8(CG_TITLE_T_WIDTH, CG_TITLE_T_HEIGHT));
}
void stepTitleDrawTitleHyphen(const u8* drawAddr)
{
    vVramDrawRectTransparent(drawAddr, sTitle_Hyphen, W8H8(CG_TITLE_HYPHEN_WIDTH, CG_TITLE_HYPHEN_HEIGHT));
}
void stepTitleDrawTitle2(const u8* drawAddr)
{
    vVramDrawRectTransparent(drawAddr, sTitle_2,      W8H8(CG_TITLE_2_WIDTH, CG_TITLE_2_HEIGHT));
}
