/**
 * エンディング シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../system/sys.h"
#include "../system/sound.h"
#include "../system/print.h"
#include "../system/vram.h"
#include "../system/math.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "../game/se.h"
#include "../game/game_mode.h"
#include "../game/stars.h"
#include "../objworks/obj_player.h"
#include "../objworks/obj_enemy.h"
#include "../objworks/obj_item.h"
#include "scene_logo.h"
#include "scene_game_mode.h"
#include "scene_ending.h"

// ---------------------------------------------------------------- 変数, マクロ
#define ENDING_CT        200
#define ENDING_CT_FINISH 120
#define ENDING_CT_DEMO   80

#define SYS_SCENE_WORKSTR_INDEX 0

// ---------------------------------------------------------------- 初期化
void sceneEndingInit()
{
#if DEBUG
    static const u8 str[] = { DC_E, DC_N, DC_D, 0, };
    scoreSetStepString(str);
#endif
    // プレーヤー去る
    Obj* p_player = objGetInUsePlayer();
    p_player->step          = OBJ_PLAYER_STEP_DEMO;
    p_player->u_geo.geo.sx   =  0x0000;
    p_player->u_geo.geo.sy   = -0x0080;

    objInitItem(); // アイテムは全部消す

    sdSetEnabled(true);
    sdPlaySe(SE_END);
    sysSetSceneCounter(ENDING_CT);
}


// ---------------------------------------------------------------- メイン
void sceneEndingMain(u16 scene_ct)
{
    if (ENDING_CT_FINISH < scene_ct) {
        sceneEndingDispFinish(VATB(scene_ct & 0x07, 0, 0));
    } else if (scene_ct <= ENDING_CT_DEMO) { // 文字は 1 フレーム先から表示
        starsInit();
#define L 16
#include "../../music/ending_bar8.h"
        static const u8* SD_TAB0[] = { nullptr, mml0_0, mml0_1, mml0_2, mml0_3, mml0_4, mml0_5, };
        static const u8* SD_TAB1[] = { nullptr, mml1_0, mml1_1, mml1_2, mml1_3, mml1_4, mml1_5, };
        static const u8* SD_TAB2[] = { nullptr, mml2_0, mml2_1, mml2_2, mml2_3, mml2_4, mml2_5, };

#include "../../text/ending1.h"
#include "../../text/ending2.h"
#include "../../text/ending3.h"
#include "../../text/ending4.h"
#include "../../text/ending5.h"
#include "../../text/ending6.h"
        static const u8* STR_TAB[] = { text_ending1, text_ending2, text_ending3, text_ending4, text_ending5, text_ending6, nullptr, };
        u8 str_idx = sysSceneGetWork(SYS_SCENE_WORKSTR_INDEX);
        if (str_idx < COUNT_OF(STR_TAB)) {
            if (SD_TAB0[str_idx]) {
#if DEBUG
                sd3Play(SD_TAB0[str_idx], SD_TAB1[str_idx], SD_TAB2[str_idx], true);
#else
                sd3Play(SD_TAB0[str_idx], SD_TAB1[str_idx], SD_TAB2[str_idx], false);// エンディングBGMはスキップできない
#endif
            }

            if (STR_TAB[str_idx]) {
                printSetAddr((u8*)VVRAM_TEXT_ADDR(6, 5));
                printString(STR_TAB[str_idx]);
                if (str_idx == 5) {
                    sceneEndingDispGameResults((u8*)VVRAM_TEXT_ADDR(10, 9));
                    sceneGamePrintGameMode(    (u8*)VVRAM_TEXT_ADDR(15, 11), gameGetMode(), false);
                    sceneEndingDispContinues(  (u8*)VVRAM_TEXT_ADDR(10, 15));
                    sceneEndingDispMisses(     (u8*)VVRAM_TEXT_ADDR(10, 17));
                    sceneEndingDispItems(      (u8*)VVRAM_TEXT_ADDR(10, 19));
                    sceneEndingDispEnemies(    (u8*)VVRAM_TEXT_ADDR(10, 21));
                }
            } else {
                sdPlaySe(SE_END_GAME_OVER);
                sysSetMode(false);
            }
            sysSceneIncWork(SYS_SCENE_WORKSTR_INDEX);
        }
    }

    if (scene_ct == 0)  {
        scoreReflectHiScore();
        scoreResetLevel();
        sysSetScene(sceneLogoInit, sceneLogoMain);
        //sysSetScene(sceneEndingInit, sceneEndingMain);//TEST
    }
}

// ---------------------------------------------------------------- 統計結果表示
void sceneEndingDispFinish(const u8 atb) __z88dk_fastcall
{
#include "../../text/finish.h"
    printSetAtb(atb);
    printSetAddr((u8*)VVRAM_TEXT_ADDR(16, 7));
    printString(text_finish);
}

void sceneEndingDispGameResults(u8* const disp_addr) __z88dk_fastcall
{
#include "../../text/game_results.h"
    printSetAddr(disp_addr);
    printString(text_game_results);
}

void sceneEndingDispContinues(u8* const disp_addr) __z88dk_fastcall
{
#include "../../text/ending6_continue.h"
    printSetAddr(disp_addr);
    printString(text_ending6_continue);
    printU16Left(scoreGetNrContinues());
}

void sceneEndingDispMisses(u8* const disp_addr) __z88dk_fastcall
{
#include "../../text/ending6_miss.h"
    printSetAddr(disp_addr);
    printString(text_ending6_miss);
    printU16Left(scoreGetNrMisses());
}

void sceneEndingDispItems(u8* const disp_addr) __z88dk_fastcall
{
#include "../../text/ending6_items.h"
    printSetAddr(disp_addr);
    printString(text_ending6_items);
    u16 nr_gen_items = objItemGetNrGenereted();// 生成アイテム数は必ず 0 以上になります
    u16 nr_obt_items = objItemGetNrObtained();
    //nrGenItems = 7400;//TEST
    //nrObtItems = 6000;//TEST
    printU16Left(nr_obt_items);
    printPutc(DC_SLASH);
    printU16Left(nr_gen_items);
    printPutc(DC_EQUAL);
    nr_gen_items = (u32)nr_obt_items * 100 / nr_gen_items;// div32 は使いたくないが, やむを得なん!!
    printU16Left(nr_gen_items);
    printPutc(DC_PERCENT);
}

void sceneEndingDispEnemies(u8* const disp_addr) __z88dk_fastcall
{
#include "../../text/ending6_enemies.h"
    printSetAddr(disp_addr);
    printString(text_ending6_enemies);
    u16 nr_killed = objEnemyGetNrKilled();
    printU16Left(nr_killed);
}
