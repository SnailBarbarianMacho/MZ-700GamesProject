/**
 * タイトル ステップ
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/version.h"
#include "../system/input.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/math.h"
#include "../system/sound.h"
#include "../system/obj.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "stepInstruction.h"
#include "stepOpening.h"
#include "stepTitleDemo.h"
#include "stepTitle.h"

extern u16 buildNr;
u8 sStartCounter;


// ---------------------------------------------------------------- サウンド
#define SE_START_CT  48
// スタート音
static void seStart(u8 ct)
{
    sdMake(((ct & 3) + (ct / 4) + 1) << 9);
}


// ---------------------------------------------------------------- 初期化
void stepTitleInit()
{
    objInit();
#if DEBUG
    static const u8 stepStr[] = {CHAR_T, CHAR_I, CHAR_T, CHAR_L, CHAR_E, 0};
    scoreSetStepString(stepStr);
#endif
    sysSetStepCounter(300);
    sStartCounter = 0;
}


// ---------------------------------------------------------------- メイン
void stepTitleMain(u16 stepCounter)
{
#include "../../text/titlesnail.h"
#include "../../text/titlestart.h"
#include "../../text/titlebonus.h"
#include "../../text/titlecomma.h"
#include "../../text/titlepts.h"
    static const u8 strVersion[] = {
        CHAR_V, CHAR_CAPS, CHAR_E, CHAR_R, CHAR_PERIOD,
        CHAR_0 + VER_MAJOR, CHAR_PERIOD, CHAR_0 + VER_MINOR0, CHAR_0 + VER_MINOR1, CHAR_PERIOD, 0
    };

    // -------- NWK Presents
    printSetAddr((u8*)VVRAM_TEXT_ADDR(3, 2)); printString(stepTitleDemoStrNwkPresents);

    // -------- コピーライトとバージョン
    printSetAddr((u8*)VVRAM_TEXT_ADDR(13, 23)); printString(str_titlesnail);
    printSetAddr((u8*)VVRAM_TEXT_ADDR(27, 24)); printString(strVersion); printU16Left(buildNr);

    // -------- P ボタンでゲーム開始
    u8 flash = (sStartCounter) ? 0x01 : 0x08;
    if (!(stepCounter & flash)) {
        printSetAddr((u8*)VVRAM_TEXT_ADDR(8, 18)); printString(str_titlestart);
    }

    // -------- 宇宙船ボーナス案内
    printSetAddr((u8*)VVRAM_TEXT_ADDR( 6, 20)); printString(str_titlebonus);
    printU16Left(SCORE_BONUS_SHIP_1);                        printString(str_titlecomma);
    printU16Left(SCORE_BONUS_SHIP_1 + SCORE_BONUS_SHIP);     printString(str_titlecomma);
    printU16Left(SCORE_BONUS_SHIP_1 + SCORE_BONUS_SHIP * 2); printString(str_titlepts);

    // -------- タイトル表示
    stepTitleDrawTitleS(     (u8*)VVRAM_TEXT_ADDR(7,  5));
    stepTitleDrawTitleT(     (u8*)VVRAM_TEXT_ADDR(14, 5));
    stepTitleDrawTitleHyphen((u8*)VVRAM_TEXT_ADDR(21, 8));
    stepTitleDrawTitle2(     (u8*)VVRAM_TEXT_ADDR(26, 5));

    // -------- ゲームスタート
    if (sStartCounter) {
        sStartCounter++;
        if (30 < sStartCounter) {
           sysSetStep(stepOpeningInit, stepOpeningMain);
        }
    } else if (inputGetTrigger() & INPUT_MASK_P) {
        sdSetEnabled(true);
        sdSetSeSequencer(seStart, SD_SE_PRIORITY_0, SE_START_CT);
        sysSetGameMode(true);
        scoreGameStart();
        stgInit(0);
        sStartCounter = 1;
    } else if (stepCounter == 0)  {
        sysSetStep(stepInstructionInit, stepInstructionMain);
    }

}
