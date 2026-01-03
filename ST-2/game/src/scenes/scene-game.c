/**
 * ゲーム シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "../system/sys.h"
#include "../system/addr.h"
#include "../system/input.h"
#include "../system/vvram.h"
#include "../system/print.h"
#include "../system/sound.h"
#include "../system/obj.h"
#include "../system/math.h"
#include "../game/score.h"
#include "../game/game-mode.h"
#include "../game/stage.h"
#include "../objworks/obj-player.h"
#include "../../cg/Pause.h"
#include "scene-stage-clear.h"
#include "scene-ending.h"
#include "scene-score-table.h"
#include "scene-game-over.h"
#include "scene-game.h"

#if DEBUG
static bool b_pause_;
#endif

// ---------------------------------------------------------------- 変数, マクロ
//static u8 sStageDrawTimer;
#define SYS_SCENE_WORK_DRAW_STAGE_TIMER   0 // STAGE 表示タイマ


// ---------------------------------------------------------------- 初期化
#if DEBUG
static const u8 SCENE_NAME_[] = { DC_G, DC_A, DC_M, DC_E, 0, };
#endif
void sceneGameInit(void)
{
#if DEBUG
    b_pause_ = false;
    scoreSetStepString(SCENE_NAME_);
#endif
    if (!sysIsGameMode()) {// アトラクト モード時
#if DEBUG
        stgInit(0);// デバッグ時はいきなりラスボスとかあるので安全の為に最初の実表示
#else
        stgInit(sceneScoreTableGetLoopCt());// アトラクトモード時はループします
#endif
        objCreatePlayer(objPlayerInit, objPlayerMain, objPlayerDraw, nullptr);
        sysSetSceneCounter(1000);
    } else {
        // ゲームのルール上, スコア0点でクリアすることは不可能
        if (scoreGet() == 0) {
            stgInit(0);
        }
        sdSetEnabled(true);
    }

    stgSubInit();
    sysSceneSetWork(SYS_SCENE_WORK_DRAW_STAGE_TIMER, 100);

    //sStageDrawTimer = 100;
}


// ---------------------------------------------------------------- メイン
static const u8 STR_STAGE_[] = { DC_S, DC_T, DC_A, DC_G, DC_E, 0, };
static const u8 MML0_[] = { SD3_C3, 4, SD3_G3, 4, SD3_E3, 8, 0, };
void sceneGameMain(u16 scene_ct)
{
    // ---------------- アトラクト モードでの挙動. ステージ進行, タイマ減算処理
    if (!sysIsGameMode()) {
        if (stgGetNrEnemies() == 0) {
            // -------- ステージ進行. 敵が 0 になったら次のステージへ
            switch (stgSubInit()) {
            default: break;
            case STG_STATUS_CLEAR:
                sysSetScene(sceneScoreTableInit, sceneScoreTableMain);
                return;
            }
        }
        if (scene_ct == 0) { // 時間切れでのシーンへ. プレーヤーは何度死んでもいい
            sysSetScene(sceneScoreTableInit, sceneScoreTableMain);
        }
        return;
    }

    // ---------------- ゲーム モードでの挙動. ステージ数表示
    if (sysSceneGetWork(SYS_SCENE_WORK_DRAW_STAGE_TIMER)) {
        sysSceneDecWork(SYS_SCENE_WORK_DRAW_STAGE_TIMER);
        printSetAddr((u8*)VVRAM_TEXT_ADDR(16, 8));
        printString(STR_STAGE_);
        printU8Right(stgGetStageNr());
        printReady();
    }

    // ---------------- ゲーム モードでの挙動. ポーズ, シーン, コンティニュー, ステージ進行
    Obj* const p_player = objGetInUsePlayer();
    if (p_player == nullptr) { return; }

    switch (p_player->step) {
    default: // OBJ_PLAYER_STEP_NORMAL
        // -------- ステージ進行. 敵が 0 になったら次のステージへ
        // プレーヤーがたまたま死んだ判定があるかもしれないのでそれもチェック
        if ((stgGetNrEnemies() == 0) && (!p_player->b_hit)) {
            switch (stgSubInit()) {
            default: //case STG_STATUS_OK:
                break;
            case STG_STATUS_CLEAR:
                objInitEnemyBullet();
                sysSetScene(sceneStageClearInit, sceneStageClearMain);
                return;
            case STG_STATUS_ENDING:
                objInitEnemyBullet();
                sysSetScene(sceneEndingInit, sceneEndingMain);
                return;
            }
        }

#if DEBUG
        // デバッグ時ポーズ, シーン動作, ゲーム中断
        if (b_pause_ || (inputGetTrigger() & INPUT_MASK_START)) {
            sd3Play(MML0_, MML0_, MML0_, false);
            while (true) {
                inputMain();
                u8 inp = inputGetTrigger();
                if (inp & INPUT_MASK_START) {// ポーズ解除
                    b_pause_ = false;
                    sd3Play(MML0_, MML0_, MML0_, false);
                    break;
                }
                if (inp & INPUT_MASK_B) {   // シーン動作
                    b_pause_ = true;
                    break;
                }
                if (inp & INPUT_MASK_CANCEL) { // ゲーム中断
                    sysSetGameMode(false);
                    sysSetScene(sceneGameOverInitWithoutReflectHiScore, sceneGameOverMain);
                    return;
                }
            } // while (true)
        }
#else
        // リリース時ポーズ, ゲーム中断
        if (inputGetTrigger() & INPUT_MASK_START) {
            vramDrawRect((u8*)VRAM_TEXT_ADDR((u16)(VRAM_WIDTH - CG_PAUSE_WIDTH) / 2, 10), cg_Pause, W8H8(CG_PAUSE_WIDTH, CG_PAUSE_HEIGHT));
            sd3Play(MML0_, MML0_, MML0_, false);
            while (true) {
                inputMain();
                u8 inp = inputGetTrigger();
                if (inp & INPUT_MASK_START) {   // ポーズ解除
                    sd3Play(MML0_, MML0_, MML0_, false);
                    break;
                }
                if (inp & INPUT_MASK_CANCEL) { // ゲーム中断
                    sysSetGameMode(false);
                    sysSetScene(sceneGameOverInitWithoutReflectHiScore, sceneGameOverMain);
                    return;
                }
            } // while (true)
        }
#endif
        // fall through
    case OBJ_PLAYER_STEP_DEAD:
            if (gameIsCaravan()) {          // キャラバン モード
                gameDecTimer();             // 時間切れ検出は objPlayer で
            } else if (!gameCanIncLeft()) { // サバイバル, むぼう
                gameIncTimer();
            }
        break;
    case OBJ_PLAYER_STEP_CONTINUE:
        if (p_player->ct == 0) {// カウントダウン 0 ならゲームオーバー
            sysSetScene(sceneGameOverInit, sceneGameOverMain);
            return;
        } else if (inputGetTrigger() & INPUT_MASK_START) { // コンティニュー
            scoreContinue();
            objPlayerSetNormalStep(p_player);
        }
        break;
    case OBJ_PLAYER_STEP_END_SURVIVAL: // サバイバル モード終了表示
    case OBJ_PLAYER_STEP_END_CARAVAN:  // キャラバン モード終了表示
        if (inputGetTrigger() & INPUT_MASK_START) { // ゲームオーバー
            sysSetScene(sceneGameOverInit, sceneGameOverMain);
            return;
        }
        break;
    } // switch (p_player->strp)
}
