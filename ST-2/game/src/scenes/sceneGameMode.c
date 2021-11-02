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
#include "../game/gameMode.h"
#include "sceneOpening.h"
#include "sceneGameMode.h"

// ---------------------------------------------------------------- 変数
static u8 _sceneGameMode = GAME_MODE_NORMAL;

// ---------------------------------------------------------------- 初期化
void sceneGameModeInit()
{
#if DEBUG
    static const u8 stepStr[] = {CHAR_M, CHAR_O, CHAR_D, CHAR_E, 0};
    scoreSetStepString(stepStr);
#endif
    sdSetEnabled(true);
    gameSetMode(_sceneGameMode);
}

#define SYS_SCENE_WORK_START_CT     0

// ---------------------------------------------------------------- メイン
void sceneGameModeMain(u16 sceneCounter)
{
    // -------- テキスト表示
#include "../../text/gameMode.h"
    printSetAddr((u8*)VVRAM_TEXT_ADDR(9, 3)); printString(textGameMode);

    //u8 mode = gameGetMode();
    if (!sysSceneGetWork(SYS_SCENE_WORK_START_CT)) {
        // -------- メニュー表示

        for (u8 i = 0; i < NR_GAME_MODES; i++) {
            sceneGamePrintGameMode((u8*)VVRAM_TEXT_ADDR(5, 8 + i * 2), i, true);
        }

        // -------- カーソル移動
        u8 inp = inputGetTrigger();
        if (inp & INPUT_MASK_U) {
            sdPlaySe(SE_GET_ITEM);
            if (_sceneGameMode == 0) { _sceneGameMode = NR_GAME_MODES; }
            _sceneGameMode --;
        }
        if (inp & INPUT_MASK_D) {
            sdPlaySe(SE_GET_ITEM);
            _sceneGameMode ++;
            if (_sceneGameMode == NR_GAME_MODES) { _sceneGameMode = 0; }
        }
        scoreGameStart();

        // -------- カーソル表示
        printSetAddr((u8*)VVRAM_TEXT_ADDR(3, 8 + _sceneGameMode * 2));
        printSetAtb(VATB(2, 0, 0)) ;
        printPutc((sceneCounter & 4) ? CHAR_NICOCHAN_0 : CHAR_NICOCHAN_1);

        // -------- スタート ボタン
        if (inp & (INPUT_MASK_A | INPUT_MASK_P)) {
            sdPlaySe(SE_START);
            sysSceneIncWork(SYS_SCENE_WORK_START_CT);
        }
        gameSetMode(_sceneGameMode);
    } else {
        // -------- スタート
        sysSceneIncWork(SYS_SCENE_WORK_START_CT);
        if (sceneCounter & 1) {
            sceneGamePrintGameMode((u8*)VVRAM_TEXT_ADDR(5, 8 + _sceneGameMode * 2), _sceneGameMode, true);
        }
        if (60 < sysSceneGetWork(SYS_SCENE_WORK_START_CT)) {
            sysSetScene(sceneOpeningInit, sceneOpeningMain);
        }
    }

}

// ---------------------------------------------------------------- print
void sceneGamePrintGameMode(u8* const addr, const u8 gameMode, const bool bWithExplanation)
{
#include "../../text/gameModeEasy.h"
#include "../../text/gameModeNormal.h"
#include "../../text/gameModeHard.h"
#include "../../text/gameModeSurvival.h"
#include "../../text/gameModeCaravan.h"
    static const u8* tab[] = {
        textGameModeEasy,
        textGameModeNormal,
        textGameModeHard,
        textGameModeSurvival,
        textGameModeCaravan,
    };
    STATIC_ASSERT(COUNT_OF(tab) == NR_GAME_MODES, invalid_game_modes);
    printSetAddr(addr);
    if (bWithExplanation) {
        printString(tab[gameMode]);
    } else {
        printStringWithLength(tab[gameMode], 8);
    }
}
