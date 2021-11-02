/**
 * ステージ クリア シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/sound.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "sceneGame.h"
#include "sceneStageClear.h"

// ---------------------------------------------------------------- 初期化
void sceneStageClearInit()
{
#if DEBUG
    static const u8 stepStr[] = {CHAR_S, CHAR_C, CHAR_L, CHAR_R, 0};
    scoreSetStepString(stepStr);
#endif
    sdSetEnabled(true);
    sysSetSceneCounter(100);
}

#if 0
//  NICE FIGHT!
// * GOOD JOB! *
// YOU MADE IT!
// ** GREAT! **
//  YOU DID IT!
// *** xxxx! *** やったね!
// ** xxxxxx **  すげぇぜ
//  xxxx xx xxx  ごくろう で あった
//BONUS xxxx Pts
static const u8 strNiceFight[] = { CHAR_SP, CHAR_N, CHAR_I, CHAR_C, CHAR_E, CHAR_SP, CHAR_F, CHAR_I, CHAR_G, CHAR_H, CHAR_T, CHAR_EXCLAMATION, 0, };
static const u8 strGoodJob[]   = { CHAR_DIAMOND, CHAR_SP, CHAR_G, CHAR_O, CHAR_O, CHAR_D, CHAR_SP, CHAR_J, CHAR_O, CHAR_B, CHAR_EXCLAMATION, CHAR_SP, CHAR_DIAMOND, 0, };
static const u8 strYouMadeIt[] = { CHAR_Y, CHAR_O, CHAR_U, CHAR_SP, CHAR_M, CHAR_A, CHAR_D, CHAR_E, CHAR_SP, CHAR_I, CHAR_T, CHAR_EXCLAMATION, 0, };
static const u8 strGreat[]     = { CHAR_HEART, CHAR_HEART, CHAR_SP, CHAR_G, CHAR_R, CHAR_E, CHAR_A, CHAR_T, CHAR_EXCLAMATION, CHAR_SP, CHAR_HEART, CHAR_HEART, 0, };
static const u8 strYouDidIt[]  = { CHAR_SP, CHAR_Y, CHAR_O, CHAR_U, CHAR_SP, CHAR_D, CHAR_I, CHAR_D, CHAR_SP, CHAR_I, CHAR_T, CHAR_EXCLAMATION, 0, };
static const u8 strYattane[]   = { CHAR_UFO, CHAR_UFO, CHAR_UFO, CHAR_SP, CHAR_CAPS, CHAR_KANA_YA, CHAR_KANA_XTSU, CHAR_KANA_TA, CHAR_KANA_NE, CHAR_SP, CHAR_UFO, CHAR_UFO, CHAR_UFO, 0, };
static const u8 strSugeeze[]   = { CHAR_NICOCHAN_0, CHAR_NICOCHAN_0, CHAR_SP, CHAR_CAPS, CHAR_KANA_SU, CHAR_KANA_GE, CHAR_KANA_XE, CHAR_KANA_ZE, CHAR_SP, CHAR_NICOCHAN_0, CHAR_NICOCHAN_0, 0, };
static const u8 strGokurou[]   = { CHAR_SP, CHAR_CAPS, CHAR_KANA_GO, CHAR_KANA_KU, CHAR_KANA_RO, CHAR_KANA_U, CHAR_KANA_DE, CHAR_SP, CHAR_KANA_A, CHAR_KANA_XTSU, CHAR_KANA_TA, 0, };
static const u8* const strs[] = {
    strNiceFight, strGoodJob, strYouMadeIt, strGreat,
    strYouDidIt, strYattane, strSugeeze, strGokurou,
};
#endif
#include "../../text/clear1.h"
#include "../../text/clear2.h"
#include "../../text/clear3.h"
#include "../../text/clear4.h"
#include "../../text/clear5.h"
#include "../../text/clear6.h"
#include "../../text/clear7.h"
#include "../../text/clear8.h"
static const u8* const strs[] = { textClear1, textClear2, textClear3, textClear4, textClear5, textClear6, textClear7, textClear8, };

// ---------------------------------------------------------------- メイン
void sceneStageClearMain(u16 sceneCounter)
{
    if (sceneCounter == 31) {
        u8  stgNr = stgGetStageNr() - 2; // ステージ番号は既に +1 されている ので stgNr は 0, 1, ...
        u16 bonus = ((stgNr / 2) + 1) * 100;

        scoreAdd(bonus);
        static const u8 str1[] = { CHAR_B, CHAR_O, CHAR_N, CHAR_U, CHAR_S, CHAR_SP, 0, };
        static const u8 str2[] = { CHAR_0, CHAR_SP, CHAR_P, CHAR_CAPS, CHAR_T, CHAR_S, 0, };
        stgNr = stgNr & 0x07;
        printSetAddr((u8*)VVRAM_TEXT_ADDR(14, 6)); printSetAtb(VATB(6, 0, 0)); printString(strs[stgNr]);
        printSetAddr((u8*)VVRAM_TEXT_ADDR(13, 8)); printSetAtb(VATB(7, 0, 0)); printString(str1); printU16Left(bonus); printString(str2);
    } else if (sceneCounter == 30) {
#define L   10
#include "../../music/clear.h"
        sd3Play(mml0_0, mml1_0, mml2_0, true);

    } else if (sceneCounter == 0)  {
        sysSetScene(sceneGameInit, sceneGameMain);
    }
}