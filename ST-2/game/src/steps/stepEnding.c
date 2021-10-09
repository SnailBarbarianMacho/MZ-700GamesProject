/**
 * エンディング ステップ
 * @author Snail Barbarian Macho (NWK)
 */

#include "../system/sys.h"
#include "../system/sound.h"
#include "../system/print.h"
#include "../system/vram.h"
#include "../system/math.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "../game/stars.h"
#include "../objworks/objPlayer.h"
#include "../objworks/objItem.h"
#include "stepLogo.h"
#include "stepEnding.h"

// ---------------------------------------------------------------- 変数, マクロ
static u8 sStrIndex;
static u8 sPlayIndex;
#define ENDING_CT        200
#define ENDING_CT_FINISH 120
#define ENDING_CT_DEMO   80

// ---------------------------------------------------------------- サウンド
#define SE_END_CT        128
#define SE_GAME_OVER_CT  70
static void seEnd(u8 ct)
{
    sdMake(((ct & 31) + (ct / 32) + 1) << 9);
}

#pragma disable_warning 85
#pragma save
static void seGameOver(u8 ct)
{
    sdMake(rand8() << 7);
}
#pragma restore

// ---------------------------------------------------------------- 初期化
void stepEndingInit()
{
#if DEBUG
    static const u8 stepStr[] = {CHAR_E, CHAR_N, CHAR_D, 0};
    scoreSetStepString(stepStr);
#endif
    // プレーヤー去る
    Obj* pPlayer = objGetInUsePlayer();
    pPlayer->step          = OBJ_PLAYER_STEP_DEMO;
    pPlayer->uGeo.geo.sx   =  0x0000;
    pPlayer->uGeo.geo.sy   = -0x0080;

    objInitItem(); // アイテムは全部消す

    sStrIndex = 0;
    sPlayIndex = 0;

    sdSetEnabled(true);
    sdSetSeSequencer(seEnd, SD_SE_PRIORITY_0, SE_END_CT);
    sysSetStepCounter(ENDING_CT);
}


// ---------------------------------------------------------------- メイン
void stepEndingMain(u16 stepCounter)
{
    if (ENDING_CT_FINISH < stepCounter) {
#include "../../text/finish.h"
        printSetAtb(VATB(stepCounter & 0x07, 0, 0));
        printSetAddr((u8*)VVRAM_TEXT_ADDR(16, 7));
        printString(str_finish);
    } else if (stepCounter <= ENDING_CT_DEMO) { // 文字は 1 フレーム先から表示
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
#include "../../text/ending6_2.h"
        static const u8* strTab[] = { str_ending1, str_ending2, str_ending3, str_ending4, str_ending5, str_ending6, nullptr, };
        if (sStrIndex < COUNT_OF(strTab)) {
            if (sdTab0[sStrIndex]) {
#if DEBUG
                sd3Play(sdTab0[sStrIndex], sdTab1[sStrIndex], sdTab2[sStrIndex], true);
#else
                sd3Play(sdTab0[sStrIndex], sdTab1[sStrIndex], sdTab2[sStrIndex], false);// エンディングBGMはスキップできない
#endif
            }

            if (strTab[sStrIndex]) {
                printSetAddr((u8*)VVRAM_TEXT_ADDR(6, 5));
                printString(strTab[sStrIndex]);
                if (sStrIndex == 5) {
                    printU16Left(scoreGetNrContinues());
                    printSetAddr((u8*)VVRAM_TEXT_ADDR(6, 17));
                    printString(str_ending6_2);
                    u16 nrGenItems = objItemGetNrGeneretedItems();// 生成アイテム数は必ず 0 以上になります
                    u16 nrObtItems = objItemGetNrObtainedItems();
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
            } else {
                sdSetSeSequencer(seGameOver, SD_SE_PRIORITY_0, SE_GAME_OVER_CT);
                sysSetGameMode(false);
            }
            sStrIndex++;
        }
    }

    if (stepCounter == 0)  {
        scoreReflectHiScore();
        scoreResetLevel();
        sysSetStep(stepLogoInit, stepLogoMain);
        //sysSetStep(stepEndingInit, stepEndingMain);//TEST
    }
}