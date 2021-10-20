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
#include "../objworks/objEnemy.h"
#include "stepLogo.h"
#include "stepTitleDemo.h"
#include "stepScoreTable.h"

#define INST_CT 450

static u8 sLoopCt = 0;

// ---------------------------------------------------------------- 初期化
void stepScoreTableInit()
{
#if DEBUG
    static const u8 stepStr[] = {CHAR_T, CHAR_A, CHAR_B, CHAR_L, CHAR_E, 0};
    scoreSetStepString(stepStr);
#endif
    objInit();
    sysSetStepCounter(INST_CT);
}

// ---------------------------------------------------------------- メイン
void stepScoreTableMain(u16 stepCounter)
{
#include "../../text/tabscore.h"
#include "../../text/tab100.h"
#include "../../text/tab150.h"
#include "../../text/tab200.h"
#include "../../text/tab250.h"
#include "../../text/tab300.h"
#include "../../text/tab400.h"
#include "../../text/tablevelup.h"
    // -------- 敵の表示
    s16 ct = INST_CT - stepCounter;
    if (ct == 80) {
        struct s_Tab {
            void (*initFunc)(Obj* const, Obj* const);
            void (*drawFunc)(Obj* const, u8* drawAddr);
            s8      x, y;
        } const tab[] = {
            { objEnemyInit3_1, objEnemyDraw3_1,  3,  4, },
            { objEnemyInit3_2, objEnemyDraw3_2,  3,  8, },
            { objEnemyInit3_3, objEnemyDraw3_3,  3, 12, },
            { objEnemyInit3_4, objEnemyDraw3_4, 22,  4, },
            { objEnemyInit3_5, objEnemyDraw3_5, 22,  8, },
            { objEnemyInit3_6, objEnemyDraw3_6, 22, 12, },
            { nullptr, nullptr, 0, 0, },
        };
        for (const struct s_Tab* pTab = tab; pTab->initFunc; pTab++) {
            Obj* pEnemy = objCreateEnemy(pTab->initFunc, objEnemyMainDemo3_1, pTab->drawFunc, nullptr);
            pEnemy->uGeo.geo8.xh = pTab->x;
            pEnemy->uGeo.geo8.yh = pTab->y;
        }
    }

    // -------- アイテム表示
    if (80 < ct) {
        *((u8*)VVRAM_TEXT_ADDR(4, 20)) = ((ct >> 2) & 3) ? CHAR_SP : CHAR_L;
        *((u8*)VVRAM_ATB_ADDR(4, 20))  = VATB(7, 1, 0);
    }

    // -------- 説明の表示
    static struct s_Tab {
        u8* const drawAddr;
        u8* const str;
        u16 ctOffset;
    } const tab[] = {
        { (u8*)VVRAM_TEXT_ADDR(12,  2), str_tabscore, 30 },
        { (u8*)VVRAM_TEXT_ADDR( 7,  5), str_tab100, 100 },
        { (u8*)VVRAM_TEXT_ADDR( 7,  9), str_tab150, 140 },
        { (u8*)VVRAM_TEXT_ADDR( 7, 13), str_tab200, 180 },
        { (u8*)VVRAM_TEXT_ADDR(26,  5), str_tab250, 220 },
        { (u8*)VVRAM_TEXT_ADDR(26,  9), str_tab300, 260 },
        { (u8*)VVRAM_TEXT_ADDR(26, 13), str_tab400, 300 },
        { (u8*)VVRAM_TEXT_ADDR( 7, 20), str_tablevelup, 340 },
        { nullptr, nullptr, 0 },
    };
    for (const struct s_Tab* pTab = tab; pTab->drawAddr; pTab++) {
        printSetAddr(pTab->drawAddr);
        printStringWithLength(pTab->str, ct - pTab->ctOffset);
    }

    // -------- 次のステップへ
    if (stepCounter == 0)  {
        sLoopCt++;
        if (4 < sLoopCt) {
            sLoopCt = 0;
            vramClear();// 仮想 VRAM の残像を消す
            sysSetStep(stepLogoInit, stepLogoMain);
        } else {
            sysSetStep(stepTitleDemoInit, stepTitleDemoMain);
        }
    }
}

// ---------------------------------------------------------------- デモ ループ カウンタ
u8 stepScoreTableGetLoopCt() {
    return sLoopCt;
}