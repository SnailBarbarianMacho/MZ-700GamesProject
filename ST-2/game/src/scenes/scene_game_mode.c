/**
 * ゲーム モード選択 シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../system/sys.h"
#include "../system/sound.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/input.h"
#include "../game/score.h"
#include "../game/se.h"
#include "../game/game_mode.h"
#include "scene_logo.h"
#include "scene_opening.h"
#include "scene_game_mode.h"

// ---------------------------------------------------------------- 変数, 定数
static u8 scene_game_mode_ = GAME_MODE_NORMAL;

#include "../../text/game_mode_easy.h"
#include "../../text/game_mode_normal.h"
#include "../../text/game_mode_hard.h"
#include "../../text/game_mode_survival.h"
#include "../../text/game_mode_caravan.h"
static const u8* STR_TAB[] = {
    text_game_mode_easy,
    text_game_mode_normal,
    text_game_mode_hard,
    text_game_mode_survival,
    text_game_mode_caravan,
};
#include "../../text/game_mode_back.h"

// ---------------------------------------------------------------- 初期化
void sceneGameModeInit()
{
#if DEBUG
    static const u8 str[] = { DC_M, DC_O, DC_D, DC_E, 0, };
    scoreSetStepString(str);
#endif
    sdSetEnabled(true);
    gameSetMode(scene_game_mode_);
}

#define SYS_SCENE_WORK_START_CT     0

// ---------------------------------------------------------------- メイン
void sceneGameModeMain(u16 scene_ct)
{
#include "../../text/game_mode.h"
#define MENU_Y 8
    if (!sysSceneGetWork(SYS_SCENE_WORK_START_CT)) {
        // -------- タイトルに戻る
        u8 inp = inputGetTrigger();
        if (inp & INPUT_MASK_SEL) {
            sysSetMode(false);
            sysSetScene(sceneLogoInit, sceneLogoMain);
            return; // 表示しないで返す
        }

        // -------- メニュー表示
        printSetAddr((u8*)VVRAM_TEXT_ADDR(9, 3)); printString(text_game_mode);
        for (u8 i = 0; i < COUNT_OF(STR_TAB); i++) {
            sceneGamePrintGameMode((u8*)VVRAM_TEXT_ADDR(5, MENU_Y + i * 2), i, true);
        }
        printSetAddr((u8*)VVRAM_TEXT_ADDR(9, 20));
        printString(text_game_mode_back);

        // -------- カーソル移動
        if (inp & INPUT_MASK_U) {
            sdPlaySe(SE_GET_ITEM);
            if (scene_game_mode_ == 0) { scene_game_mode_ = COUNT_OF(STR_TAB); }
            --scene_game_mode_;
        }
        if (inp & INPUT_MASK_D) {
            sdPlaySe(SE_GET_ITEM);
            scene_game_mode_ ++;
            if (scene_game_mode_ == COUNT_OF(STR_TAB)) { scene_game_mode_ = 0; }
        }
        scoreGameStart();

        // -------- カーソル表示
        printSetAddr((u8*)VVRAM_TEXT_ADDR(3, MENU_Y + scene_game_mode_ * 2));
        printSetAtb(VATB(2, 0, 0)) ;
        printPutc((scene_ct & 4) ? DC_NICOCHAN_0 : DC_NICOCHAN_1);

        // -------- スタート ボタン
        if (scene_game_mode_ != NR_GAME_MODES) { // ゲーム開始
            if (inp & (INPUT_MASK_A | INPUT_MASK_B | INPUT_MASK_PLAY)) {
                sdPlaySe(SE_START);
                sysSceneIncWork(SYS_SCENE_WORK_START_CT);
            }
            gameSetMode(scene_game_mode_);
        }
    } else {
        // -------- スタート
        sysSceneIncWork(SYS_SCENE_WORK_START_CT);
        if (scene_ct & 1) {
            sceneGamePrintGameMode((u8*)VVRAM_TEXT_ADDR(5, MENU_Y + scene_game_mode_ * 2), scene_game_mode_, true);
        }
        if (60 < sysSceneGetWork(SYS_SCENE_WORK_START_CT)) {
            sysSetScene(sceneOpeningInit, sceneOpeningMain);
        }
    }

}

// ---------------------------------------------------------------- print
void sceneGamePrintGameMode(u8* const addr, const u8 game_mode, const bool b_with_explanation)
{
    STATIC_ASSERT(COUNT_OF(STR_TAB) == NR_GAME_MODES, invalid_game_modes);
    printSetAddr(addr);
    if (b_with_explanation) {
        printString(STR_TAB[game_mode]);
    } else {
        printStringWithLength(STR_TAB[game_mode], 8);
    }
}
