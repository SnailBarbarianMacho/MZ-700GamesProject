/**
 * スコア テーブル ステップ
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
#include "stepGame.h"
#include "stepInstruction.h"

#define INST_CT 350

// ---------------------------------------------------------------- 初期化
void stepInstructionInit()
{
#if DEBUG
    static const u8 stepStr[] = {CHAR_I, CHAR_N, CHAR_S, CHAR_T, 0};
    scoreSetStepString(stepStr);
#endif
    objInit();
    scoreResetLevel();// ちゃんとリセットしておかないと, デモ プレイでどんどんレベルアップしていく!
    {
        Obj* const pPlayer = objCreatePlayer(objPlayerInit, objPlayerMain, objPlayerDisp, nullptr);
        pPlayer->step          = OBJ_PLAYER_STEP_DEMO;
        pPlayer->uGeo.geo8.xh  = 5;
        pPlayer->uGeo.geo8.sxl = 0x20;
    }
    {
        Obj* const pEnemy = objCreateEnemy(objEnemyInit3_1, objEnemyMainDemo3_1, objEnemyDisp3_1, nullptr);
        pEnemy->uGeo.geo8.xh = 33;
        pEnemy->uGeo.geo8.yh = 5;
    }

    sysSetStepCounter(INST_CT);
}

// ---------------------------------------------------------------- メイン
void stepInstructionMain(u16 stepCounter)
{
     // -------- 説明の表示
#include "../../text/keyassign.h"
#include "../../text/keymove.h"
#include "../../text/keyshot.h"
#include "../../text/keyjoy.h"
#include "../../text/keypause.h"
    s16 ct = INST_CT - stepCounter;
    static struct s_Tab {
        u8* const dispAddr;
        u8* const str;
        u8  ctOffset;
    } const tab[] = {
        { (u8*)VVRAM_TEXT_ADDR(10, 2), str_keyassign, 0 },
        { (u8*)VVRAM_TEXT_ADDR(4,  5), str_keymove,   40 },
        { (u8*)VVRAM_TEXT_ADDR(4,  9), str_keyshot,  110 },
        { (u8*)VVRAM_TEXT_ADDR(4, 11), str_keypause, 150 },
        { (u8*)VVRAM_TEXT_ADDR(4, 13), str_keyjoy,   190 },
        { nullptr, nullptr, 0 },
    };
    for (const struct s_Tab* pTab = tab; pTab->dispAddr; pTab++) {
        printSetAddr(pTab->dispAddr);
        printStringWithLength(pTab->str, ct - pTab->ctOffset);
    }

    // -------- プレーヤーと敵の制御
    Obj* const pPlayer = objGetInUsePlayer();
    if (ct == (33 - 4) * 8) { // 停止して弾を撃つ
        pPlayer->uGeo.geo8.sxl = 0x00;
        objCreatePlayerBullet(objPlayerBulletInit, objPlayerBulletMain, objPlayerBulletDisp, pPlayer);
    }

    // -------- 次のステップへ
    if (stepCounter == 0)  {
        objInit();
        sysSetStep(stepGameInit, stepGameMain);
    }
}
