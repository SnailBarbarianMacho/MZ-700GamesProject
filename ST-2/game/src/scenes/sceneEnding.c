/**
 * エンディング シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../system/sys.h"
#include "../system/sound.h"
#include "../system/print.h"
#include "../system/vram.h"
#include "../system/math.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "../game/se.h"
#include "../game/gameMode.h"
#include "../game/stars.h"
#include "../objworks/objPlayer.h"
#include "../objworks/objEnemy.h"
#include "../objworks/objItem.h"
#include "sceneLogo.h"
#include "sceneGameMode.h"
#include "sceneEnding.h"

// ---------------------------------------------------------------- 変数, マクロ
#define ENDING_CT        200
#define ENDING_CT_FINISH 120
#define ENDING_CT_DEMO   80

#define SYS_SCENE_WORKSTR_INDEX 0

// ---------------------------------------------------------------- 初期化
void sceneEndingInit()
{
#if DEBUG
    static const u8 sceneStr[] = {CHAR_E, CHAR_N, CHAR_D, 0};
    scoreSetStepString(sceneStr);
#endif
    // プレーヤー去る
    Obj* pPlayer = objGetInUsePlayer();
    pPlayer->step          = OBJ_PLAYER_STEP_DEMO;
    pPlayer->uGeo.geo.sx   =  0x0000;
    pPlayer->uGeo.geo.sy   = -0x0080;

    objInitItem(); // アイテムは全部消す

    sdSetEnabled(true);
    sdPlaySe(SE_END);
    sysSetSceneCounter(ENDING_CT);
}


// ---------------------------------------------------------------- メイン
void sceneEndingMain(u16 sceneCounter)
{
    if (ENDING_CT_FINISH < sceneCounter) {
        sceneEndingDispFinish(VATB(sceneCounter & 0x07, 0, 0));
    } else if (sceneCounter <= ENDING_CT_DEMO) { // 文字は 1 フレーム先から表示
        starsInit();
#define L 16
#include "../../music/ending_bar8.h"
        static const u8* sdTab0[] = { nullptr, mml0_0, mml0_1, mml0_2, mml0_3, mml0_4, mml0_5, };
        static const u8* sdTab1[] = { nullptr, mml1_0, mml1_1, mml1_2, mml1_3, mml1_4, mml1_5, };
        static const u8* sdTab2[] = { nullptr, mml2_0, mml2_1, mml2_2, mml2_3, mml2_4, mml2_5, };

#include "../../text/ending1.h"
#include "../../text/ending2.h"
#include "../../text/ending3.h"
#include "../../text/ending4.h"
#include "../../text/ending5.h"
#include "../../text/ending6.h"
        static const u8* strTab[] = { textEnding1, textEnding2, textEnding3, textEnding4, textEnding5, textEnding6, nullptr, };
        u8 strIdx = sysSceneGetWork(SYS_SCENE_WORKSTR_INDEX);
        if (strIdx < COUNT_OF(strTab)) {
            if (sdTab0[strIdx]) {
#if DEBUG
                sd3Play(sdTab0[strIdx], sdTab1[strIdx], sdTab2[strIdx], true);
#else
                sd3Play(sdTab0[strIdx], sdTab1[strIdx], sdTab2[strIdx], false);// エンディングBGMはスキップできない
#endif
            }

            if (strTab[strIdx]) {
                printSetAddr((u8*)VVRAM_TEXT_ADDR(6, 5));
                printString(strTab[strIdx]);
                if (strIdx == 5) {
                    sceneEndingDispGameResults((u8*)VVRAM_TEXT_ADDR(10, 9));
                    sceneGamePrintGameMode(    (u8*)VVRAM_TEXT_ADDR(15, 11), gameGetMode(), false);
                    sceneEndingDispContinues(  (u8*)VVRAM_TEXT_ADDR(10, 15));
                    sceneEndingDispMisses(     (u8*)VVRAM_TEXT_ADDR(10, 17));
                    sceneEndingDispItems(      (u8*)VVRAM_TEXT_ADDR(10, 19));
                    sceneEndingDispEnemies(    (u8*)VVRAM_TEXT_ADDR(10, 21));
                }
            } else {
                sdPlaySe(SE_END_GAME_OVER);
                sysSetMode(false);
            }
            sysSceneIncWork(SYS_SCENE_WORKSTR_INDEX);
        }
    }

    if (sceneCounter == 0)  {
        scoreReflectHiScore();
        scoreResetLevel();
        sysSetScene(sceneLogoInit, sceneLogoMain);
        //sysSetScene(sceneEndingInit, sceneEndingMain);//TEST
    }
}

// ---------------------------------------------------------------- 統計結果表示
void sceneEndingDispFinish(const u8 atb) __z88dk_fastcall
{
#include "../../text/finish.h"
    printSetAtb(atb);
    printSetAddr((u8*)VVRAM_TEXT_ADDR(16, 7));
    printString(textFinish);
}

void sceneEndingDispGameResults(u8* const dispAddr) __z88dk_fastcall
{
#include "../../text/gameResults.h"
    printSetAddr(dispAddr);
    printString(textGameResults);
}

void sceneEndingDispContinues(u8* const dispAddr) __z88dk_fastcall
{
#include "../../text/ending6continue.h"
    printSetAddr(dispAddr);
    printString(textEnding6continue);
    printU16Left(scoreGetNrContinues());
}

void sceneEndingDispMisses(u8* const dispAddr) __z88dk_fastcall
{
#include "../../text/ending6miss.h"
    printSetAddr(dispAddr);
    printString(textEnding6miss);
    printU16Left(scoreGetNrMisses());
}

void sceneEndingDispItems(u8* const dispAddr) __z88dk_fastcall
{
#include "../../text/ending6items.h"
    printSetAddr(dispAddr);
    printString(textEnding6items);
    u16 nrGenItems = objItemGetNrGenereted();// 生成アイテム数は必ず 0 以上になります
    u16 nrObtItems = objItemGetNrObtained();
    //nrGenItems = 7400;//TEST
    //nrObtItems = 6000;//TEST
    printU16Left(nrObtItems);
    printPutc(CHAR_SLASH);
    printU16Left(nrGenItems);
    printPutc(CHAR_EQUALS);
    nrGenItems = (u32)nrObtItems * 100 / nrGenItems;// div32 は使いたくないが, やむを得なん!!
    printU16Left(nrGenItems);
    printPutc(CHAR_PERCENT);
}

void sceneEndingDispEnemies(u8* const dispAddr) __z88dk_fastcall
{
#include "../../text/ending6enemies.h"
    printSetAddr(dispAddr);
    printString(textEnding6enemies);
    u16 nrKilled = objEnemyGetNrKilled();
    printU16Left(nrKilled);
}
