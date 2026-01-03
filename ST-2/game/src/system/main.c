/**
 * メイン ループ
 *
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "sys.h"
#include "input.h"
#include "vvram.h"
#include "print.h"
#include "sound.h"
#include "obj.h"
#include "math.h"
#include "../scenes/scene-logo.h"
#include "../game/stars.h"
#include "../game/score.h"
#include "../game/game-mode.h"
#include "../objworks/obj-item.h"

void main(void) __naked
{
    sysInit();
    sysSetScene(sceneLogoInit, sceneLogoMain);
    inputInit();
    vvramInit();
    sdInit();
    objInit();
    mathInit();
    scoreInit();
    starsInit();
    objItemInitTab();
    gameSetMode(GAME_MODE_NORMAL);
    while (1) {
        sdSeMain(); // サウンドは 1 フレームに 2 回呼ぶ. 最初は SE→BGMの順, 2回目はその逆
        sdBgmMain();
        starsMain();// 背景なので表示優先一番低い
        inputMain();// Obj の前
        objMain();  // 入力の後
        sysMain();  // Obj より表示優先高い
        scoreMain();
        sdBgmMain();
        sdSeMain();
        vvramTrans();
    }
}
