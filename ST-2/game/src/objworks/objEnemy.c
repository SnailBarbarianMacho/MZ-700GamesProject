/**
 * 敵オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */
#include "../../../../src-common/common.h"
#include "../system/sys.h"
#include "../system/obj.h"
#include "../system/vram.h"
#include "../system/print.h"// DEBUG
#include "../system/sound.h"
#include "../system/math.h"
#include "../game/score.h"
#include "../game/stage.h"
#include "../../cg/Enemy3_1.h"
#include "../../cg/Enemy3_2.h"
#include "../../cg/Enemy3_3.h"
#include "../../cg/Enemy3_4.h"
#include "../../cg/Enemy3_5.h"
#include "../../cg/Enemy3_6.h"
#include "../../cg/Enemy4_1.h"
#include "../../cg/Enemy4_2.h"
#include "../../cg/Enemy4_3.h"
#include "../../cg/Enemy5_1.h"
#include "../../cg/Enemy5_2.h"
#include "../../cg/Enemy5_3.h"
#include "../../cg/Enemy8_1.h"
#include "../../cg/EnemyDamage_3.h"
#include "../../cg/EnemyDamage_3_6.h"
#include "../../cg/EnemyDamage_4.h"
#include "../../cg/EnemyDamage_5.h"
#include "../../cg/EnemyDamage_8.h"
#include "../../cg/EnemyRed_8.h"
#include "objItem.h"
#include "objPlayer.h"
#include "objExplosion.h"
#include "objEnemyBullet.h"
#include "objScore.h"
#include "objEnemy.h"

// ---------------------------------------------------------------- マクロ
#define INVINCIBLE_TIME 64
#define STEP_MAIN  0
#define STEP_DEAD  1
#define BULLET_MODE_NONE         0
#define BULLET_MODE_FIRE_ON_MOVE 1 // 方向が変わると弾を吐く
#define BULLET_MODE_FIRE_ON_DEAD 2 // 撃ち返し

#define SCORE_3_1 10
#define SCORE_3_2 15
#define SCORE_3_3 20
#define SCORE_3_4 25
#define SCORE_3_5 30
#define SCORE_3_6 40
#define SCORE_4_1 50
#define SCORE_4_2 60
#define SCORE_4_3 70
#define SCORE_5_1 80
#define SCORE_5_2 90
#define SCORE_5_3 100
#define SCORE_8_1 100
#define SCORE_8_2 200
#define SCORE_8_3 2000

#define FITNESS_3_1 1
#define FITNESS_3_2 8
#define FITNESS_3_3 20
#define FITNESS_3_4 30
#define FITNESS_3_5 50
#define FITNESS_3_6 60
#define FITNESS_4_1 250
#define FITNESS_4_2 250
#define FITNESS_4_3 350
#define FITNESS_5_1 1200
#define FITNESS_5_2 2000
#define FITNESS_5_3 3000
#define FITNESS_8_1 700
#define FITNESS_8_2 5000
#define FITNESS_8_3 33000

// アイテム数は, 「nrItems + rand8() % nrItems」個
#define H4_ITEM_SUB_LEVEL4_NR8(h, subLevel, nrItems) (((h) << 12) | ((subLevel) << 8) | (nrItems))


// ---------------------------------------------------------------- サウンド
#define SE_DEAD3_CT     20
#define SE_DEAD45_CT    64
#define SE_DEAD8_CT     128
#define SE_DAMAGE_CT    3

// 雑魚死亡音
static void seDead3(u8 ct)
{
    static const u8 tab[SE_DEAD3_CT] = {
        0x15, 0x1d, 0x1a, 0x1b, 0x1a, 0x13, 0x12, 0x12, 0x14, 0x13, 0x11, 0x12, 0x11, 0x0f, 0x0e, 0x0d, 0x07, 0x0b, 0x08, 0x05,
    };
    sdMake(tab[ct] << 10);
}

// 中ボス死亡音
static void seDead45(u8 ct)
{
    sdMake((20 - (ct & 15)) << 10);
}

// ラスボス死亡音
static void seDead8(u8 ct)
{
    sdMake((22 - (ct & 7)) << 10);
}

// 敵ダメージ音
static void seDamage(u8 ct)
{
    sdMake((SE_DAMAGE_CT - ct + 1) << 7);
}

// ---------------------------------------------------------------- 初期化
/**
 * @param hItem H4_ITEM_SUB_LEVEL4_NR8 マクロを使ってください
 */
static void initEnemySub(Obj* const pObj, const u16 fitness, const u16 score, const u16 hItem)
{
    u8 h = hItem >> 12;
    OBJ_INIT(pObj, (rand8() % (VRAM_WIDTH + 1 - h)) << 8, (rand8() % (VRAM_HEIGHT + 1 - h)) << 8, 0, h, 0, 0);

    pObj->ct      = INVINCIBLE_TIME; // 無敵時間 ct は負の値は無敵, 0 以上は方向カウンタ
    pObj->step    = STEP_MAIN;
    pObj->fitness = fitness;
    pObj->uObjWork.enemy.score        = score;
    pObj->uObjWork.enemy.itemSubLevel = (hItem >> 8) & 0x0f;
    pObj->uObjWork.enemy.nrItems      = hItem;
    pObj->uObjWork.enemy.ct           = rand8() & 0x1f;
    pObj->uObjWork.enemy.animCt       = rand8();
    pObj->uObjWork.enemy.dir          = 0;
    pObj->uObjWork.enemy.bulletMode   = BULLET_MODE_NONE;
}

#pragma disable_warning 85  // pParent 未使用
#pragma save
// テーブル化するより, 個別に initEnemySub() を呼んだ方がコード量が小さくなる...
void objEnemyInit3_1(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_3_1, SCORE_3_1, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY3_1_HEIGHT, 1, 1));
}
void objEnemyInit3_2(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_3_2, SCORE_3_2, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY3_2_HEIGHT, 1, 2));
}
void objEnemyInit3_3(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_3_3, SCORE_3_3, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY3_3_HEIGHT, 2, 2));
}
void objEnemyInit3_4(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_3_4, SCORE_3_4, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY3_4_HEIGHT, 2, 3));
}
void objEnemyInit3_5(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_3_5, SCORE_3_5, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY3_5_HEIGHT, 3, 1));
}
void objEnemyInit3_6(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_3_6, SCORE_3_6, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY3_6_HEIGHT, 3, 2));
}
void objEnemyInit4_1(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_4_1, SCORE_4_1, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY4_1_HEIGHT, 1, 6));// 安っぽいアイテムを沢山ばら撒く
}
void objEnemyInit4_2(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_4_2, SCORE_4_2, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY4_2_HEIGHT, 4, 5));// 強敵なのでアイテム沢山あげる
    pObj->uObjWork.enemy.bulletMode = BULLET_MODE_FIRE_ON_MOVE;
}
void objEnemyInit4_3(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_4_3, SCORE_4_3, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY4_3_HEIGHT, 5, 2));
}
void objEnemyInit5_1(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_5_1, SCORE_5_1, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY5_1_HEIGHT, 5, 5));// ここいらへんからアイテムは乱発!
    pObj->uObjWork.enemy.bulletMode = BULLET_MODE_FIRE_ON_DEAD;
}
void objEnemyInit5_2(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_5_2, SCORE_5_2, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY5_2_HEIGHT, 6, 5));
}
void objEnemyInit5_3(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_5_3, SCORE_5_3, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY5_3_HEIGHT, 7, 7));
    pObj->uObjWork.enemy.bulletMode = BULLET_MODE_FIRE_ON_MOVE;
}
void objEnemyInit8_1(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_8_1, SCORE_8_1, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY8_1_HEIGHT, 7, 30));
}
void objEnemyInit8_2(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_8_2, SCORE_8_2, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY8_1_HEIGHT, 7, 30));
}
void objEnemyInit8_3(Obj* const pObj, Obj* const pParent)
{
    initEnemySub(pObj, FITNESS_8_3, SCORE_8_3, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY8_1_HEIGHT, 7, 30));
    pObj->uObjWork.enemy.bulletMode = BULLET_MODE_FIRE_ON_MOVE;
}
#pragma restore

// ---------------------------------------------------------------- メイン サブ関数(画面はみ出し, 死亡チェック)
static void overhangDeadCheck3(Obj* const pObj) __z88dk_fastcall
{
    //if (3 <= soundGetPriority()) {
    //    sdMake(((((u8)pObj &0x3f) + 1) << 8) + pObj->uGeo.geo8.yh);
    //}

    // -------- 画面はみ出し処理
    if (pObj->uGeo.geo8.xh < 0) {
        pObj->uGeo.geo8.xh = 0;
    } else if (VRAM_WIDTH - CG_ENEMY3_1_WIDTH < pObj->uGeo.geo8.xh) {
        pObj->uGeo.geo8.xh = VRAM_WIDTH - CG_ENEMY3_1_WIDTH;
    }
    if (pObj->uGeo.geo8.yh < 0) {
        pObj->uGeo.geo8.yh = 0;
    } else if (VRAM_HEIGHT - CG_ENEMY3_1_HEIGHT < pObj->uGeo.geo8.yh) {
        pObj->uGeo.geo8.yh = VRAM_HEIGHT - CG_ENEMY3_1_HEIGHT;
    }

    // -------- 死亡チェック
    if (pObj->fitness == 0) {
        // アイテムの生成
        u8 nrItems = pObj->uObjWork.enemy.nrItems;
        for (u8 i = (rand8() % nrItems) + nrItems; i != 0; i--) {
            if (!objCreateItem(objItemInit, objItemMain, objItemDisp, pObj)) { break; }
        }

        // 爆発, スコア, 敵減
        objCreateEtc(objExplosionInit, objExplosionMain, objExplosionDisp, pObj);
        scoreAdd(pObj->uObjWork.enemy.score);
        sdSetSeSequencer(seDead3, SD_SE_PRIORITY_2, SE_DEAD3_CT);
        pObj->uGeo.geo.sx = 0;// 移動停止
        pObj->uGeo.geo.sy = 0;// 移動停止
        pObj->uGeo.geo.w  = 0;// 衝突判定を止める
        pObj->ct   = 1;       // 表示しない
        pObj->uObjWork.enemy.ct = 10;
        pObj->step = STEP_DEAD;
    } else if (pObj->bHit) {
        sdSetSeSequencer(seDamage, SD_SE_PRIORITY_3, SE_DAMAGE_CT);// ダメージ音
    }
}
static void deadCheck458(Obj* const pObj) __z88dk_fastcall
{
    //if (3 <= soundGetPriority()) {
    //    sdMake(((((u8)pObj &0x3f) + 1) << 8) + pObj->uGeo.geo8.yh);
    //}
    if (pObj->fitness == 0) {
        // アイテムの生成
        u8 nrItems = pObj->uObjWork.enemy.nrItems;
        for (u8 i = (rand8() % nrItems) + nrItems; i != 0; i--) {
            if (!objCreateItem(objItemInit, objItemMain, objItemDisp, pObj)) { break; }
        }
         // 撃ち返し!
        if (pObj->uObjWork.enemy.bulletMode == BULLET_MODE_FIRE_ON_DEAD) {
            objCreateEnemyBullet(objEnemyBulletInit, objEnemyBulletMain, objEnemyBulletDisp, pObj);
        }
        // スコア処理
        u16 score = pObj->uObjWork.enemy.score;
        scoreAdd(score);
        if (100 <= score) { // 1000 点以上はスコアを表示
            objCreateEtc(objScoreInit, objScoreMain, objScoreDisp, pObj);
        }
        if (pObj->uGeo.geo.w == 8) {
            pObj->ct = 120;
            sdSetSeSequencer(seDead8, SD_SE_PRIORITY_0, SE_DEAD8_CT);
        } else {
            pObj->ct = 30;
            sdSetSeSequencer(seDead45, SD_SE_PRIORITY_1, SE_DEAD45_CT);
        }
        pObj->uGeo.geo.sx = 0;// 移動停止
        pObj->uGeo.geo.sy = 0;// 移動停止
        pObj->uGeo.geo.w  = 0;// 衝突判定を止める
        pObj->step = STEP_DEAD;
    } else if (pObj->bHit) {
        sdSetSeSequencer(seDamage, SD_SE_PRIORITY_3, SE_DAMAGE_CT);// ダメージ音
    }
}
// ---------------------------------------------------------------- メイン サブ関数(死亡処理)
static bool dead3(Obj* const pObj) __z88dk_fastcall
{
    pObj->uObjWork.enemy.ct--;
    if (pObj->uObjWork.enemy.ct == 0) {
        stgDecrementEnemies();
        return false;
    }
    return true;
}
static bool dead45(Obj* const pObj) __z88dk_fastcall
{
    pObj->ct--;
    if ((pObj->ct & 3) == 0) {
        objCreateEtc(objExplosionInit, objExplosionMain, objExplosionDisp, pObj);
    }
    if (pObj->ct == 0) {
        stgDecrementEnemies();
        return false;
    }
    return true;
}

// ---------------------------------------------------------------- メイン サブ関数(無敵, 弾出し)
// 無敵チェック
static bool invincibleCheck(Obj* const pObj) __z88dk_fastcall
{
    pObj->uObjWork.enemy.animCt++;

    if (pObj->ct) {
        pObj->ct--;
        if (pObj->ct == 0) {
            pObj->uGeo.geo8.w = pObj->uGeo.geo8.h;
        }
        return true;
    }
    return false;
}

// 敵の数に応じて弾を吐く
static void createEnemyBullet(Obj* const pObj) __z88dk_fastcall
{
    static const u8 tab[] = {
        0x01, 0x01, 0x03, 0x03,  0x07, 0x07, 0x07, 0x0f,  0x0f, 0x0f, 0x1f, 0x1f,  0xff, 0xff, 0xff, 0xff
    };
    if (pObj->uObjWork.enemy.ct & tab[stgGetNrEnemies()]) {
        return;
    }
    objCreateEnemyBullet(objEnemyBulletInit, objEnemyBulletMain, objEnemyBulletDisp, pObj);
}

// ---------------------------------------------------------------- メイン サブ関数(移動系)
// 低速ランダム
static void moveA(Obj* const pObj) __z88dk_fastcall
{
    pObj->uObjWork.enemy.ct--;
    if (pObj->uObjWork.enemy.ct) { return; }
    pObj->uObjWork.enemy.ct = (rand8() & 0x1f) + 0x0f;
    // 速度変化
    volatile s16 s;
    s = (s16)rand8_sign() << 6; // 0xffc0, 0x0000, 0x0040
    pObj->uGeo.geo.sx = s;
    s = (s16)rand8_sign() << 6; // 0xffc0, 0x0000, 0x0040
    pObj->uGeo.geo.sy = s;

    // ガタガタ動かない様にする調整
    pObj->uGeo.geo8.xl = (0 <= pObj->uGeo.geo8.sxh) ? 0x00: -0x40;
    pObj->uGeo.geo8.yl = (0 <= pObj->uGeo.geo8.syh) ? 0x00: -0x40;

    // 弾を吐く
    switch (pObj->uObjWork.enemy.bulletMode) {
    default: break;
    case BULLET_MODE_FIRE_ON_MOVE:
        createEnemyBullet(pObj);
        break;
    }
}
// 中速ランダム±方向
static void moveB(Obj* const pObj) __z88dk_fastcall
{
    pObj->uObjWork.enemy.ct--;
    if (pObj->uObjWork.enemy.ct) { return; }
    pObj->uObjWork.enemy.ct = (rand8() & 0x0f) + 0x0f;
    // 速いとランダム ウォークではプレーヤーが付いてこない
    // 現在の方向から次の方向をランダムで決めます. 90度まで向きを変えられるように
    u8 dir = pObj->uObjWork.enemy.dir;
    if (dir == 0) {
        dir = rand8() % 9;
    } else {
        u8 r = rand8() % 6;
        if (r == 5) { dir = 0; }
        else {
            dir--;              // dir : [0, 7]
            dir += r - 2;       // dir : [-2, 9]
            if (0x80   <= dir) { dir += 8; }
            else if (8 <= dir) { dir -= 8; }
            dir++;              // dir : [1, 8]
        }
    }
    pObj->uObjWork.enemy.dir = dir;
    // dir から方向を決める
    {
        static const s16 tab[] = {
            0x0000,   0x0000, // 0
            0x0060,   0x0000, // 1
            0x0040,   0x0040, // 2
            0x0000,   0x0060, // 3
            -0x0040,  0x0040, // 4
            -0x0060,  0x0000, // 5
            -0x0040, -0x0040, // 6
            0x0000,  -0x0060, // 7
            0x0040,  -0x0040, // 8
        };
        const s16* p = &tab[dir * 2];
        pObj->uGeo.geo.sx = *p; p++;
        pObj->uGeo.geo.sy = *p;
    }
    // ガタガタ動かない様にする調整
    pObj->uGeo.geo8.xl = (0 <= pObj->uGeo.geo8.sxh) ? 0x00: -0x40;
    pObj->uGeo.geo8.yl = (0 <= pObj->uGeo.geo8.syh) ? 0x00: -0x40;

    // 弾を吐く
    switch (pObj->uObjWork.enemy.bulletMode) {
    default: break;
    case BULLET_MODE_FIRE_ON_MOVE:
        createEnemyBullet(pObj);
        break;
    }
}
// 低速ランダム. 残り1機で倍速
static void moveC(Obj* const pObj) __z88dk_fastcall
{
    pObj->uObjWork.enemy.ct--;
    if (pObj->uObjWork.enemy.ct) { return; }
    if (stgGetNrEnemies() != 1) {
        pObj->uObjWork.enemy.ct = (rand8() & 0x1f) + 0x0f;
        // 速度変化
        volatile s16 s;
        s = (s16)rand8_sign() << 6; // 0xffc0, 0x0000, 0x0040
        pObj->uGeo.geo.sx = s;
        s = (s16)rand8_sign() << 6; // 0xffc0, 0x0000, 0x0040
        pObj->uGeo.geo.sy = s;

        // ガタガタ動かない様にする調整
        pObj->uGeo.geo8.xl = (0 <= pObj->uGeo.geo8.sxh) ? 0x00: -0x40;
        pObj->uGeo.geo8.yl = (0 <= pObj->uGeo.geo8.syh) ? 0x00: -0x40;
    } else {
        pObj->uObjWork.enemy.ct = (rand8() & 0xf) + 0x0c;
        // 速度変化
        volatile s16 s;
        s = (s16)rand8_sign() << 7; // 0xff80, 0x0000, 0x0080
        pObj->uGeo.geo.sx = s;
        s = (s16)rand8_sign() << 7; // 0xff80, 0x0000, 0x0080
        pObj->uGeo.geo.sy = s;

        // ガタガタ動かない様にする調整
        pObj->uGeo.geo8.xl = (0 <= pObj->uGeo.geo8.sxh) ? 0x00: -0x80;
        pObj->uGeo.geo8.yl = (0 <= pObj->uGeo.geo8.syh) ? 0x00: -0x80;
    }
}
// 低速ランダム. 残り2機で倍速 残り1機で4倍速
static void moveD(Obj* const pObj) __z88dk_fastcall
{
    pObj->uObjWork.enemy.ct--;
    if (pObj->uObjWork.enemy.ct) { return; }
    volatile s16 s;
    switch (stgGetNrEnemies()) {
    default:
        pObj->uObjWork.enemy.ct = (rand8() & 0x1f) + 0x0f;
        // 速度変化
        s = (s16)rand8_sign() << 6; // 0xffc0, 0x0000, 0x0040
        pObj->uGeo.geo.sx = s;
        s = (s16)rand8_sign() << 6; // 0xffc0, 0x0000, 0x0040
        pObj->uGeo.geo.sy = s;

        // ガタガタ動かない様にする調整
        pObj->uGeo.geo8.xl = (0 <= pObj->uGeo.geo8.sxh) ? 0x00: -0x40;
        pObj->uGeo.geo8.yl = (0 <= pObj->uGeo.geo8.syh) ? 0x00: -0x40;
        break;
    case 2:
        pObj->uObjWork.enemy.ct = (rand8() & 0xf) + 0x0c;
        // 速度変化
        s = (s16)rand8_sign() << 7; // 0xff80, 0x0000, 0x0080
        pObj->uGeo.geo.sx = s;
        s = (s16)rand8_sign() << 7; // 0xff80, 0x0000, 0x0080
        pObj->uGeo.geo.sy = s;

        // ガタガタ動かない様にする調整
        pObj->uGeo.geo8.xl = (0 <= pObj->uGeo.geo8.sxh) ? 0x00: -0x80;
        pObj->uGeo.geo8.yl = (0 <= pObj->uGeo.geo8.syh) ? 0x00: -0x80;
        break;
    case 1:
        pObj->uObjWork.enemy.ct = (rand8() & 0xf) + 0x07;
        // 速度変化
        pObj->uGeo.geo8.sxl = 0;
        pObj->uGeo.geo8.syl = 0;
        pObj->uGeo.geo8.sxh = rand8_sign();
        pObj->uGeo.geo8.syh = rand8_sign();
        break;
    }
}

// ---------------------------------------------------------------- メイン サブ関数(画面はみ出し)
// 画面はみ出しチェック
static void overhangCheck4(Obj* const pObj) __z88dk_fastcall
{
    if (pObj->uGeo.geo8.xh < 0) {
        pObj->uGeo.geo8.xh = 0;
    } else if (VRAM_WIDTH - CG_ENEMY4_1_WIDTH < pObj->uGeo.geo8.xh) {
        pObj->uGeo.geo8.xh = VRAM_WIDTH - CG_ENEMY4_1_WIDTH;
    }
    if (pObj->uGeo.geo8.yh < 0) {
        pObj->uGeo.geo8.yh = 0;
    } else if (VRAM_HEIGHT - CG_ENEMY4_1_HEIGHT < pObj->uGeo.geo8.yh) {
        pObj->uGeo.geo8.yh = VRAM_HEIGHT - CG_ENEMY4_1_HEIGHT;
    }
}
static void overhangCheck5(Obj* const pObj) __z88dk_fastcall
{
    if (pObj->uGeo.geo8.xh < 0) {
        pObj->uGeo.geo8.xh = 0;
    } else if (VRAM_WIDTH - CG_ENEMY5_1_WIDTH < pObj->uGeo.geo8.xh) {
        pObj->uGeo.geo8.xh = VRAM_WIDTH - CG_ENEMY5_1_WIDTH;
    }
    if (pObj->uGeo.geo8.yh < 0) {
        pObj->uGeo.geo8.yh = 0;
    } else if (VRAM_HEIGHT - CG_ENEMY5_1_HEIGHT < pObj->uGeo.geo8.yh) {
        pObj->uGeo.geo8.yh = VRAM_HEIGHT - CG_ENEMY5_1_HEIGHT;
    }
}
static void overhangCheck8(Obj* const pObj) __z88dk_fastcall
{
    if (pObj->uGeo.geo8.xh < 0) {
        pObj->uGeo.geo8.xh = 0;
    } else if (VRAM_WIDTH - CG_ENEMY8_1_WIDTH < pObj->uGeo.geo8.xh) {
        pObj->uGeo.geo8.xh = VRAM_WIDTH - CG_ENEMY8_1_WIDTH;
    }
    if (pObj->uGeo.geo8.yh < 0) {
        pObj->uGeo.geo8.yh = 0;
    } else if (VRAM_HEIGHT - CG_ENEMY8_1_HEIGHT < pObj->uGeo.geo8.yh) {
        pObj->uGeo.geo8.yh = VRAM_HEIGHT - CG_ENEMY8_1_HEIGHT;
    }
}

// ---------------------------------------------------------------- メイン
bool objEnemyMain3_1(Obj* const pObj)
{
    switch (pObj->step) {
    default:
        if (!invincibleCheck(pObj)) { moveA(pObj); }
        overhangDeadCheck3(pObj);
        return true;
    case STEP_DEAD:
        return dead3(pObj);
    }
    //printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 10));printHex16(pObj->uGeo.geo.x);printAddAddr(1);printHex16(pObj->uGeo.geo.y);
}
bool objEnemyMainDemo3_1(Obj* const pObj)
{
    pObj->uGeo.geo8.w = CG_ENEMY3_1_WIDTH;
    pObj->uObjWork.enemy.animCt++;

    if (pObj->ct) {
        pObj->ct--;
    } else {
        pObj->uObjWork.enemy.ct--;
    }

    // -------- 死亡チェック
    if (pObj->fitness == 0) {
        // 爆発, スコア, 敵減
        objCreateEtc(objExplosionInit, objExplosionMain, objExplosionDisp, pObj);
        return false;
    }
    return true;
}
bool objEnemyMain3_3(Obj* const pObj)
{
    switch (pObj->step) {
    default:
        if (!invincibleCheck(pObj)) { moveB(pObj); }
        overhangDeadCheck3(pObj);
        return true;
    case STEP_DEAD:
        return dead3(pObj);
    }
}
bool objEnemyMain3_5(Obj* const pObj)
{
    switch (pObj->step) {
    default:
        if (!invincibleCheck(pObj)) { moveC(pObj); }
        overhangDeadCheck3(pObj);
        return true;
    case STEP_DEAD:
        return dead3(pObj);
    }
}
bool objEnemyMain3_6(Obj* const pObj)
{
    switch (pObj->step) {
    default:
        if (!invincibleCheck(pObj)) { moveD(pObj); }
        overhangDeadCheck3(pObj);
        return true;
    case STEP_DEAD:
        return dead3(pObj);
    }
}

bool objEnemyMain4_1(Obj* const pObj)
{
    switch (pObj->step) {
    default:
        if (!invincibleCheck(pObj)) { moveA(pObj); }
        overhangCheck4(pObj);
        deadCheck458(pObj);
        return true;
    case STEP_DEAD:
        return dead45(pObj);
    }
}

bool objEnemyMain4_3(Obj* const pObj)
{
    switch (pObj->step) {
    default:
        if (!invincibleCheck(pObj)) { moveB(pObj); }
        overhangCheck4(pObj);
        deadCheck458(pObj);
        return true;
    case STEP_DEAD:
        return dead45(pObj);
    }
}
bool objEnemyMain5_1(Obj* const pObj)
{
    switch (pObj->step) {
    default:
        if (!invincibleCheck(pObj)) { moveA(pObj); }
        overhangCheck5(pObj);
        deadCheck458(pObj);
        return true;
    case STEP_DEAD:
        return dead45(pObj);
    }
}
bool objEnemyMain5_2(Obj* const pObj)
{
    switch (pObj->step) {
    default:
        if (!invincibleCheck(pObj)) { moveB(pObj); }
        overhangCheck5(pObj);
        deadCheck458(pObj);
        return true;
    case STEP_DEAD:
        return dead45(pObj);
    }
}

// 放射状に弾を吐く
static void objEnemyBullet1(Obj* const pObj) __z88dk_fastcall
{
    if (pObj->uObjWork.enemy.animCt & 0x3f) { return; }
    for (u8 i = 0; i < 16; i++) {
        Obj* const pBullet = objCreateEnemyBullet(objEnemyBulletInitWithoutVelocity, objEnemyBulletMain, objEnemyBulletDisp, pObj);
        if (pBullet) {
            u8 a = i * 16;
            pBullet->uGeo.geo.sx = cos(a) * 1;
            pBullet->uGeo.geo.sy = sin(a) * 1;
        }
    }
}
// スパイラル状に弾を吐く
static void objEnemyBullet2(Obj* const pObj) __z88dk_fastcall
{
    u8 animCt = pObj->uObjWork.enemy.animCt & 0x7f;
    if (50 < animCt) { return; }

    Obj* const pBullet = objCreateEnemyBullet(objEnemyBulletInitWithoutVelocity, objEnemyBulletMain, objEnemyBulletDisp, pObj);
    if (pBullet) {
        u8 a = - animCt * 16;
        pBullet->uGeo.geo.sx = cos(a);
        pBullet->uGeo.geo.sy = sin(a);
    }
}
// プレイヤー向けに三方向に弾を吐く
static void objEnemyBullet3(Obj* const pObj) __z88dk_fastcall
{
    if (pObj->uObjWork.enemy.animCt & 0x1f) { return; }

    // 移動方向はプレーヤー向け
    const Obj* const pPlayerObj = objGetInUsePlayer();
    if (!pPlayerObj) { return; }

    s8 dx = pPlayerObj->uGeo.geo8.xh     - pObj->uGeo.geo8.xh;
    s8 dy = pPlayerObj->uGeo.geo8.yh + 1 - pObj->uGeo.geo8.yh;
    u16 dxdy = ((u16)dx << 8) | ((u16)dy & 0xff);
    u8  a = atan2(dxdy) - 10;

    for (u8 i = 3; i != 0; i--, a += 10) {
        Obj* const pBullet = objCreateEnemyBullet(objEnemyBulletInitWithoutVelocity, objEnemyBulletMain, objEnemyBulletDisp, pObj);
        if (pBullet) {
            pBullet->uGeo.geo.sx = cos(a) * 2;
            pBullet->uGeo.geo.sy = sin(a) * 2;
        }
    }
}
// 4 方向にスパイラル
static void objEnemyBullet4(Obj* const pObj) __z88dk_fastcall
{
    u8 animCt = pObj->uObjWork.enemy.animCt;
    if (animCt & 0x07) { return; }

    for (u8 i = 4; i != 0; i--, animCt += 0x40) {
        Obj* const pBullet = objCreateEnemyBullet(objEnemyBulletInitWithoutVelocity, objEnemyBulletMain, objEnemyBulletDisp, pObj);
        if (pBullet) {
            pBullet->uGeo.geo.sx = cos(animCt) * 2;
            pBullet->uGeo.geo.sy = sin(animCt) * 2;
        }
    }
}

bool objEnemyMain8_1(Obj* const pObj)
{
    switch (pObj->step) {
    default:
        if (!invincibleCheck(pObj)) {
            moveB(pObj);
            objEnemyBullet1(pObj);
        }
        overhangCheck8(pObj);
        deadCheck458(pObj);
        return true;
    case STEP_DEAD:
        return dead45(pObj);
    }
}
bool objEnemyMain8_2(Obj* const pObj)
{
    switch (pObj->step) {
    default:
        if (!invincibleCheck(pObj)) {
            moveB(pObj);
            if (pObj->fitness < (u16)FITNESS_8_2 / 2) {
                objEnemyBullet2(pObj);
            } else {
                objEnemyBullet1(pObj);
            }
        }
        overhangCheck8(pObj);
        deadCheck458(pObj);
        return true;
    case STEP_DEAD:
        return dead45(pObj);
    }
}
bool objEnemyMain8_3(Obj* const pObj)
{
    switch (pObj->step) {
    default:
        if (!invincibleCheck(pObj)) {
            moveB(pObj);
            if        (pObj->fitness < (u16)FITNESS_8_3 / 4) {
                objEnemyBullet4(pObj);
            } else if (pObj->fitness < (u16)FITNESS_8_3 / 2) {
                objEnemyBullet3(pObj);
            } else if (pObj->fitness < (u16)FITNESS_8_3 / 4 * 3) {
                objEnemyBullet2(pObj);
            } else {
                objEnemyBullet1(pObj);
            }
        }
        overhangCheck8(pObj);
        deadCheck458(pObj);
        return true;
    case STEP_DEAD:
        return dead45(pObj);
    }
}

// ---------------------------------------------------------------- 描画
static const u8 coreTextTab[] = {
    0xff, 0xf3, 0xf5, 0xf6,
    0xf7, 0xf9, 0xfa, 0xfb,
    0xf7, 0xfb, 0xfd, 0xfe,
    0xfc, 0xfd, 0xfe, 0xff, };
void objEnemyDisp3_1(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        ((u8*)sEnemy3_1)[4] = coreTextTab[(pObj->uObjWork.enemy.animCt >> 2) & 0x0f];
        vVramDraw3x3(dispAddr, sEnemy3_1);
    }
}
void objEnemyDisp3_2(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        static const u8 tab[] = {0x62, 0x02};
        ((u8*)sEnemy3_2)[13] = tab[(pObj->uObjWork.enemy.animCt >> 3) & 1];
        vVramDraw3x3(dispAddr, pObj->bHit ? sEnemyDamage_3 : sEnemy3_2);
    }
}
void objEnemyDisp3_3(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        static const u8 tab[] = {0x04, 0x14, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74};
        ((u8*)sEnemy3_3)[13] = tab[pObj->uObjWork.enemy.animCt & 7];
        vVramDraw3x3(dispAddr, pObj->bHit ? sEnemyDamage_3 : sEnemy3_3);
    }
}
void objEnemyDisp3_4(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        ((u8*)sEnemy3_4)[4] = coreTextTab[rand8() & 0x0f];
        vVramDraw3x3(dispAddr, pObj->bHit ? sEnemyDamage_3 : sEnemy3_4);
    }
}
void objEnemyDisp3_5(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        static const u8 tab[] = {0x00, 0x00, 0x00, 0x20, };
        ((u8*)sEnemy3_5)[13] = tab[(pObj->uObjWork.enemy.animCt >> 2) & 3];
        vVramDraw3x3(dispAddr, pObj->bHit ? sEnemyDamage_3 : sEnemy3_5);
    }
}
void objEnemyDisp3_6(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        static const u8 tab[] = {
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x40, 0x00,
            0x40, 0x40, 0x40, 0x40,
            0x40, 0x40, 0x00, 0x40, };
        ((u8*)sEnemy3_6)[13] = tab[pObj->uObjWork.enemy.animCt & 0x0f];
        vVramDraw3x3(dispAddr, pObj->bHit ? sEnemyDamage_3_6 : sEnemy3_6);
    }
}
void objEnemyDisp4_1(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        static const u8 tab[] = {
            0xfe, 0xfd, 0xfb, 0xf7,
            0xf6, 0xf9, 0xf9, 0xf6,
        };
        const u8* p = &tab[pObj->uObjWork.enemy.animCt & 0x04];
        ((u8*)sEnemy4_1)[ 5] = *p++;
        ((u8*)sEnemy4_1)[ 6] = *p++;
        ((u8*)sEnemy4_1)[ 9] = *p++;
        ((u8*)sEnemy4_1)[10] = *p;
        vVramDraw4x4(dispAddr, pObj->bHit ? sEnemyDamage_4 : sEnemy4_1);
    }
}
void objEnemyDisp4_2(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        static const u8 tab[] = { 0x73, 0x63, 0x52, 0x42, 0x32, 0x42, 0x52, 0x63 };
        const u8 c = tab[pObj->uObjWork.enemy.animCt & 7];
        ((u8*)sEnemy4_2)[16 +  5] = c;
        ((u8*)sEnemy4_2)[16 +  6] = c;
        ((u8*)sEnemy4_2)[16 +  9] = c;
        ((u8*)sEnemy4_2)[16 + 10] = c;
        vVramDraw4x4(dispAddr, pObj->bHit ? sEnemyDamage_4 : sEnemy4_2);
    }
}
void objEnemyDisp4_3(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        static const u8 tab[] = {
            0xf2, 0xf9, 0xf9, 0xf4,
            0xf4, 0xf9, 0xf9, 0xf2,
            0xf6, 0xf8, 0xf1, 0xf6,
            0xf6, 0xf1, 0xf8, 0xf6,
        };
        const u8* p = &tab[(pObj->uObjWork.enemy.animCt << 1) & 0x0c];
        ((u8*)sEnemy4_3)[ 5] = *p++;
        ((u8*)sEnemy4_3)[ 6] = *p++;
        ((u8*)sEnemy4_3)[ 9] = *p++;
        ((u8*)sEnemy4_3)[10] = *p;
        vVramDraw4x4(dispAddr, pObj->bHit ? sEnemyDamage_4 : sEnemy4_3);
    }
}

void objEnemyDisp5_1(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        static const u8 tab[] = {
            0xf3, 0xf5, 0xf0, 0xfa, 0xfc,
            0xff, 0xff, 0xf0, 0xff, 0xff,
            0xfc, 0xfa, 0xff, 0xf5, 0xf3,
            0xf0, 0xf0, 0xff, 0xf0, 0xf0,
        };
        const u8* p = &tab[((pObj->uObjWork.enemy.animCt >> 1) & 0x03) * 5];
        ((u8*)sEnemy5_1)[ 7] = *p++;
        ((u8*)sEnemy5_1)[11] = *p++;
        ((u8*)sEnemy5_1)[12] = *p++;
        ((u8*)sEnemy5_1)[13] = *p++;
        ((u8*)sEnemy5_1)[17] = *p;
        vVramDraw5x5(dispAddr, pObj->bHit ? sEnemyDamage_5 : sEnemy5_1);
    }
}
void objEnemyDisp5_2(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        static const u8 tab[] = {
            0x06, 0x16, 0x26, 0x36, 0x46, 0x56, 0x66, 0x76,
            0x76, 0x66, 0x56, 0x46, 0x36, 0x26, 0x16, 0x06,
        };
        const u8 c = tab[pObj->uObjWork.enemy.animCt & 0x0f];
        ((u8*)sEnemy5_2)[25 +  7] = c;
        ((u8*)sEnemy5_2)[25 + 11] = c;
        ((u8*)sEnemy5_2)[25 + 13] = c;
        ((u8*)sEnemy5_2)[25 + 17] = c;
        ((u8*)sEnemy5_2)[     12] = (rand8() & 0x0f) + 0xf0;
        vVramDraw5x5(dispAddr, pObj->bHit ? sEnemyDamage_5 : sEnemy5_2);
    }
}
void objEnemyDisp5_3(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        static const u8 tab[] = {
            0xf0, 0xd0, 0xd0, 0x90,
            0x70, 0x50, 0x50, 0x10,
        };
        const u8* p = &tab[(pObj->uObjWork.enemy.animCt >> 1) & 0x04];
        ((u8*)sEnemy5_3)[25 +  0] = *p++;
        ((u8*)sEnemy5_3)[25 +  4] = *p++;
        ((u8*)sEnemy5_3)[25 + 20] = *p++;
        ((u8*)sEnemy5_3)[25 + 24] = *p;
        vVramDraw5x5(dispAddr, pObj->bHit ? sEnemyDamage_5 : sEnemy5_3);
    }
}

static void objEnemyDisp8_1Anim(const u8 animCt) __z88dk_fastcall
{
    {
        const u8 atb = (animCt & 0x0c) ? 0x60 : 0x20;
        ((u8*)sEnemy8_1)[64 +  0] = atb;
        ((u8*)sEnemy8_1)[64 +  7] = atb;
        ((u8*)sEnemy8_1)[64 + 56] = atb;
        ((u8*)sEnemy8_1)[64 + 63] = atb;
    }
    {
        static const u8 tab[] = {
            0x05, 0x15, 0x25, 0x35, 0x45, 0x55, 0x65, 0x75,
            0x75, 0x65, 0x55, 0x45, 0x35, 0x25, 0x15, 0x05,
        };
        const u8 atb = tab[animCt & 0x0f];
        ((u8*)sEnemy8_1)[64 +  17] = atb;
        ((u8*)sEnemy8_1)[64 +  22] = atb;
        ((u8*)sEnemy8_1)[64 +  41] = atb;
        ((u8*)sEnemy8_1)[64 +  46] = atb;
    }
}

void objEnemyDisp8_1(Obj* const pObj, u8* dispAddr)
{
    if (!(pObj->ct & 1)) {
        const u8* e = sEnemy8_1;
        if (pObj->bHit) {
            e = sEnemyDamage_8;
        }
        objEnemyDisp8_1Anim(pObj->uObjWork.enemy.animCt);
        vVramDrawRect(dispAddr, e, W8H8(8, 8));
    }
}
void objEnemyDisp8_2(Obj* const pObj, u8* dispAddr)
{
    ((u8*)sEnemy8_1)[12] = CHAR_2;
    objEnemyDisp8_1(pObj, dispAddr);
}
void objEnemyDisp8_3(Obj* const pObj, u8* dispAddr)
{
    ((u8*)sEnemy8_1)[12] = CHAR_3;
    if (!(pObj->ct & 1)) { // 点滅
        const u8* e = sEnemy8_1;
        if (pObj->bHit) {
            e = sEnemyDamage_8;
        } else {// 赤点滅
            u16 f = pObj->fitness;
            if (f) { // 死んでたら点滅しない
                f /= ((u16)FITNESS_8_3 / 4);
                static const u8 tab[] = { 0x03, 0x0f, 0x3f, 0x00, 0x00};
                if ((pObj->uObjWork.enemy.animCt & tab[f]) == 1) { // テーブル値が 0x00 ならば点滅しない
                    e = sEnemyRed_8;
                }
            }
        }
        objEnemyDisp8_1Anim(pObj->uObjWork.enemy.animCt);
        vVramDrawRect(dispAddr, e, W8H8(8, 8));
    }
}
