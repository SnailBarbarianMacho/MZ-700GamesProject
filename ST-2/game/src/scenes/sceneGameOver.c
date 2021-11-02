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
#include "sceneLogo.h"
#include "sceneGameOver.h"

// ---------------------------------------------------------------- 初期化
void sceneGameOverInit()
{
    scoreReflectHiScore();
    sceneGameOverInitWithoutReflectHiScore();
}
void sceneGameOverInitWithoutReflectHiScore() // 強制ゲームオーバー時は, ハイスコアへの反映は無し
{
#if DEBUG
    static const u8 stepStr[] = {CHAR_O, CHAR_V, CHAR_E, CHAR_R, 0};
    scoreSetStepString(stepStr);
#endif
    sysSetMode(false);
    objInit();
    sysSetSceneCounter(20);
    sdPlayBgm(BGM_NONE);
}

// ---------------------------------------------------------------- メイン
void sceneGameOverMain(u16 sceneCounter)
{
    // -------- ゲームオーバー ジングル
    if (sceneCounter == 18) {
#define L  8
#include "../../music/ronin.h"
        sd3Play(mml0_0, mml1_0, mml2_0, true);
        sdSetEnabled(false);// 余計な音は全て停止
    } else if (sceneCounter == 0)  {
        scoreResetLevel();
        sysSetScene(sceneLogoInit, sceneLogoMain);
    }
}