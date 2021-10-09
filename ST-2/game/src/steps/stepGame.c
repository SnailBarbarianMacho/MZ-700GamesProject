/**
 * ゲーム ステップ
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "../system/sys.h"
#include "../system/addr.h"
#include "../system/input.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/sound.h"
#include "../system/obj.h"
#include "../system/math.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "../objworks/objPlayer.h"
#include "../../cg/Pause.h"
#include "stepStageClear.h"
#include "stepEnding.h"
#include "stepScoreTable.h"
#include "stepGameOver.h"
#include "stepGame.h"

#if DEBUG
static bool sbPause;
#endif

static u8 sStageDispTimer;

// ---------------------------------------------------------------- 初期化
void stepGameInit()
{
#if DEBUG
    sbPause = false;
    static const u8 stepStr[] = {CHAR_G, CHAR_A, CHAR_M, CHAR_E, 0,};
    scoreSetStepString(stepStr);
#endif
    if (!sysIsGameMode()) {// アトラクト モード時
#if DEBUG
        stgInit(0);// デバッグ時はいきなりラスボスとかあるので安全の為に最初の実表示
#else
        stgInit(stepScoreTableGetLoopCt());
#endif
        objCreatePlayer(objPlayerInit, objPlayerMain, objPlayerDisp, nullptr);
        sysSetStepCounter(1000);
    } else {
        sdSetEnabled(true);
    }

    stgSubInit();
    sStageDispTimer = 100;
}


// ---------------------------------------------------------------- メイン
void stepGameMain(u16 stepCounter)
{
    // ---------------- アトラクト モードでの挙動. ステージ進行, タイマ減算処理
    if (!sysIsGameMode()) {
        if (stgGetNrEnemies() == 0) {
            // -------- ステージ進行. 敵が 0 になったら次のステージへ
            switch (stgSubInit()) {
            default: break;
            case STG_STATUS_CLEAR:
                sysSetStep(stepScoreTableInit, stepScoreTableMain);
                return;
            }
        }
        if (stepCounter == 0) { // 時間切れでのステップへ. プレーヤーは何度死んでもいい
            sysSetStep(stepScoreTableInit, stepScoreTableMain);
        }
        return;
    }

    // ---------------- ゲーム モードでの挙動. ステージ数表示
    if (sStageDispTimer) {
        sStageDispTimer--;
        static const u8 strStage[] = { CHAR_S, CHAR_T, CHAR_A, CHAR_G, CHAR_E, 0, };
        printSetAddr((u8*)VVRAM_TEXT_ADDR(16, 8));
        printString(strStage);
        printU8Right(stgGetStageNr());
        printReady();
    }

    // ---------------- ゲーム モードでの挙動. ポーズ, ステップ, コンティニュー, ステージ進行
    Obj* const pPlayer = objGetInUsePlayer();
    if (pPlayer == nullptr) { return; }

    switch (pPlayer->step) {
    default: // OBJ_PLAYER_STEP_NORMAL
        // -------- ステージ進行. 敵が 0 になったら次のステージへ
        // プレーヤーがたまたま死んだ判定があるかもしれないのでそれもチェック
        if ((stgGetNrEnemies() == 0) && (!pPlayer->bHit)) {
            switch (stgSubInit()) {
            default: //case STG_STATUS_OK:
                break;
            case STG_STATUS_CLEAR:
                objInitEnemyBullet();
                sysSetStep(stepStageClearInit, stepStageClearMain);
                return;
            case STG_STATUS_ENDING:
                objInitEnemyBullet();
                sysSetStep(stepEndingInit, stepEndingMain);
                return;
            }
        }

        static const u8 mml0[] = { SD3_C3, 4, SD3_G3, 4, SD3_E3, 8, 0, };
#if DEBUG
        // デバッグ時ポーズ, ステップ動作, ゲーム中断
        if (sbPause || (inputGetTrigger() & INPUT_MASK_P)) {
            sd3Play(mml0, mml0, mml0, false);
            while (true) {
                inputMain();
                u8 inp = inputGetTrigger();
                if (inp & INPUT_MASK_P) {// ポーズ解除
                    sbPause = false;
                    sd3Play(mml0, mml0, mml0, false);
                    break;
                }
                if (inp & INPUT_MASK_B) {   // ステップ動作
                    sbPause = true;
                    break;
                }
                if (inp & INPUT_MASK_S) {   // ゲーム中断
                    sysSetGameMode(false);
                    sysSetStep(stepGameOverInitWithoutReflectHiScore, stepGameOverMain);
                    return;
                }
            }
        }
#else
        // リリース時ポーズ, ゲーム中断
        if (inputGetTrigger() & INPUT_MASK_P) {
            vramDrawRect((u8*)VRAM_TEXT_ADDR((u16)(VRAM_WIDTH - CG_PAUSE_WIDTH) / 2, 10), sPause, W8H8(CG_PAUSE_WIDTH, CG_PAUSE_HEIGHT));
            sd3Play(mml0, mml0, mml0, false);
            while (true) {
                inputMain();
                u8 inp = inputGetTrigger();
                if (inp & INPUT_MASK_P) {// ポーズ解除
                    sd3Play(mml0, mml0, mml0, false);
                    break;
                }
                if (inp & INPUT_MASK_S) {   // ゲーム中断
                    sysSetGameMode(false);
                    sysSetStep(stepGameOverInitWithoutReflectHiScore, stepGameOverMain);
                    return;
                }
            }
        }
#endif
        break;
    case OBJ_PLAYER_STEP_DEAD:
        break;
    case OBJ_PLAYER_STEP_CONTINUE:
        if (pPlayer->ct == 0) {// カウントダウン 0 ならゲームオーバー
            sysSetStep(stepGameOverInit, stepGameOverMain);
            return;
        } else if (inputGetTrigger() & INPUT_MASK_P) { // コンティニュー
            scoreContinue();
            objPlayerSetNormalStep(pPlayer);
        }
    } // switch (pPlayer->strp)
}
