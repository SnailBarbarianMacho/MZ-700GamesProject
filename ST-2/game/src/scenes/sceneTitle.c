/**
 * タイトル シーン
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
#include "../game/se.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "sceneInstruction.h"
#include "sceneGameMode.h"
#include "sceneTitleDemo.h"
#include "sceneTitle.h"

// ---------------------------------------------------------------- 変数, マクロ
extern u16 buildNr;

#define SYS_SCENE_WORK_START_CT 0

// ---------------------------------------------------------------- 初期化
void sceneTitleInit()
{
    objInit();
#if DEBUG
    static const u8 stepStr[] = {CHAR_T, CHAR_I, CHAR_T, CHAR_L, CHAR_E, 0};
    scoreSetStepString(stepStr);
#endif
    sysSetSceneCounter(300);
}

// ---------------------------------------------------------------- メイン
void sceneTitleMain(u16 sceneCounter)
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
    printSetAddr((u8*)VVRAM_TEXT_ADDR(3, 2)); printString(sceneTitleDemoStrNwkPresents);

    // -------- コピーライトとバージョン
    printSetAddr((u8*)VVRAM_TEXT_ADDR(13, 23)); printString(textTitlesnail);
    printSetAddr((u8*)VVRAM_TEXT_ADDR(27, 24)); printString(strVersion); printU16Left(buildNr);

    // -------- スタートの点滅
    u8 flash = sysSceneGetWork(SYS_SCENE_WORK_START_CT) ? 0x01 : 0x08;
    if (!(sceneCounter & flash)) {
        printSetAddr((u8*)VVRAM_TEXT_ADDR(8, 18)); printString(textTitlestart);
    }

    // -------- 宇宙船ボーナス案内
    printSetAddr((u8*)VVRAM_TEXT_ADDR( 6, 20)); printString(textTitlebonus);
    printU16Left(SCORE_BONUS_SHIP_1);                        printString(textTitlecomma);
    printU16Left(SCORE_BONUS_SHIP_1 + SCORE_BONUS_SHIP);     printString(textTitlecomma);
    printU16Left(SCORE_BONUS_SHIP_1 + SCORE_BONUS_SHIP * 2); printString(textTitlepts);

    // -------- タイトル表示
    sceneTitleDrawTitleS(     (u8*)VVRAM_TEXT_ADDR(7,  5));
    sceneTitleDrawTitleT(     (u8*)VVRAM_TEXT_ADDR(14, 5));
    sceneTitleDrawTitleHyphen((u8*)VVRAM_TEXT_ADDR(21, 8));
    sceneTitleDrawTitle2(     (u8*)VVRAM_TEXT_ADDR(26, 5));

    // -------- ゲームスタート
    if (sysSceneGetWork(SYS_SCENE_WORK_START_CT)) {
        sysSceneIncWork(SYS_SCENE_WORK_START_CT);
        if (30 < sysSceneGetWork(SYS_SCENE_WORK_START_CT)) {
           sysSetScene(sceneGameModeInit, sceneGameModeMain);
        }
    } else if (inputGetTrigger() & INPUT_MASK_P) {
        sdSetEnabled(true);
        sdPlaySe(SE_START);
        sysSetMode(true);
        scoreGameStart();
        sysSceneSetWork(SYS_SCENE_WORK_START_CT, 1);
    } else if (sceneCounter == 0)  {
        sysSetScene(sceneInstructionInit, sceneInstructionMain);
    }

}
