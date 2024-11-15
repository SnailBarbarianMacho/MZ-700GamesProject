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
#include "../objworks/obj_player.h"
#include "../objworks/obj_player_bullet.h"
#include "../objworks/obj_enemy.h"
#include "scene_game.h"
#include "scene_instruction.h"

// ---------------------------------------------------------------- マクロ
#define INST_CT 400

// ---------------------------------------------------------------- 初期化
#if DEBUG
static const u8 SCENE_NAME_[] = { DC_I, DC_N, DC_S, DC_T, 0, };
#endif
void sceneInstructionInit(void)
{
#if DEBUG
    scoreSetStepString(SCENE_NAME_);
#endif
    objInit();
    scoreResetLevel();// ちゃんとリセットしておかないと, デモ プレイでどんどんレベルアップしていく!
    {
        Obj* const p_player = objCreatePlayer(objPlayerInit, objPlayerMain, objPlayerDraw, nullptr);
        p_player->step          = OBJ_PLAYER_STEP_DEMO;
        p_player->u_geo.geo8.xh  = 5;
        p_player->u_geo.geo8.sxl = 0x20;
    }
    {
        Obj* const p_enemy = objCreateEnemy(objEnemyInit3_1, objEnemyMainDemo3_1, objEnemyDraw3_1, nullptr);
        p_enemy->u_geo.geo8.xh = 33;
        p_enemy->u_geo.geo8.yh = 5;
    }

    sysSetSceneCounter(INST_CT);
}

// ---------------------------------------------------------------- メイン
#include "../../text/key_assign.h"
#include "../../text/key_move.h"
#include "../../text/key_shot.h"
#include "../../text/key_joy.h"
#include "../../text/key_start.h"
#include "../../text/key_cancel.h"
void sceneInstructionMain(u16 scene_ct)
{
     // -------- 説明の表示
    s16 ct = INST_CT - scene_ct;
    static struct s_Tab {
        u8* const draw_addr;
        u8* const str;
        u8  ct_offset;
    } const tab[] = {
        { (u8*)VVRAM_TEXT_ADDR(10, 2), text_key_assign, 0 },
        { (u8*)VVRAM_TEXT_ADDR(4,  5), text_key_move,   40 },
        { (u8*)VVRAM_TEXT_ADDR(4,  9), text_key_shot,  110 },
        { (u8*)VVRAM_TEXT_ADDR(4, 11), text_key_joy,   150 },
        { (u8*)VVRAM_TEXT_ADDR(4, 18), text_key_start, 220 },
        { (u8*)VVRAM_TEXT_ADDR(4, 20), text_key_cancel,250 },
        { nullptr, nullptr, 0 },
    };
    for (const struct s_Tab* pTab = tab; pTab->draw_addr; pTab++) {
        printSetAddr(pTab->draw_addr);
        printStringWithLength(pTab->str, ct - pTab->ct_offset);
    }

    // -------- プレーヤーと敵の制御
    Obj* const p_player = objGetInUsePlayer();
    if (ct == (33 - 4) * 8) { // 停止して弾を撃つ
        p_player->u_geo.geo8.sxl = 0x00;
        objCreatePlayerBullet(objPlayerBulletInit, objPlayerBulletMain, objPlayerBulletDraw, p_player);
    }

    // -------- 次のシーンへ
    if (scene_ct == 0)  {
        objInit();
        sysSetScene(sceneGameInit, sceneGameMain);
    }
}
