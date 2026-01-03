/**
 * プレーヤー オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */
#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/input.h"
#include "../system/obj.h"
#include "../system/vvram.h"
#include "../system/print.h"
#include "../system/sound.h"
#include "../system/math.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "../game/game-mode.h"
#include "../game/se.h"
#include "../../cg/player.h"
#include "../../cg/nr0.h" // ‘コンティニュー用カウントダウン数字
#include "../../cg/nr1.h"
#include "../../cg/nr2.h"
#include "../../cg/nr3.h"
#include "../../cg/nr4.h"
#include "../../cg/nr5.h"
#include "../../cg/nr6.h"
#include "../../cg/nr7.h"
#include "../scenes/scene-game-mode.h"
#include "../scenes/scene-ending.h"
#include "obj-explosion.h"
#include "obj-player-bullet.h"
#include "obj-player.h"

// ---------------------------------------------------------------- 変数
static u8   obj_player_rapid_fire_timer_;// 連射タイマー

// ---------------------------------------------------------------- マクロ
#if DEBUG
#define DEBUG_INVINCIBLE 0      // 0/1 = 通常/無敵. デバッグ用
#define RAPID_FIRE_PERIOD 2     // 連射度. 小さいほど連射する. 0 なら自動連射
//#define RAPID_FIRE_PERIOD 0     // TEST デバッグ用
#else
#define DEBUG_INVINCIBLE 0      // 0/1 = 通常/無敵. デバッグ用
#define RAPID_FIRE_PERIOD 2     // 連射度. 小さいほど連射する. 0 なら自動連射
#endif

#define CT_NORMAL       100     // 無敵時間
#define CT_DEAD         50
#define CT_CONTINUE     255     // コンティニュー時間
#define PLAYER_X        19
#define PLAYER_Y        20
#define PLAYER_W        1
#define PLAYER_H        (CG_PLAYER_HEIGHT - 1)

// ---------------------------------------------------------------- 初期化
#pragma disable_warning 85  // p_parent 未使用
#pragma save
void objPlayerInit(Obj* const p_obj, Obj* const p_parent)
{
    OBJ_INIT(p_obj, PLAYER_X<<8, PLAYER_Y<<8, PLAYER_W, PLAYER_H, 0, 0);
    obj_player_rapid_fire_timer_ = 0;
    p_obj->offence   = 255;
    p_obj->step      = OBJ_PLAYER_STEP_NORMAL;
    p_obj->ct        = 0;
    p_obj->u_obj_work.player.attract_input = INPUT_MASK_A;
}
#pragma restore

// ---------------------------------------------------------------- メイン
bool objPlayerMain(Obj* const p_obj)
{
    u8 inp = inputGet();

    // -------- アトラクト モードでの入力データフック
    if (!sysIsGameMode()) {
        u8 att_inp = p_obj->u_obj_work.player.attract_input;
        if (240 < rand8()) {
            att_inp &= INPUT_MASK_A;
            att_inp |= rand8() & (INPUT_MASK_L | INPUT_MASK_R | INPUT_MASK_U | INPUT_MASK_D);
            p_obj->u_obj_work.player.attract_input = att_inp;
        }
        inp = att_inp;
    }

    switch (p_obj->step) {
    default:// case OBJ_PLAYER_STEP_NORMAL:
        // -------- 移動
        {
            s8 sx = 0;
            s8 sy = 0;
            if (inp & INPUT_MASK_L) {
                sx = -1;
            }
            if (inp & INPUT_MASK_R) {
                sx ++;
            }
            if (inp & INPUT_MASK_U) {
                sy = -1;
            }
            if (inp & INPUT_MASK_D) {
                sy ++;
            }
            p_obj->u_geo.geo8.sxh = sx;
            p_obj->u_geo.geo8.syh = sy;
        }

        // -------- 画面はみ出し
        if (p_obj->u_geo.geo8.xh < 1) {
            p_obj->u_geo.geo8.xh = 1;
        } else if (VRAM_WIDTH - 2 <= p_obj->u_geo.geo8.xh) {
            p_obj->u_geo.geo8.xh = VRAM_WIDTH - 2;
        }
        if (p_obj->u_geo.geo8.yh < 1) {
            p_obj->u_geo.geo8.yh = 1;
        } else if (VRAM_HEIGHT - CG_PLAYER_HEIGHT <= p_obj->u_geo.geo8.yh) {
            p_obj->u_geo.geo8.yh = VRAM_HEIGHT - CG_PLAYER_HEIGHT;
        }

        // -------- 無敵時間
        if (p_obj->ct) {
            p_obj->ct--;
            if (p_obj->ct == 0) {
                p_obj->u_geo.geo.w = PLAYER_W;
            }
        } else {
            if (p_obj->b_hit) {
#if DEBUG_INVINCIBLE != 1// 死亡処理
                p_obj->u_geo.geo.w = 0; // 衝突判定を無くす
                p_obj->step = OBJ_PLAYER_STEP_DEAD;
                p_obj->ct   = CT_DEAD;
                p_obj->u_geo.geo.sx = 0;
                p_obj->u_geo.geo.sy = 0;
                objCreateEtc(objExplosionPlayerInit, objExplosionMain, objExplosionPlayerDraw, p_obj);
                sdPlaySe(SE_PLAYER_DEAD);
#endif
            }
        }

        // -------- 弾
        // 押しっぱなしでも連射しますが, 連打のほうが沢山弾が出ます
#if RAPID_FIRE_PERIOD == 0
        objCreatePlayerBullet(objPlayerBulletInit, objPlayerBulletMain, objPlayerBulletDraw, p_obj);
#else
        if (inp & (INPUT_MASK_A | INPUT_MASK_B)) {
            if (RAPID_FIRE_PERIOD <= obj_player_rapid_fire_timer_) {
                objCreatePlayerBullet(objPlayerBulletInit, objPlayerBulletMain, objPlayerBulletDraw, p_obj);
                obj_player_rapid_fire_timer_ = 0;
            }
            obj_player_rapid_fire_timer_ ++;
        } else {
            obj_player_rapid_fire_timer_ = RAPID_FIRE_PERIOD;
        }
#endif

        // -------- キャラバン モード
        if (gameIsCaravan() && gameGetTimer() == 0) {
            p_obj->step = OBJ_PLAYER_STEP_END_CARAVAN;
            p_obj->ct = 0;
        }
        break;

    case OBJ_PLAYER_STEP_DEAD:
        // 死亡時間
        if (p_obj->ct) {
            p_obj->ct--;
            if (p_obj->ct == 0) {
                objPlayerSetNormalStep(p_obj);
                if (scoreDecrementLeft()) {
                    u8 game_mode = gameGetMode();
                    if (game_mode == GAME_MODE_SURVIVAL || game_mode == GAME_MODE_MUBO) {
                        p_obj->step = OBJ_PLAYER_STEP_END_SURVIVAL;// サバイバル/無謀モード
                        p_obj->ct   = 0;
                    } else {
                        p_obj->step = OBJ_PLAYER_STEP_CONTINUE;
                        p_obj->ct   = CT_CONTINUE;
                    }
                }
            }
        }
        break;

    case OBJ_PLAYER_STEP_CONTINUE:  // コンティニュー カウントダウン
        if (p_obj->ct != 0) {       // ゲーム オーバーへの画面遷移は sceneGame でやる
            if (inputGetTrigger() & (INPUT_MASK_CANCEL)) { // 時間短縮
                p_obj->ct = p_obj->ct & 0xe0;
                if (p_obj->ct == 0) {
                    p_obj->ct++;
                }
            }
            p_obj->ct--;
        }
        break;

    case OBJ_PLAYER_STEP_END_SURVIVAL:  // サバイバル/無謀 モード終了表示
    case OBJ_PLAYER_STEP_END_CARAVAN:   // キャラバン モード終了表示
        break;

    case OBJ_PLAYER_STEP_DEMO:  // 外で制御
        // 画面はみ出し & 1
        if (p_obj->u_geo.geo8.xh < 1) {
            p_obj->u_geo.geo8.xh = 1;
        } else if (VRAM_WIDTH - 2 <= p_obj->u_geo.geo8.xh) {
            p_obj->u_geo.geo8.xh = VRAM_WIDTH - 2;
        }
        p_obj->ct = 0;                      // 点滅停止
        if (p_obj->u_geo.geo8.yh < -3) {    // 画面外に出たら消える
            return false;
        }
        break;
    }

    return true;
}

// ---------------------------------------------------------------- 描画
static const u8* CONTINUE_TAB_[] = { cg_nr0, cg_nr1, cg_nr2, cg_nr3, cg_nr4, cg_nr5, cg_nr6, cg_nr7};
// 炎
static const u16 BACKFIRE_TAB1_[] = {
    VATB_CODE(2, 0, 0, 0xf1),
    VATB_CODE(6, 0, 0, 0xf2),
    VATB_CODE(2, 0, 0, 0xf3),
    VATB_CODE(2, 0, 0, 0xf4),
    VATB_CODE(6, 0, 0, 0xf5),
    VATB_CODE(2, 0, 0, 0xf6),
    VATB_CODE(6, 0, 0, 0xf8),
    VATB_CODE(2, 0, 0, 0xf9),
};
static const u16 BACKFIRE_TAB2_[] = {
    VATB_CODE(2, 0, 0, 0xf7),
    VATB_CODE(6, 0, 0, 0xf7),
    VATB_CODE(2, 0, 0, 0xfb),
    VATB_CODE(6, 0, 0, 0xfb),
    VATB_CODE(6, 0, 0, 0xfd),
    VATB_CODE(6, 0, 0, 0xfe),
    VATB_CODE(2, 0, 0, 0xff),
    VATB_CODE(6, 0, 0, 0xff),
};
void objPlayerDraw(Obj* const p_obj, u8* draw_addr)
{
    u8 ct = p_obj->ct;

    switch (p_obj->step) {
    default: //case OBJ_PLAYER_STEP_NORMAL: case OBJ_PLAYER_STEP_DEMO:
        draw_addr--;
        if (ct) {
            printReady();
        }
        ct &= 1;
        if (!ct) {
            vvramDraw3x3(draw_addr, cg_player);
            draw_addr += VVRAM_WIDTH * 2 + 1;
            vvramDraw1x1(draw_addr, BACKFIRE_TAB2_[rand8() & 0x07]);
            draw_addr += VVRAM_WIDTH;
            vvramDraw1x1(draw_addr, BACKFIRE_TAB1_[rand8() & 0x07]);
        }
        break;
    case OBJ_PLAYER_STEP_DEAD:
        break;
    case OBJ_PLAYER_STEP_CONTINUE: // カウントダウン表示
        {
#include "../../text/continue.h"
            printSetAddr((u8*)VVRAM_TEXT_ADDR(9, 7));
            printString(text_continue);
#pragma disable_warning 110 // 除算最適化警告
#pragma save
            vvramDrawRectTransparent((u8*)VVRAM_TEXT_ADDR(18, 10), CONTINUE_TAB_[p_obj->ct / 32], W8H8(4, 4));
#pragma restore
            if ((p_obj->ct % 32) == 31) {
                sdPlaySe(SE_CONTINUE);
            }
        }
        break;
    case OBJ_PLAYER_STEP_END_CARAVAN:  // キャラバン モード終了表示
        sceneEndingDispMisses((u8*)VVRAM_TEXT_ADDR(12, 14));
        // fall through
    case OBJ_PLAYER_STEP_END_SURVIVAL: // サバイバル/無謀 モード終了表示
        if (gameGetTimer() != 0) {
            sceneEndingDispSurviveTime((u8*)VVRAM_TEXT_ADDR(12, 14));
        }
        sceneEndingDispEnemies((u8*)VVRAM_TEXT_ADDR(12, 16));
        p_obj->u_geo.geo.w = 0; // 衝突判定を無くす
        {
#include "../../text/finish.h"
#include "../../text/game-end.h"
            sceneEndingDispFinish(VATB(7, 0, 0));
            sceneGamePrintGameMode((u8*)VVRAM_TEXT_ADDR(16, 9), gameGetMode(), false);
            printSetAddr((u8*)VVRAM_TEXT_ADDR(11, 20)); printString(text_game_end);
        }
        if (p_obj->ct == 0) {
#define L   12
#include "../../music/end.h"
            sd3Play(mml0_0, mml1_0, mml2_0, true);
            p_obj->ct = 1;
        }
        break;

    }
}

// ---------------------------------------------------------------- ユーティリティ
void objPlayerSetNormalStep(Obj* const p_player)
{
    p_player->u_geo.geo8.xh = PLAYER_X;
    p_player->u_geo.geo8.yh = PLAYER_Y;
    p_player->u_geo.geo8.w  = 0;
    p_player->step = OBJ_PLAYER_STEP_NORMAL;
    p_player->ct   = gameIsCaravan() ? CT_NORMAL / 4 : CT_NORMAL;  // キャラバン モードは無敵時間が短い(無敵時の荒っぽい戦いを無くすため)
}