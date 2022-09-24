/**
 * タイトル シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/version.h"
#include "../system/input.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/math.h"
#include "../system/sound.h"
#include "../system/obj.h"
#include "../game/se.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "scene_instruction.h"
#include "scene_game_mode.h"
#include "scene_title_demo.h"
#include "scene_title.h"

// ---------------------------------------------------------------- 変数, マクロ
extern u16 build_nr; // Defined at crt0.asm

#define SYS_SCENE_WORK_START_CT 0

// ---------------------------------------------------------------- 初期化
void sceneTitleInit()
{
    objInit();
#if DEBUG
    static const u8 str[] = { DC_T, DC_I, DC_T, DC_L, DC_E, 0, };
    scoreSetStepString(str);
#endif
    sysSetSceneCounter(300);
}

// ---------------------------------------------------------------- メイン
void sceneTitleMain(u16 scene_ct)
{
#include "../../text/title_snail.h"
#include "../../text/title_start.h"
#include "../../text/title_bonus.h"
#include "../../text/title_comma.h"
#include "../../text/title_pts.h"
    // -------- NWK Presents
    printSetAddr((u8*)VVRAM_TEXT_ADDR(3, 2)); printString(sceneTitleGetStrNwkPresents());

    // -------- コピーライトとバージョン
    printSetAddr((u8*)VVRAM_TEXT_ADDR(10, 23)); printString(text_title_snail);
    {
#if VER_ALPHA != 0
        static const u8 str_version[] = {
            DC_V, DC_CAPS, DC_E, DC_R, DC_PERIOD,
            DC_0 + VER_MAJOR, DC_PERIOD, DC_0 + VER_MINOR0, DC_0 + VER_MINOR1, DC_PERIOD,
            DC_CAPS, DC_A, DC_CAPS, DC_L, DC_P, DC_H, DC_A, DC_0 + VER_ALPHA, DC_PERIOD, 0
        };
        printSetAddr((u8*)VVRAM_TEXT_ADDR(20, 24));
#elif VER_BETA != 0
        static const u8 str_version[] = {
            DC_V, DC_CAPS, DC_E, DC_R, DC_PERIOD,
            DC_0 + VER_MAJOR, DC_PERIOD, DC_0 + VER_MINOR0, DC_0 + VER_MINOR1, DC_PERIOD,
            DC_CAPS, DC_B, DC_CAPS, DC_E, DC_T, DC_A, DC_0 + VER_BETA, DC_PERIOD, 0
        };
        printSetAddr((u8*)VVRAM_TEXT_ADDR(21, 24));
#else
        static const u8 str_version[] = {
            DC_V, DC_CAPS, DC_E, DC_R, DC_PERIOD,
            DC_0 + VER_MAJOR, DC_PERIOD, DC_0 + VER_MINOR0, DC_0 + VER_MINOR1, DC_PERIOD, 0
        };
        printSetAddr((u8*)VVRAM_TEXT_ADDR(27, 24));
#endif
        printString(str_version); printU16Left(build_nr);
    }

    // -------- スタートの点滅
    u8 flash = sysSceneGetWork(SYS_SCENE_WORK_START_CT) ? 0x01 : 0x08;
    if (!(scene_ct & flash)) {
        printSetAddr((u8*)VVRAM_TEXT_ADDR(8, 18)); printString(text_title_start);
    }

    // -------- 宇宙船ボーナス案内
    printSetAddr((u8*)VVRAM_TEXT_ADDR( 6, 20)); printString(text_title_bonus);
    printU16Left(SCORE_BONUS_SHIP_1);                        printString(text_title_comma);
    printU16Left(SCORE_BONUS_SHIP_1 + SCORE_BONUS_SHIP);     printString(text_title_comma);
    printU16Left(SCORE_BONUS_SHIP_1 + SCORE_BONUS_SHIP * 2); printString(text_title_pts);

    // -------- タイトル表示
    sceneTitleDrawTitleS(     (u8*)VVRAM_TEXT_ADDR(7,  5));
    sceneTitleDrawTitleT(     (u8*)VVRAM_TEXT_ADDR(14, 5));
    sceneTitleDrawTitleHyphen((u8*)VVRAM_TEXT_ADDR(21, 8));
    sceneTitleDrawTitle2(     (u8*)VVRAM_TEXT_ADDR(26, 5));

    // -------- ゲームスタート
    if (sysSceneGetWork(SYS_SCENE_WORK_START_CT)) {
        sysSceneIncWork(SYS_SCENE_WORK_START_CT);
        if (30 < sysSceneGetWork(SYS_SCENE_WORK_START_CT)) {
           sysSetScene(sceneGameModeInit, sceneGameModeMain);
        }
    } else if (inputGetTrigger() & INPUT_MASK_PLAY) {
        sdSetEnabled(true);
        sdPlaySe(SE_START);
        sysSetMode(true);
        scoreGameStart();
        sysSceneSetWork(SYS_SCENE_WORK_START_CT, 1);
    } else if (scene_ct == 0)  {
        sysSetScene(sceneInstructionInit, sceneInstructionMain);
    }

}

// ---------------------------------------------------------------- NWK Presents...
const u8* sceneTitleGetStrNwkPresents()
{
#include "../../text/nwk_presents.h"
    return text_nwk_presents;
}

// ---------------------------------------------------------------- タイトル ロゴ表示
#include "../../cg/title_s.h"
#include "../../cg/title_t.h"
#include "../../cg/title_hyphen.h"
#include "../../cg/title_2.h"

void sceneTitleDrawTitleS(const u8* draw_addr)
{
    vVramDrawRectTransparent(draw_addr, cg_title_s,      W8H8(CG_TITLE_S_WIDTH, CG_TITLE_S_HEIGHT));
}
void sceneTitleDrawTitleT(const u8* draw_addr)
{
    vVramDrawRectTransparent(draw_addr, cg_title_t,      W8H8(CG_TITLE_T_WIDTH, CG_TITLE_T_HEIGHT));
}
void sceneTitleDrawTitleHyphen(const u8* draw_addr)
{
    vVramDrawRectTransparent(draw_addr, cg_title_hyphen, W8H8(CG_TITLE_HYPHEN_WIDTH, CG_TITLE_HYPHEN_HEIGHT));
}
void sceneTitleDrawTitle2(const u8* draw_addr)
{
    vVramDrawRectTransparent(draw_addr, cg_title_2,      W8H8(CG_TITLE_2_WIDTH, CG_TITLE_2_HEIGHT));
}
