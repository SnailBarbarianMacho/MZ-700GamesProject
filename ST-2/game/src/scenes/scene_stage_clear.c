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
#include "scene_game.h"
#include "scene_stage_clear.h"

// ---------------------------------------------------------------- 初期化
void sceneStageClearInit()
{
#if DEBUG
    static const u8 str[] = { DC_S, DC_C, DC_L, DC_R, 0, };
    scoreSetStepString(str);
#endif
    sdSetEnabled(true);
    sysSetSceneCounter(100);
}

#include "../../text/clear1.h"
#include "../../text/clear2.h"
#include "../../text/clear3.h"
#include "../../text/clear4.h"
#include "../../text/clear5.h"
#include "../../text/clear6.h"
#include "../../text/clear7.h"
#include "../../text/clear8.h"
static const u8* const STRS_TAB[] = { text_clear1, text_clear2, text_clear3, text_clear4, text_clear5, text_clear6, text_clear7, text_clear8, };

// ---------------------------------------------------------------- メイン
void sceneStageClearMain(u16 scene_ct)
{
    if (scene_ct == 31) {
        u8  stgNr = stgGetStageNr() - 2; // ステージ番号は既に +1 されている ので stgNr は 0, 1, ...
        u16 bonus = ((stgNr / 2) + 1) * 100;

        scoreAdd(bonus);
        static const u8 str1[] = { DC_B, DC_O, DC_N, DC_U, DC_S, DC_SP, 0, };
        static const u8 str2[] = { DC_0, DC_SP, DC_P, DC_CAPS, DC_T, DC_S, 0, };
        stgNr = stgNr & 0x07;
        printSetAddr((u8*)VVRAM_TEXT_ADDR(14, 6)); printSetAtb(VATB(6, 0, 0)); printString(STRS_TAB[stgNr]);
        printSetAddr((u8*)VVRAM_TEXT_ADDR(13, 8)); printSetAtb(VATB(7, 0, 0)); printString(str1); printU16Left(bonus); printString(str2);
    } else if (scene_ct == 30) {
#define L   10
#include "../../music/clear.h"
        sd3Play(mml0_0, mml1_0, mml2_0, true);

    } else if (scene_ct == 0)  {
        sysSetScene(sceneGameInit, sceneGameMain);
    }
}