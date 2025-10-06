/**
 * オープニング シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../system/sys.h"
#include "../system/sound.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/obj.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "../objworks/obj-player.h"
#include "scene-game.h"
#include "scene-opening.h"

// ---------------------------------------------------------------- 初期化
#if DEBUG
static const u8 SCENE_NAME_[] = { DC_O, DC_P, DC_E, DC_N, 0, };
#endif
void sceneOpeningInit(void)
{
#if DEBUG
    scoreSetStepString(SCENE_NAME_);
#endif
    // プレーヤー出現
    Obj* p_player = objCreatePlayer(objPlayerInit, objPlayerMain, objPlayerDraw, nullptr);
    p_player->step          = OBJ_PLAYER_STEP_DEMO;
    p_player->u_geo.geo8.yh = VRAM_HEIGHT;
    p_player->u_geo.geo.sy  = -0x0080;

    sysSetSceneCounter(13);
}


// ---------------------------------------------------------------- メイン
#include "../../text/opening.h"
void sceneOpeningMain(u16 scene_ct)
{
    switch (scene_ct) {
        default: break;
        case 1:
            {
                // プレーヤー制御可能に
                Obj* p_player = objGetInUsePlayer();
                p_player->step         = OBJ_PLAYER_STEP_NORMAL;
                p_player->u_geo.geo.sy = 0x0000;
            }
            printSetAddr((u8*)VVRAM_TEXT_ADDR(15, 8));
            printString(text_opening);
        break;
        case 0: // オープニング ジングル
        {
#define L   12
#include "../../music/opening.h"
            sd3Play(mml0_0, mml1_0, mml2_0, true);
            sysSetScene(sceneGameInit, sceneGameMain);
        }
    }
}