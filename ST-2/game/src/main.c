/**
 * メイン ループ
 *
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../src-common/common.h"
#include "system/sys.h"
#include "system/input.h"
#include "system/vram.h"
#include "system/print.h"
#include "system/sound.h"
#include "system/obj.h"
#include "system/math.h"
#include "scenes/scene_logo.h"
#include "game/stars.h"
#include "game/score.h"
#include "game/game_mode.h"
#include "objworks/obj_item.h"

void main() __naked
{
    sysInit();
    sysSetScene(sceneLogoInit, sceneLogoMain);
    inputInit();
    vramInit();
    sdInit();
    objInit();
    mathInit();
    scoreInit();
    starsInit();
    objItemInitTab();
    gameSetMode(GAME_MODE_NORMAL);
    while (1)
    {
        sdSeMain(); // サウンドは 1 フレームに 2 回呼ぶ. 最初は SE→BGMの順, 2回目はその逆
        sdBgmMain();
        starsMain();// 背景なので表示優先一番低い
        inputMain();// Obj の前
        objMain();  // 入力の後
        sysMain();  // Obj より表示優先高い
        scoreMain();
        sdBgmMain();
        sdSeMain();
        vramTrans();
    }
}
