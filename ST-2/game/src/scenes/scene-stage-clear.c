/**
 * ステージ クリア シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/sound.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "scene-game.h"
#include "scene-stage-clear.h"

// ---------------------------------------------------------------- 初期化
#if DEBUG
static const u8 SCENE_NAME_[] = { DC_S, DC_C, DC_L, DC_R, 0, };
#endif
void sceneStageClearInit(void)
{
#if DEBUG
    scoreSetStepString(SCENE_NAME_);
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
static const u8* const STRS_TAB_[] = { text_clear1, text_clear2, text_clear3, text_clear4, text_clear5, text_clear6, text_clear7, text_clear8, };

// ---------------------------------------------------------------- メイン
static const u8 STR_BONUS_[] = { DC_B, DC_O, DC_N, DC_U, DC_S, DC_SP, 0, };
static const u8 STR_0PTS_[] = { DC_0, DC_SP, DC_P, DC_CAPS, DC_T, DC_S, 0, };
void sceneStageClearMain(u16 scene_ct)
{
    if (scene_ct == 31) {
#pragma disable_warning 110 // 除算最適化警告
#pragma save
        u8  stgNr = stgGetStageNr() - 2; // ステージ番号は既に +1 されている ので stgNr は 0, 1, ...
        u16 bonus = ((stgNr / 2) + 1) * 100;
#pragma restore

        scoreAdd(bonus);
        stgNr = stgNr & 0x07;
        printSetAddr((u8*)VVRAM_TEXT_ADDR(14, 6)); printSetAtb(VATB(6, 0, 0)); printString(STRS_TAB_[stgNr]);
        printSetAddr((u8*)VVRAM_TEXT_ADDR(13, 8)); printSetAtb(VATB(7, 0, 0)); printString(STR_BONUS_); printU16Left(bonus); printString(STR_0PTS_);
    } else if (scene_ct == 30) {
#define L   10
#include "../../music/clear.h"
        sd3Play(mml0_0, mml1_0, mml2_0, true);

    } else if (scene_ct == 0)  {
        sysSetScene(sceneGameInit, sceneGameMain);
    }
}