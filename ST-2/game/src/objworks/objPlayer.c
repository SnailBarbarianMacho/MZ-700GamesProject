/**
 * プレーヤー オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */
#include "../../../../src-common/common.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/input.h"
#include "../system/obj.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/sound.h"
#include "../system/math.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "../../cg/Player.h"
#include "../../cg/Nr0.h" // ‘コンティニュー用カウントダウン数字
#include "../../cg/Nr1.h"
#include "../../cg/Nr2.h"
#include "../../cg/Nr3.h"
#include "../../cg/Nr4.h"
#include "../../cg/Nr5.h"
#include "../../cg/Nr6.h"
#include "../../cg/Nr7.h"
#include "objExplosion.h"
#include "objPlayerBullet.h"
#include "objPlayer.h"

// ---------------------------------------------------------------- 変数
static u8   sRapidFireTimer;// 連射タイマー

// ---------------------------------------------------------------- マクロ
#define DEBUG_INVINCIBLE 0      // 1 だと無敵. デバッグ用
#define RAPID_FIRE_PERIOD 2     // 連射度. 小さいほど連射する
#define CT_NORMAL       100     // 無敵時間
#define CT_DEAD         50
#define CT_CONTINUE     255     // コンティニュー時間
#define PLAYER_X        19
#define PLAYER_Y        20
#define PLAYER_W        1
#define PLAYER_H        (CG_PLAYER_HEIGHT - 1)

// ---------------------------------------------------------------- サウンド
#define SE_PLAYER_DEAD_CT 48
#define SE_CONTINUE_CT    10
// 敵ダメージ音
static void sePlayerDead(u8 ct)
{
    sdMake(((10 - (ct & 7)) << 10) + rand8());
}
static void seContinue(u8 ct)
{
    sdMake((ct & 1) ? 0x0000 : 0x0200);
}

// ---------------------------------------------------------------- 初期化
#pragma disable_warning 85  // pParent 未使用
#pragma save
void objPlayerInit(Obj* const pObj, Obj* const pParent)
{
    OBJ_INIT(pObj, PLAYER_X<<8, PLAYER_Y<<8, PLAYER_W, PLAYER_H, 0, 0);
    sRapidFireTimer = 0;
    pObj->offence   = 255;
    pObj->step      = OBJ_PLAYER_STEP_NORMAL;
    pObj->ct        = 0;
    pObj->uObjWork.player.attractInput = INPUT_MASK_A;
}
#pragma restore

// ---------------------------------------------------------------- メイン
bool objPlayerMain(Obj* const pObj)
{
    u8 inp = inputGet();

    // アトラクト モードでの入力データフック
    if (!sysIsGameMode()) {
        u8 attInp = pObj->uObjWork.player.attractInput;
        if (240 < rand8()) {
            attInp &= INPUT_MASK_A;
            attInp |= rand8() & (INPUT_MASK_L | INPUT_MASK_R | INPUT_MASK_U | INPUT_MASK_D);
            pObj->uObjWork.player.attractInput = attInp;
        }
        inp = attInp;
    }

    switch (pObj->step) {
    default:// case OBJ_PLAYER_STEP_NORMAL:
        // 移動
        {
            s8 sx = 0;
            s8 sy = 0;
            if (inp & INPUT_MASK_L) {
                sx = -1;
            }
            if (inp & INPUT_MASK_R) {
                sx = 1;
            }
            if (inp & INPUT_MASK_U) {
                sy = -1;
            }
            if (inp & INPUT_MASK_D) {
                sy = 1;
            }
            pObj->uGeo.geo8.sxh = sx;
            pObj->uGeo.geo8.syh = sy;
        }

        // 画面はみ出し
        if (pObj->uGeo.geo8.xh < 1) {
            pObj->uGeo.geo8.xh = 1;
        } else if (VRAM_WIDTH - 2 <= pObj->uGeo.geo8.xh) {
            pObj->uGeo.geo8.xh = VRAM_WIDTH - 2;
        }
        if (pObj->uGeo.geo8.yh < 1) {
            pObj->uGeo.geo8.yh = 1;
        } else if (VRAM_HEIGHT - CG_PLAYER_HEIGHT <= pObj->uGeo.geo8.yh) {
            pObj->uGeo.geo8.yh = VRAM_HEIGHT - CG_PLAYER_HEIGHT;
        }

        // 無敵時間
        if (pObj->ct) {
            pObj->ct--;
            if (pObj->ct == 0) {
                pObj->uGeo.geo.w = PLAYER_W;
            }
        } else {
            if (pObj->bHit) {
#if DEBUG_INVINCIBLE != 1// 死亡処理
                pObj->uGeo.geo.w = 0;
                pObj->step = OBJ_PLAYER_STEP_DEAD;
                pObj->ct   = CT_DEAD;
                pObj->uGeo.geo.sx = 0;
                pObj->uGeo.geo.sy = 0;
                objCreateEtc(objExplosionPlayerInit, objExplosionMain, objExplosionPlayerDisp, pObj);
                sdSetSeSequencer(sePlayerDead, SD_SE_PRIORITY_1, SE_PLAYER_DEAD_CT);
#endif
            }
        }

        // 弾
        // 押しっぱなしでも連射しますが, 連打のほうが沢山弾が出ます
        if (inp & INPUT_MASK_A) {
            if (RAPID_FIRE_PERIOD <= sRapidFireTimer) {
                objCreatePlayerBullet(objPlayerBulletInit, objPlayerBulletMain, objPlayerBulletDisp, pObj);
                sRapidFireTimer = 0;
            }
            sRapidFireTimer ++;
        } else {
            sRapidFireTimer = RAPID_FIRE_PERIOD;
        }
        break;

    case OBJ_PLAYER_STEP_DEAD:
        // 死亡時間
        if (pObj->ct) {
            pObj->ct--;
            if (pObj->ct == 0) {
                objPlayerSetNormalStep(pObj);
                if (scoreDecrementLeft()) {
                    pObj->step = OBJ_PLAYER_STEP_CONTINUE;
                    pObj->ct   = CT_CONTINUE;
                }
            }
        }
        break;

    case OBJ_PLAYER_STEP_CONTINUE:
        if (pObj->ct != 0) {  // ゲーム オーバーへの画面遷移は stepGame でやる
            if (inputGetTrigger() & INPUT_MASK_A) { // 時間短縮
                pObj->ct = pObj->ct & 0xe0;
                if (pObj->ct == 0) {
                    pObj->ct++;
                }
            }
            pObj->ct--;
        }
        break;

    case OBJ_PLAYER_STEP_DEMO:  // 外で制御
        if (pObj->uGeo.geo8.yh < -3) {  // 画面外に出たら消える
            return false;
        }
        break;
    }

    return true;
}

// ---------------------------------------------------------------- 描画
void objPlayerDisp(Obj* const pObj, u8* dispAddr)
{
    u8 ct = pObj->ct;

    switch (pObj->step) {
    default: //case OBJ_PLAYER_STEP_NORMAL: case OBJ_PLAYER_STEP_DEMO:
        dispAddr--;
        if (ct) {
            printReady();
        }
        ct &= 1;
        if (!ct) {
            vVramDraw3x3(dispAddr, sPlayer);
            // 炎
            static const u16 tab1[] = {
                VATB_CODE(2, 0, 0, 0xf1),
                VATB_CODE(6, 0, 0, 0xf2),
                VATB_CODE(2, 0, 0, 0xf3),
                VATB_CODE(2, 0, 0, 0xf4),
                VATB_CODE(6, 0, 0, 0xf5),
                VATB_CODE(2, 0, 0, 0xf6),
                VATB_CODE(6, 0, 0, 0xf8),
                VATB_CODE(2, 0, 0, 0xf9),
            };
            static const u16 tab2[] = {
                VATB_CODE(2, 0, 0, 0xf7),
                VATB_CODE(6, 0, 0, 0xf7),
                VATB_CODE(2, 0, 0, 0xfb),
                VATB_CODE(6, 0, 0, 0xfb),
                VATB_CODE(6, 0, 0, 0xfd),
                VATB_CODE(6, 0, 0, 0xfe),
                VATB_CODE(2, 0, 0, 0xff),
                VATB_CODE(6, 0, 0, 0xff),
            };
            dispAddr += VVRAM_WIDTH * 2 + 1;
            vVramDraw1x1(dispAddr, tab2[rand8() & 0x07]);
            dispAddr += VVRAM_WIDTH;
            vVramDraw1x1(dispAddr, tab1[rand8() & 0x07]);
        }
        break;
    case OBJ_PLAYER_STEP_DEAD:
        break;
    case OBJ_PLAYER_STEP_CONTINUE:
        {
#include "../../text/continue.h"
            printSetAddr((u8*)VVRAM_TEXT_ADDR(7, 8));
            printString(str_continue);
            static const u8* tab[] = { sNr0, sNr1, sNr2, sNr3, sNr4, sNr5, sNr6, sNr7};
            vVramDrawRectTransparent((u8*)VVRAM_TEXT_ADDR(18, 10), tab[pObj->ct / 32], W8H8(4, 4));
            if ((pObj->ct % 32) == 31) {
                sdSetSeSequencer(seContinue, SD_SE_PRIORITY_1, SE_CONTINUE_CT);
            }
        }
        break;
    }
}

// ---------------------------------------------------------------- ユーティリティ
void objPlayerSetNormalStep(Obj* const pPlayer)
{
    pPlayer->uGeo.geo8.xh = PLAYER_X;
    pPlayer->uGeo.geo8.yh = PLAYER_Y;
    pPlayer->uGeo.geo8.w  = 0;
    pPlayer->step = OBJ_PLAYER_STEP_NORMAL;
    pPlayer->ct   = CT_NORMAL;
}