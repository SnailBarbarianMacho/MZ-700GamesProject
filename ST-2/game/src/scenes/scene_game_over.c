/**
 * ゲーム オーバー シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../system/sys.h"
#include "../system/sound.h"
#include "../system/print.h"
#include "../system/obj.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "../game/bgm.h"
#include "scene_logo.h"
#include "scene_game_over.h"

// ---------------------------------------------------------------- 初期化
void sceneGameOverInit(void)
{
    scoreReflectHiScore();
    sceneGameOverInitWithoutReflectHiScore();
}

#if DEBUG
static const u8 SCENE_NAME_[] = { DC_O, DC_V, DC_E, DC_R, 0, };
#endif
void sceneGameOverInitWithoutReflectHiScore(void) // 強制ゲームオーバー時は, ハイスコアへの反映は無し
{
#if DEBUG
    scoreSetStepString(SCENE_NAME_);
#endif
    sysSetMode(false);
    objInit();
    sysSetSceneCounter(20);
    sdPlayBgm(BGM_NONE);
}


// ---------------------------------------------------------------- メイン
void sceneGameOverMain(u16 scene_ct)
{
    // -------- ゲームオーバー ジングル
    if (scene_ct == 18) {
#define L  8
#include "../../music/ronin.h"
        sd3Play(mml0_0, mml1_0, mml2_0, true);
        sdSetEnabled(false);// 余計な音は全て停止
    } else if (scene_ct == 0)  {
        scoreResetLevel();
        sysSetScene(sceneLogoInit, sceneLogoMain);
    }
}