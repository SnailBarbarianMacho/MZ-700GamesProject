/**
 * ゲーム モード選択 シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "../system/sys.h"
#include "../system/sound.h"
#include "../system/vvram.h"
#include "../system/print.h"
#include "../system/input.h"
#include "../game/score.h"
#include "../game/se.h"
#include "../game/game-mode.h"
#include "scene-logo.h"
#include "scene-opening.h"
#include "scene-game-mode.h"

// ---------------------------------------------------------------- 変数, 定数
static u8 scene_game_mode_ = GAME_MODE_NORMAL;

#include "../../text/game-mode-easy.h"
#include "../../text/game-mode-normal.h"
#include "../../text/game-mode-hard.h"
#include "../../text/game-mode-survival.h"
#include "../../text/game-mode-mubo.h"
#include "../../text/game-mode-caravan.h"
#include "../../text/joymode.h"
#include "../../text/joymode-auto-detect.h"
#include "../../text/joymode-mz1x03.h"
#include "../../text/game-mode-mz1x03-sensitivity.h"
static const u8* STR_TAB[] = {
    text_game_mode_easy,
    text_game_mode_normal,
    text_game_mode_hard,
    text_game_mode_survival,
    text_game_mode_mubo,
    text_game_mode_caravan,
    text_joymode,
    text_game_mode_mz1x03_sensitivity,
};
#include "../../text/game-mode-back.h"

#define MENU_X  5
#define MENU_Y  6

// ---------------------------------------------------------------- 初期化
#if DEBUG
static const u8 SCENE_NAME_[] = { DC_M, DC_O, DC_D, DC_E, 0, };
#endif
void sceneGameModeInit(void)
{
#if DEBUG
    scoreSetStepString(SCENE_NAME_);
#endif
    sdSetEnabled(true);
    gameSetMode(scene_game_mode_);
}

#define SYS_SCENE_WORK_START_CT     0

// ---------------------------------------------------------------- メイン
#include "../../text/game-mode.h"
void sceneGameModeMain(u16 scene_ct)
{
    // -------- ゲームオーバー表示抑止
    scoreSetGameOverDisabled();

    if (!sysSceneGetWork(SYS_SCENE_WORK_START_CT)) {
        // -------- タイトルに戻る
        u8 inp = inputGetTrigger();
        if (inp & INPUT_MASK_CANCEL) {
            sysSetScene(sceneLogoInit, sceneLogoMain);
            return; // 表示しないで返す
        }

        // -------- メニュー表示
        printSetAddr((u8*)VVRAM_TEXT_ADDR(9, 3)); printString(text_game_mode);
        for (u8 i = 0; i < COUNT_OF(STR_TAB); i++) {
            sceneGamePrintGameMode((u8*)VVRAM_TEXT_ADDR(MENU_X, MENU_Y + i * 2), i, true);
        }

        // ジョイスティックモード
        printSetAddr((u8*)VVRAM_TEXT_ADDR(MENU_X + TEXT_JOYMODE_LEN, MENU_Y + GAME_MODE_JOYMODE * 2));
        printString(inputIsMZ1X03Enabled() ? text_joymode_mz1x03 : text_joymode_auto_detect);

        // MZ1X03 感度
        printSetAddr((u8*)VVRAM_TEXT_ADDR(MENU_X + TEXT_GAME_MODE_MZ1X03_SENSITIVITY_LEN, MENU_Y + GAME_MODE_MZ1X03_SENSITIVITY * 2));
        printPutc(DC_1 + inputGetMZ1X03sensitivity());

        // 上下左右入力表示
        u8 i = inputGet();
        u8 *addr = (u8*)VVRAM_ATB_ADDR(MENU_X + TEXT_GAME_MODE_MZ1X03_SENSITIVITY_LEN - 5, MENU_Y + (COUNT_OF(STR_TAB) - 1) * 2);
        if (i & INPUT_MASK_L) { addr[0] = VATB(7, 0, 0); }
        if (i & INPUT_MASK_R) { addr[1] = VATB(7, 0, 0); }
        if (i & INPUT_MASK_U) { addr[2] = VATB(7, 0, 0); }
        if (i & INPUT_MASK_D) { addr[3] = VATB(7, 0, 0); }

        printSetAddr((u8*)VVRAM_TEXT_ADDR(11, 23)); printString(text_game_mode_back);

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
        gameSetMode(scene_game_mode_);
        scoreGameStart();

        // -------- カーソル表示
        printSetAddr((u8*)VVRAM_TEXT_ADDR(MENU_X - 2, MENU_Y + scene_game_mode_ * 2));
        printSetAtb(VATB(2, 0, 0)) ;
        printPutc((scene_ct & 4) ? DC_NICOCHAN_0 : DC_NICOCHAN_1);

        // -------- 決定ボタンでスタート
        if (inp & (INPUT_MASK_A | INPUT_MASK_B | INPUT_MASK_START)) {
            switch (scene_game_mode_) {
                default:
                    sysSetGameMode(true);
                    scoreGameStart();
                    sdPlaySe(SE_START);
                    sysSceneIncWork(SYS_SCENE_WORK_START_CT);
                    break;
                case GAME_MODE_JOYMODE:
                    sdPlaySe(SE_GET_ITEM);
                    inputSetMZ1X03Enabled(!inputIsMZ1X03Enabled());
                    break;
                case GAME_MODE_MZ1X03_SENSITIVITY:
                    sdPlaySe(SE_GET_ITEM);
                    inputIncMZ1X03sensitivity();
                    break;
            }
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
    STATIC_ASSERT(COUNT_OF(STR_TAB) == NR_GAME_MODES, "invalid_game_modes");
    printSetAddr(addr);
    if (b_with_explanation) {
        printString(STR_TAB[game_mode]);
    } else {
        printStringWithLength(STR_TAB[game_mode], 8);
    }
}
