/**
 * オープニング ステップ
 * @author Snail Barbarian Macho (NWK)
 */

#include "../system/sys.h"
#include "../system/sound.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/obj.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "../objworks/objPlayer.h"
#include "stepGame.h"
#include "stepOpening.h"

// ---------------------------------------------------------------- 初期化
void stepOpeningInit()
{
#if DEBUG
    static const u8 stepStr[] = {CHAR_O, CHAR_P, CHAR_E, CHAR_N, 0};
    scoreSetStepString(stepStr);
#endif
    // プレーヤー出現
    Obj* pPlayer = objCreatePlayer(objPlayerInit, objPlayerMain, objPlayerDisp, nullptr);
    pPlayer->step          = OBJ_PLAYER_STEP_DEMO;
    pPlayer->uGeo.geo8.yh  = VRAM_HEIGHT;
    pPlayer->uGeo.geo.sy   = -0x0080;

    sysSetStepCounter(13);
}

// ---------------------------------------------------------------- メイン
void stepOpeningMain(u16 stepCounter)
{
#include "../../text/opening.h"
    switch (stepCounter) {
        default: break;
        case 1:
            {
                // プレーヤー制御可能に
                Obj* pPlayer         = objGetInUsePlayer();
                pPlayer->step        = OBJ_PLAYER_STEP_NORMAL;
                pPlayer->uGeo.geo.sy = 0x0000;
            }
            printSetAddr((u8*)VVRAM_TEXT_ADDR(15, 8));
            printString(str_opening);
        break;
        case 0: // オープニング ジングル
        {
#define L   12
#include "../../music/opening.h"
            sd3Play(mml0_0, mml1_0, mml2_0, true);
            sysSetStep(stepGameInit, stepGameMain);
        }
    }
}