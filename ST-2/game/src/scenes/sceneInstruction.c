/**
 * スコア テーブル シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/obj.h"
#include "../game/score.h"
#include "../objworks/objPlayer.h"
#include "../objworks/objPlayerBullet.h"
#include "../objworks/objEnemy.h"
#include "sceneGame.h"
#include "sceneInstruction.h"

// ---------------------------------------------------------------- マクロ
#define INST_CT 350

// ---------------------------------------------------------------- 初期化
void sceneInstructionInit()
{
#if DEBUG
    static const u8 stepStr[] = {CHAR_I, CHAR_N, CHAR_S, CHAR_T, 0};
    scoreSetStepString(stepStr);
#endif
    objInit();
    scoreResetLevel();// ちゃんとリセットしておかないと, デモ プレイでどんどんレベルアップしていく!
    {
        Obj* const pPlayer = objCreatePlayer(objPlayerInit, objPlayerMain, objPlayerDraw, nullptr);
        pPlayer->step          = OBJ_PLAYER_STEP_DEMO;
        pPlayer->uGeo.geo8.xh  = 5;
        pPlayer->uGeo.geo8.sxl = 0x20;
    }
    {
        Obj* const pEnemy = objCreateEnemy(objEnemyInit3_1, objEnemyMainDemo3_1, objEnemyDraw3_1, nullptr);
        pEnemy->uGeo.geo8.xh = 33;
        pEnemy->uGeo.geo8.yh = 5;
    }

    sysSetSceneCounter(INST_CT);
}

// ---------------------------------------------------------------- メイン
void sceneInstructionMain(u16 sceneCounter)
{
     // -------- 説明の表示
#include "../../text/keyassign.h"
#include "../../text/keymove.h"
#include "../../text/keyshot.h"
#include "../../text/keyjoy.h"
#include "../../text/keypause.h"
    s16 ct = INST_CT - sceneCounter;
    static struct s_Tab {
        u8* const drawAddr;
        u8* const str;
        u8  ctOffset;
    } const tab[] = {
        { (u8*)VVRAM_TEXT_ADDR(10, 2), textKeyassign, 0 },
        { (u8*)VVRAM_TEXT_ADDR(4,  5), textKeymove,   40 },
        { (u8*)VVRAM_TEXT_ADDR(4,  9), textKeyshot,  110 },
        { (u8*)VVRAM_TEXT_ADDR(4, 11), textKeyjoy,   150 },
        { (u8*)VVRAM_TEXT_ADDR(4, 13), textKeypause, 190 },
        { nullptr, nullptr, 0 },
    };
    for (const struct s_Tab* pTab = tab; pTab->drawAddr; pTab++) {
        printSetAddr(pTab->drawAddr);
        printStringWithLength(pTab->str, ct - pTab->ctOffset);
    }

    // -------- プレーヤーと敵の制御
    Obj* const pPlayer = objGetInUsePlayer();
    if (ct == (33 - 4) * 8) { // 停止して弾を撃つ
        pPlayer->uGeo.geo8.sxl = 0x00;
        objCreatePlayerBullet(objPlayerBulletInit, objPlayerBulletMain, objPlayerBulletDraw, pPlayer);
    }

    // -------- 次のシーンへ
    if (sceneCounter == 0)  {
        objInit();
        sysSetScene(sceneGameInit, sceneGameMain);
    }
}
