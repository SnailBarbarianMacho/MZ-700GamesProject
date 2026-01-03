/**
 * スコア テーブル シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/vvram.h"
#include "../system/print.h"
#include "../system/obj.h"
#include "../game/score.h"
#include "../objworks/obj-enemy.h"
#include "scene-logo.h"
#include "scene-title-demo.h"
#include "scene-score-table.h"

#define INST_CT 450

// ---------------------------------------------------------------- 変数
u8 scene_score_tab_loop_ct_ = 0;  // 数回に1回ロゴループ, デモ時のステージ

// ---------------------------------------------------------------- 初期化
#if DEBUG
static const u8 SCENE_NAME_[] = { DC_T, DC_A, DC_B, DC_L, DC_E, 0, };
#endif
void sceneScoreTableInit(void)
{
#if DEBUG
    scoreSetStepString(SCENE_NAME_);
#endif
    objInit();
    sysSetSceneCounter(INST_CT);
}


// ---------------------------------------------------------------- メイン
#include "../../text/tab-score.h"
#include "../../text/tab100.h"
#include "../../text/tab150.h"
#include "../../text/tab200.h"
#include "../../text/tab250.h"
#include "../../text/tab300.h"
#include "../../text/tab400.h"
#include "../../text/tab-level-up.h"
void sceneScoreTableMain(u16 scene_ct)
{
    // -------- 敵の表示
    s16 ct = INST_CT - scene_ct;
    if (ct == 80) {
        struct s_Tab {
            void (*init_func)(Obj* const, Obj* const);
            void (*draw_func)(Obj* const, u8*);
            s8      x, y;
        } const TAB[] = {
            { objEnemyInit3_1, objEnemyDraw3_1,  3,  4, },
            { objEnemyInit3_2, objEnemyDraw3_2,  3,  8, },
            { objEnemyInit3_3, objEnemyDraw3_3,  3, 12, },
            { objEnemyInit3_4, objEnemyDraw3_4, 22,  4, },
            { objEnemyInit3_5, objEnemyDraw3_5, 22,  8, },
            { objEnemyInit3_6, objEnemyDraw3_6, 22, 12, },
            { nullptr, nullptr, 0, 0, },
        };
        for (const struct s_Tab* p_tab = TAB; p_tab->init_func; p_tab++) {
            Obj* p_enemy = objCreateEnemy(p_tab->init_func, objEnemyMainDemo3_1, p_tab->draw_func, nullptr);
            p_enemy->u_geo.geo8.xh = p_tab->x;
            p_enemy->u_geo.geo8.yh = p_tab->y;
        }
    }

    // -------- アイテム表示
    if (80 < ct) {
        *((u8*)VVRAM_TEXT_ADDR(4, 20)) = ((ct >> 2) & 3) ? DC_SP : DC_L;
        *((u8*)VVRAM_ATB_ADDR(4, 20))  = VATB(7, 1, 0);
    }

    // -------- 説明の表示
    static struct s_Tab {
        u8* const draw_addr;
        u8* const str;
        u16 ctOffset;
    } const tab[] = {
        { (u8*)VVRAM_TEXT_ADDR(12,  2), text_tab_score, 30 },
        { (u8*)VVRAM_TEXT_ADDR( 7,  5), text_tab100, 100 },
        { (u8*)VVRAM_TEXT_ADDR( 7,  9), text_tab150, 140 },
        { (u8*)VVRAM_TEXT_ADDR( 7, 13), text_tab200, 180 },
        { (u8*)VVRAM_TEXT_ADDR(26,  5), text_tab250, 220 },
        { (u8*)VVRAM_TEXT_ADDR(26,  9), text_tab300, 260 },
        { (u8*)VVRAM_TEXT_ADDR(26, 13), text_tab400, 300 },
        { (u8*)VVRAM_TEXT_ADDR( 7, 20), text_tab_level_up, 340 },
        { nullptr, nullptr, 0 },
    };
    for (const struct s_Tab* pTab = tab; pTab->draw_addr; pTab++) {
        printSetAddr(pTab->draw_addr);
        printStringWithLength(pTab->str, ct - pTab->ctOffset);
    }

    // -------- 次のシーンへ
    if (scene_ct == 0)  {
        scene_score_tab_loop_ct_++;
        if (4 < scene_score_tab_loop_ct_) {
            scene_score_tab_loop_ct_ = 0;
            vvramClear();// 仮想 VRAM の残像を消す
            sysSetScene(sceneLogoInit, sceneLogoMain);
        } else {
            sysSetScene(sceneTitleDemoInit, sceneTitleDemoMain);
        }
    }
}

// ---------------------------------------------------------------- デモ ループ カウンタ
