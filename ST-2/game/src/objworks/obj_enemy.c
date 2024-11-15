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
#include "../game/game_mode.h"
#include "../game/se.h"
#include "../../cg/enemy3_1.h"
#include "../../cg/enemy3_2.h"
#include "../../cg/enemy3_3.h"
#include "../../cg/enemy3_4.h"
#include "../../cg/enemy3_5.h"
#include "../../cg/enemy3_6.h"
#include "../../cg/enemy4_1.h"
#include "../../cg/enemy4_2.h"
#include "../../cg/enemy4_3.h"
#include "../../cg/enemy5_1.h"
#include "../../cg/enemy5_2.h"
#include "../../cg/enemy5_3.h"
#include "../../cg/enemy8_1.h"
#include "../../cg/enemy_damage3.h"
#include "../../cg/enemy_damage3_6.h"
#include "../../cg/enemy_damage4.h"
#include "../../cg/enemy_damage5.h"
#include "../../cg/enemy_damage8.h"
#include "../../cg/enemy_red8.h"
#include "obj_item.h"
#include "obj_player.h"
#include "obj_explosion.h"
#include "obj_enemy_bullet.h"
#include "obj_score.h"
#include "obj_enemy.h"

// ---------------------------------------------------------------- 変数
u16 _objEnemyNrKilled;

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

#if DEBUG && 0// TEST 全ゲームモードのテスト... 全敵がメチャ弱い
#define FITNESS_3_1 1
#define FITNESS_3_2 1
#define FITNESS_3_3 1
#define FITNESS_3_4 1
#define FITNESS_3_5 1
#define FITNESS_3_6 1
#define FITNESS_4_1 1
#define FITNESS_4_2 1
#define FITNESS_4_3 1
#define FITNESS_5_1 1
#define FITNESS_5_2 2
#define FITNESS_5_3 3
#define FITNESS_8_1 10
#define FITNESS_8_2 50
#define FITNESS_8_3 30
#else
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
#define FITNESS_8_1 1000
#define FITNESS_8_2 5000
#define FITNESS_8_3 33000
#endif

// アイテム数は, 「nrItems + rand8() % nrItems」個
#define H4_ITEM_SUB_LEVEL4_NR8(h, sub_level, nr_items) (((h) << 12) | ((sub_level) << 8) | (nr_items))


// ---------------------------------------------------------------- 初期化
/**
 * @param hItem H4_ITEM_SUB_LEVEL4_NR8 マクロを使ってください
 */
static void initEnemySub_(Obj* const p_obj, const u16 fitness, const u16 score, const u16 hItem)
{
    u8 h = hItem >> 12;
    OBJ_INIT(p_obj, (rand8() % (VRAM_WIDTH + 1 - h)) << 8, (rand8() % (VRAM_HEIGHT + 1 - h)) << 8, 0, h, 0, 0);

    p_obj->ct      = INVINCIBLE_TIME; // 無敵時間 ct は負の値は無敵, 0 以上は方向カウンタ
    p_obj->step    = STEP_MAIN;
    p_obj->fitness = fitness;
    p_obj->u_obj_work.enemy.score          = score;
    p_obj->u_obj_work.enemy.item_sub_level = (hItem >> 8) & 0x0f;
    p_obj->u_obj_work.enemy.nr_items       = hItem + gameHard();
    p_obj->u_obj_work.enemy.ct             = rand8() & 0x1f;
    p_obj->u_obj_work.enemy.anim_ct        = rand8();
    p_obj->u_obj_work.enemy.dir            = 0;
    p_obj->u_obj_work.enemy.bullet_mode    = BULLET_MODE_NONE;
}

#pragma disable_warning 85  // p_parent 未使用
#pragma save
// テーブル化するより, 個別に initEnemySub_() を呼んだ方がコード量が小さくなる...
void objEnemyInit3_1(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_3_1, SCORE_3_1, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY3_1_HEIGHT, 1, 1));
}
void objEnemyInit3_2(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_3_2, SCORE_3_2, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY3_2_HEIGHT, 1, 2));
}
void objEnemyInit3_3(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_3_3, SCORE_3_3, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY3_3_HEIGHT, 2, 2));
}
void objEnemyInit3_4(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_3_4, SCORE_3_4, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY3_4_HEIGHT, 2, 3));
}
void objEnemyInit3_5(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_3_5, SCORE_3_5, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY3_5_HEIGHT, 3, 1));
}
void objEnemyInit3_6(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_3_6, SCORE_3_6, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY3_6_HEIGHT, 3, 2));
}
void objEnemyInit4_1(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_4_1, SCORE_4_1, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY4_1_HEIGHT, 1, 6));// 安っぽいアイテムを沢山ばら撒く
}
void objEnemyInit4_2(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_4_2, SCORE_4_2, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY4_2_HEIGHT, 4, 5));// 強敵なのでアイテム沢山あげる
    p_obj->u_obj_work.enemy.bullet_mode = BULLET_MODE_FIRE_ON_MOVE;
}
void objEnemyInit4_3(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_4_3, SCORE_4_3, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY4_3_HEIGHT, 5, 2));
}
void objEnemyInit5_1(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_5_1, SCORE_5_1, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY5_1_HEIGHT, 5, 5));// ここいらへんからアイテムは乱発!
    p_obj->u_obj_work.enemy.bullet_mode = BULLET_MODE_FIRE_ON_DEAD;
}
void objEnemyInit5_2(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_5_2, SCORE_5_2, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY5_2_HEIGHT, 6, 5));
}
void objEnemyInit5_3(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_5_3, SCORE_5_3, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY5_3_HEIGHT, 7, 7));
    p_obj->u_obj_work.enemy.bullet_mode = BULLET_MODE_FIRE_ON_MOVE;
}
void objEnemyInit8_1(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_8_1, SCORE_8_1, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY8_1_HEIGHT, 7, 30));
}
void objEnemyInit8_2(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_8_2, SCORE_8_2, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY8_1_HEIGHT, 7, 30));
}
void objEnemyInit8_3(Obj* const p_obj, Obj* const p_parent)
{
    initEnemySub_(p_obj, FITNESS_8_3, SCORE_8_3, H4_ITEM_SUB_LEVEL4_NR8(CG_ENEMY8_1_HEIGHT, 7, 30));
    p_obj->u_obj_work.enemy.bullet_mode = BULLET_MODE_FIRE_ON_MOVE;
}
#pragma restore

// ---------------------------------------------------------------- メイン サブ関数(画面はみ出し, 死亡チェック)
static void overhangDeadCheck3(Obj* const p_obj) __z88dk_fastcall
{
    //if (3 <= soundGetPriority()) {
    //    sdMake(((((u8)p_obj &0x3f) + 1) << 8) + p_obj->u_geo.geo8.yh);
    //}

    // -------- 画面はみ出し処理
    if (p_obj->u_geo.geo8.xh < 0) {
        p_obj->u_geo.geo8.xh = 0;
    } else if (VRAM_WIDTH - CG_ENEMY3_1_WIDTH < p_obj->u_geo.geo8.xh) {
        p_obj->u_geo.geo8.xh = VRAM_WIDTH - CG_ENEMY3_1_WIDTH;
    }
    if (p_obj->u_geo.geo8.yh < 0) {
        p_obj->u_geo.geo8.yh = 0;
    } else if (VRAM_HEIGHT - CG_ENEMY3_1_HEIGHT < p_obj->u_geo.geo8.yh) {
        p_obj->u_geo.geo8.yh = VRAM_HEIGHT - CG_ENEMY3_1_HEIGHT;
    }

    // -------- 死亡チェック
    if (p_obj->fitness == 0) {
        // アイテムの生成
        u8 nr_items = p_obj->u_obj_work.enemy.nr_items;
        for (u8 i = (rand8() % nr_items) + nr_items; i != 0; i--) {
            if (!objCreateItem(objItemInit, objItemMain, objItemDraw, p_obj)) { break; }
        }

        // 爆発, スコア, 敵減
        objCreateEtc(objExplosionInit, objExplosionMain, objExplosionDraw, p_obj);
        scoreAdd(p_obj->u_obj_work.enemy.score);
        _objEnemyNrKilled++;
        sdPlaySe(SE_ENEMY_DEAD3);
        p_obj->u_geo.geo.sx = 0;// 移動停止
        p_obj->u_geo.geo.sy = 0;// 移動停止
        p_obj->u_geo.geo.w  = 0;// 衝突判定を止める
        p_obj->ct   = 1;        // 表示しない
        p_obj->u_obj_work.enemy.ct = 10;
        p_obj->step = STEP_DEAD;
    } else if (p_obj->b_hit) {
        sdPlaySe(SE_ENEMY_DAMAGE);
    }
}
static void deadCheck458(Obj* const p_obj) __z88dk_fastcall
{
    //if (3 <= soundGetPriority()) {
    //    sdMake(((((u8)p_obj &0x3f) + 1) << 8) + p_obj->u_geo.geo8.yh);
    //}
    if (p_obj->fitness == 0) {
        // アイテムの生成
        u8 nr_items = p_obj->u_obj_work.enemy.nr_items;
        for (u8 i = (rand8() % nr_items) + nr_items; i != 0; i--) {
            if (!objCreateItem(objItemInit, objItemMain, objItemDraw, p_obj)) { break; }
        }
         // 撃ち返し!
        if (p_obj->u_obj_work.enemy.bullet_mode == BULLET_MODE_FIRE_ON_DEAD) {
            objCreateEnemyBullet(objEnemyBulletInit, objEnemyBulletMain, objEnemyBulletDraw, p_obj);
        }
        // スコア処理
        u16 score = p_obj->u_obj_work.enemy.score;
        scoreAdd(score);
        _objEnemyNrKilled++;
        if (100 <= score) { // 1000 点以上はスコアを表示
            objCreateEtc(objScoreInit, objScoreMain, objScoreDraw, p_obj);
        }
        if (p_obj->u_geo.geo.w == 8) {
            p_obj->ct = 120;
            sdPlaySe(SE_ENEMY_DEAD8);
        } else {
            p_obj->ct = 30;
            sdPlaySe(SE_ENEMY_DEAD45);
        }
        p_obj->u_geo.geo.sx = 0;// 移動停止
        p_obj->u_geo.geo.sy = 0;// 移動停止
        p_obj->u_geo.geo.w  = 0;// 衝突判定を止める
        p_obj->step = STEP_DEAD;
    } else if (p_obj->b_hit) {
        sdPlaySe(SE_ENEMY_DAMAGE);// ダメージ音
    }
}

// ---------------------------------------------------------------- メイン サブ関数(死亡処理)
static bool dead3(Obj* const p_obj) __z88dk_fastcall
{
    p_obj->u_obj_work.enemy.ct--;
    if (p_obj->u_obj_work.enemy.ct == 0) {
        stgDecrementEnemies();
        return false;
    }
    return true;
}
static bool dead45(Obj* const p_obj) __z88dk_fastcall
{
    p_obj->ct--;
    if ((p_obj->ct & 3) == 0) {
        objCreateEtc(objExplosionInit, objExplosionMain, objExplosionDraw, p_obj);
    }
    if (p_obj->ct == 0) {
        stgDecrementEnemies();
        return false;
    }
    return true;
}

// ---------------------------------------------------------------- メイン サブ関数(無敵, 弾出し)
// 無敵チェック
static bool invincibleCheck(Obj* const p_obj) __z88dk_fastcall
{
    p_obj->u_obj_work.enemy.anim_ct++;

    if (p_obj->ct) {
        p_obj->ct--;
        if (p_obj->ct == 0) {
            p_obj->u_geo.geo8.w = p_obj->u_geo.geo8.h;
        }
        return true;
    }
    return false;
}

// 敵の数に応じて弾を吐く
static const u8 ENEMY_BULLET_TAB_[] = {
    0x01, 0x01, 0x03, 0x03,  0x07, 0x07, 0x07, 0x0f,  0x0f, 0x0f, 0x1f, 0x1f,  0xff, 0xff, 0xff, 0xff
};
static void createEnemyBullet(Obj* const p_obj) __z88dk_fastcall
{
    if (p_obj->u_obj_work.enemy.ct & ENEMY_BULLET_TAB_[stgGetNrEnemies()]) {
        return;
    }
    objCreateEnemyBullet(objEnemyBulletInit, objEnemyBulletMain, objEnemyBulletDraw, p_obj);
}

// ---------------------------------------------------------------- メイン サブ関数(移動系)
static void moveSlow(Obj* const p_obj) __z88dk_fastcall
{
    p_obj->u_obj_work.enemy.ct = (rand8() & 0x1f) + 0x0f;

    // 速度変化
    s16 s;
    s = (s16)rand8_sign() << 6; // 0xffc0, 0x0000, 0x0040
    p_obj->u_geo.geo.sx = s;
    s = (s16)rand8_sign() << 6; // 0xffc0, 0x0000, 0x0040
    p_obj->u_geo.geo.sy = s;

    // ガタガタ動かない様にする調整
    p_obj->u_geo.geo8.xl = (0 <= p_obj->u_geo.geo8.sxh) ? 0x00: -0x40;
    p_obj->u_geo.geo8.yl = (0 <= p_obj->u_geo.geo8.syh) ? 0x00: -0x40;
}

static void moveFaster(Obj* const p_obj) __z88dk_fastcall
{
    p_obj->u_obj_work.enemy.ct = (rand8() & 0xf) + 0x0c;

    // 速度変化
    volatile s16 s; // 何故か最適化に失敗する
    s = (s16)rand8_sign() << 7; // 0xff80, 0x0000, 0x0080;
    p_obj->u_geo.geo.sx = s;
    s = (s16)rand8_sign() << 7; // 0xff80, 0x0000, 0x0080;
    p_obj->u_geo.geo.sy = s;

    // ガタガタ動かない様にする調整
    p_obj->u_geo.geo8.xl = (0 <= p_obj->u_geo.geo8.sxh) ? 0x00: -0x80;
    p_obj->u_geo.geo8.yl = (0 <= p_obj->u_geo.geo8.syh) ? 0x00: -0x80;
}

// 最高速移動
static void moveFastest(Obj* const p_obj) __z88dk_fastcall
{
    p_obj->u_obj_work.enemy.ct = (rand8() & 0xf) + 0x07;
    p_obj->u_geo.geo8.sxl = 0;
    p_obj->u_geo.geo8.syl = 0;
    p_obj->u_geo.geo8.sxh = rand8_sign();
    p_obj->u_geo.geo8.syh = rand8_sign();
}

// 低速ランダム
static void moveA(Obj* const p_obj) __z88dk_fastcall
{
    p_obj->u_obj_work.enemy.ct--;
    if (p_obj->u_obj_work.enemy.ct) { return; }

    if (gameHard() && stgGetNrEnemies() == 1) { // ハードモードは高速
        moveFaster(p_obj);
    } else {
        moveSlow(p_obj);
    }
#if 0
    p_obj->u_obj_work.enemy.ct = (rand8() & 0x1f) + 0x0f;
    // 速度変化
    volatile s16 s;
    s = (s16)rand8_sign() << 6; // 0xffc0, 0x0000, 0x0040
    p_obj->u_geo.geo.sx = s;
    s = (s16)rand8_sign() << 6; // 0xffc0, 0x0000, 0x0040
    p_obj->u_geo.geo.sy = s;

    // ガタガタ動かない様にする調整
    p_obj->u_geo.geo8.xl = (0 <= p_obj->u_geo.geo8.sxh) ? 0x00: -0x40;
    p_obj->u_geo.geo8.yl = (0 <= p_obj->u_geo.geo8.syh) ? 0x00: -0x40;
#endif

    // 弾を吐く
    switch (p_obj->u_obj_work.enemy.bullet_mode) {
    default: break;
    case BULLET_MODE_FIRE_ON_MOVE:
        createEnemyBullet(p_obj);
        break;
    }
}
// 中速ランダム±方向
static const s16 ENEMY_DIR_TAB_[] = {
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
static void moveB(Obj* const p_obj) __z88dk_fastcall
{
    p_obj->u_obj_work.enemy.ct--;
    if (p_obj->u_obj_work.enemy.ct) { return; }
    p_obj->u_obj_work.enemy.ct = (rand8() & 0x0f) + 0x0f;
    // 速いとランダム ウォークではプレーヤーが付いてこない
    // 現在の方向から次の方向をランダムで決めます. 90度まで向きを変えられるように
    u8 dir = p_obj->u_obj_work.enemy.dir;
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
    p_obj->u_obj_work.enemy.dir = dir;
    // dir から方向を決める
    {
        const s16* p = &ENEMY_DIR_TAB_[dir * 2];
        if (gameHard() && stgGetNrEnemies() == 1) { // ハードモードは高速
            p_obj->u_geo.geo.sx = *p * 2; p++;
            p_obj->u_geo.geo.sy = *p * 2;
        } else {
            p_obj->u_geo.geo.sx = *p; p++;
            p_obj->u_geo.geo.sy = *p;
        }
    }
    // ガタガタ動かない様にする調整
    p_obj->u_geo.geo8.xl = (0 <= p_obj->u_geo.geo8.sxh) ? 0x00: -0x40;
    p_obj->u_geo.geo8.yl = (0 <= p_obj->u_geo.geo8.syh) ? 0x00: -0x40;

    // 弾を吐く
    switch (p_obj->u_obj_work.enemy.bullet_mode) {
    default: break;
    case BULLET_MODE_FIRE_ON_MOVE:
        createEnemyBullet(p_obj);
        break;
    }
}
// 低速ランダム. 残り1機で倍速
static void moveC(Obj* const p_obj) __z88dk_fastcall
{
    p_obj->u_obj_work.enemy.ct--;
    if (p_obj->u_obj_work.enemy.ct) { return; }
    if (stgGetNrEnemies() != 1) {
        moveSlow(p_obj);
    } else {
        if (gameHard()) { // ハードモードは高速
            moveFastest(p_obj);
        } else {
            moveFaster(p_obj);
        }
    }
}
// 低速ランダム. 残り2機で倍速 残り1機で4倍速
static void moveD(Obj* const p_obj) __z88dk_fastcall
{
    p_obj->u_obj_work.enemy.ct--;
    if (p_obj->u_obj_work.enemy.ct) { return; }
    volatile s16 s;
    switch (stgGetNrEnemies()) {
    default:
        moveSlow(p_obj);
        break;
    case 2:
        if (gameHard()) { // ハードモードは高速
            moveFastest(p_obj);
        } else {
            moveFaster(p_obj);
        }
        break;
    case 1:
        moveFastest(p_obj);
        break;
    }
}

// ---------------------------------------------------------------- メイン サブ関数(画面はみ出し)
// 画面はみ出しチェック
static void overhangCheck4(Obj* const p_obj) __z88dk_fastcall
{
    if (p_obj->u_geo.geo8.xh < 0) {
        p_obj->u_geo.geo8.xh = 0;
    } else if (VRAM_WIDTH - CG_ENEMY4_1_WIDTH < p_obj->u_geo.geo8.xh) {
        p_obj->u_geo.geo8.xh = VRAM_WIDTH - CG_ENEMY4_1_WIDTH;
    }
    if (p_obj->u_geo.geo8.yh < 0) {
        p_obj->u_geo.geo8.yh = 0;
    } else if (VRAM_HEIGHT - CG_ENEMY4_1_HEIGHT < p_obj->u_geo.geo8.yh) {
        p_obj->u_geo.geo8.yh = VRAM_HEIGHT - CG_ENEMY4_1_HEIGHT;
    }
}

static void overhangCheck5(Obj* const p_obj) __z88dk_fastcall
{
    if (p_obj->u_geo.geo8.xh < 0) {
        p_obj->u_geo.geo8.xh = 0;
    } else if (VRAM_WIDTH - CG_ENEMY5_1_WIDTH < p_obj->u_geo.geo8.xh) {
        p_obj->u_geo.geo8.xh = VRAM_WIDTH - CG_ENEMY5_1_WIDTH;
    }
    if (p_obj->u_geo.geo8.yh < 0) {
        p_obj->u_geo.geo8.yh = 0;
    } else if (VRAM_HEIGHT - CG_ENEMY5_1_HEIGHT < p_obj->u_geo.geo8.yh) {
        p_obj->u_geo.geo8.yh = VRAM_HEIGHT - CG_ENEMY5_1_HEIGHT;
    }
}

static void overhangCheck8(Obj* const p_obj) __z88dk_fastcall
{
    if (p_obj->u_geo.geo8.xh < 0) {
        p_obj->u_geo.geo8.xh = 0;
    } else if (VRAM_WIDTH - CG_ENEMY8_1_WIDTH < p_obj->u_geo.geo8.xh) {
        p_obj->u_geo.geo8.xh = VRAM_WIDTH - CG_ENEMY8_1_WIDTH;
    }
    if (p_obj->u_geo.geo8.yh < 0) {
        p_obj->u_geo.geo8.yh = 0;
    } else if (VRAM_HEIGHT - CG_ENEMY8_1_HEIGHT < p_obj->u_geo.geo8.yh) {
        p_obj->u_geo.geo8.yh = VRAM_HEIGHT - CG_ENEMY8_1_HEIGHT;
    }
}

// ---------------------------------------------------------------- メイン
bool objEnemyMain3_1(Obj* const p_obj)
{
    switch (p_obj->step) {
    default:
        if (!invincibleCheck(p_obj)) { moveA(p_obj); }
        overhangDeadCheck3(p_obj);
        return true;
    case STEP_DEAD:
        return dead3(p_obj);
    }
    //printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 10));printHex16(p_obj->u_geo.geo.x);printAddAddr(1);printHex16(p_obj->u_geo.geo.y);
}
bool objEnemyMainDemo3_1(Obj* const p_obj)
{
    p_obj->u_geo.geo8.w = CG_ENEMY3_1_WIDTH;
    p_obj->u_obj_work.enemy.anim_ct++;

    if (p_obj->ct) {
        p_obj->ct--;
    } else {
        p_obj->u_obj_work.enemy.ct--;
    }

    // -------- 死亡チェック
    if (p_obj->fitness == 0) {
        // 爆発, スコア, 敵減
        objCreateEtc(objExplosionInit, objExplosionMain, objExplosionDraw, p_obj);
        return false;
    }
    return true;
}
bool objEnemyMain3_3(Obj* const p_obj)
{
    switch (p_obj->step) {
    default:
        if (!invincibleCheck(p_obj)) { moveB(p_obj); }
        overhangDeadCheck3(p_obj);
        return true;
    case STEP_DEAD:
        return dead3(p_obj);
    }
}
bool objEnemyMain3_5(Obj* const p_obj)
{
    switch (p_obj->step) {
    default:
        if (!invincibleCheck(p_obj)) { moveC(p_obj); }
        overhangDeadCheck3(p_obj);
        return true;
    case STEP_DEAD:
        return dead3(p_obj);
    }
}
bool objEnemyMain3_6(Obj* const p_obj)
{
    switch (p_obj->step) {
    default:
        if (!invincibleCheck(p_obj)) { moveD(p_obj); }
        overhangDeadCheck3(p_obj);
        return true;
    case STEP_DEAD:
        return dead3(p_obj);
    }
}

bool objEnemyMain4_1(Obj* const p_obj)
{
    switch (p_obj->step) {
    default:
        if (!invincibleCheck(p_obj)) { moveA(p_obj); }
        overhangCheck4(p_obj);
        deadCheck458(p_obj);
        return true;
    case STEP_DEAD:
        return dead45(p_obj);
    }
}

bool objEnemyMain4_3(Obj* const p_obj)
{
    switch (p_obj->step) {
    default:
        if (!invincibleCheck(p_obj)) { moveB(p_obj); }
        overhangCheck4(p_obj);
        deadCheck458(p_obj);
        return true;
    case STEP_DEAD:
        return dead45(p_obj);
    }
}
bool objEnemyMain5_1(Obj* const p_obj)
{
    switch (p_obj->step) {
    default:
        if (!invincibleCheck(p_obj)) { moveA(p_obj); }
        overhangCheck5(p_obj);
        deadCheck458(p_obj);
        return true;
    case STEP_DEAD:
        return dead45(p_obj);
    }
}
bool objEnemyMain5_2(Obj* const p_obj)
{
    switch (p_obj->step) {
    default:
        if (!invincibleCheck(p_obj)) { moveB(p_obj); }
        overhangCheck5(p_obj);
        deadCheck458(p_obj);
        return true;
    case STEP_DEAD:
        return dead45(p_obj);
    }
}

// 放射状に弾を吐く
static void objEnemyBullet1(Obj* const p_obj) __z88dk_fastcall
{
    if (p_obj->u_obj_work.enemy.anim_ct & 0x3f) { return; }
    for (u8 i = 0; i < 16; i++) {
        Obj* const p_bullet = objCreateEnemyBullet(objEnemyBulletInitWithoutVelocity, objEnemyBulletMain, objEnemyBulletDraw, p_obj);
        if (p_bullet) {
            u8 a = i * 16;
            p_bullet->u_geo.geo.sx = cos(a) * 1;
            p_bullet->u_geo.geo.sy = sin(a) * 1;
        }
    }
}
// スパイラル状に弾を吐く
static void objEnemyBullet2(Obj* const p_obj) __z88dk_fastcall
{
    u8 anim_ct = p_obj->u_obj_work.enemy.anim_ct & 0x7f;
    if (50 < anim_ct) { return; }

    Obj* const p_bullet = objCreateEnemyBullet(objEnemyBulletInitWithoutVelocity, objEnemyBulletMain, objEnemyBulletDraw, p_obj);
    if (p_bullet) {
        u8 a = - anim_ct * 16;
        p_bullet->u_geo.geo.sx = cos(a);
        p_bullet->u_geo.geo.sy = sin(a);
    }
}
// プレイヤー向けに三方向に弾を吐く
static void objEnemyBullet3(Obj* const p_obj) __z88dk_fastcall
{
    if (p_obj->u_obj_work.enemy.anim_ct & 0x1f) { return; }

    // 移動方向はプレーヤー向け
    const Obj* const p_player = objGetInUsePlayer();
    if (!p_player) { return; }

    s8 dx = p_player->u_geo.geo8.xh     - p_obj->u_geo.geo8.xh;
    s8 dy = p_player->u_geo.geo8.yh + 1 - p_obj->u_geo.geo8.yh;
    u16 dxdy = ((u16)dx << 8) | ((u16)dy & 0xff);
    u8  a = atan2(dxdy) - 10;

    for (u8 i = 3; i != 0; i--, a += 10) {
        Obj* const p_bullet = objCreateEnemyBullet(objEnemyBulletInitWithoutVelocity, objEnemyBulletMain, objEnemyBulletDraw, p_obj);
        if (p_bullet) {
            p_bullet->u_geo.geo.sx = cos(a) * 2;
            p_bullet->u_geo.geo.sy = sin(a) * 2;
        }
    }
}
// 4 方向にスパイラル
static void objEnemyBullet4(Obj* const p_obj) __z88dk_fastcall
{
    u8 anim_ct = p_obj->u_obj_work.enemy.anim_ct;
    if (anim_ct & 0x07) { return; }

    for (u8 i = 4; i != 0; i--, anim_ct += 0x40) {
        Obj* const p_bullet = objCreateEnemyBullet(objEnemyBulletInitWithoutVelocity, objEnemyBulletMain, objEnemyBulletDraw, p_obj);
        if (p_bullet) {
            p_bullet->u_geo.geo.sx = cos(anim_ct) * 2;
            p_bullet->u_geo.geo.sy = sin(anim_ct) * 2;
        }
    }
}

bool objEnemyMain8_1(Obj* const p_obj)
{
    switch (p_obj->step) {
    default:
        if (!invincibleCheck(p_obj)) {
            moveB(p_obj);
            objEnemyBullet1(p_obj);
        }
        overhangCheck8(p_obj);
        deadCheck458(p_obj);
        return true;
    case STEP_DEAD:
        return dead45(p_obj);
    }
}
bool objEnemyMain8_2(Obj* const p_obj)
{
    switch (p_obj->step) {
    default:
        if (!invincibleCheck(p_obj)) {
            moveB(p_obj);
            if (p_obj->fitness < (u16)FITNESS_8_2 / 2) {
                objEnemyBullet2(p_obj);
            } else {
                objEnemyBullet1(p_obj);
            }
        }
        overhangCheck8(p_obj);
        deadCheck458(p_obj);
        return true;
    case STEP_DEAD:
        return dead45(p_obj);
    }
}
bool objEnemyMain8_3(Obj* const p_obj)
{
    switch (p_obj->step) {
    default:
        if (!invincibleCheck(p_obj)) {
            moveB(p_obj);
            if        (p_obj->fitness < (u16)FITNESS_8_3 / 4) {
                objEnemyBullet4(p_obj);
            } else if (p_obj->fitness < (u16)FITNESS_8_3 / 2) {
                objEnemyBullet3(p_obj);
            } else if (p_obj->fitness < (u16)FITNESS_8_3 / 4 * 3) {
                objEnemyBullet2(p_obj);
            } else {
                objEnemyBullet1(p_obj);
            }
        }
        overhangCheck8(p_obj);
        deadCheck458(p_obj);
        return true;
    case STEP_DEAD:
        return dead45(p_obj);
    }
}

// ---------------------------------------------------------------- 描画
static const u8 CORE_TEXT_TAB_[] = {
    0xff, 0xf3, 0xf5, 0xf6,
    0xf7, 0xf9, 0xfa, 0xfb,
    0xf7, 0xfb, 0xfd, 0xfe,
    0xfc, 0xfd, 0xfe, 0xff,
};
void objEnemyDraw3_1(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        ((u8*)cg_enemy3_1)[4] = CORE_TEXT_TAB_[(p_obj->u_obj_work.enemy.anim_ct >> 2) & 0x0f];
        vVramDraw3x3(draw_addr, cg_enemy3_1);
    }
}
static const u8 ENEMY_3_2_ANIM_TAB_[] = {0x62, 0x02};
void objEnemyDraw3_2(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        ((u8*)cg_enemy3_2)[13] = ENEMY_3_2_ANIM_TAB_[(p_obj->u_obj_work.enemy.anim_ct >> 3) & 1];
        vVramDraw3x3(draw_addr, p_obj->b_hit ? cg_enemy_damage3 : cg_enemy3_2);
    }
}
static const u8 ENEMY_3_3_ANIM_TAB_[] = {0x04, 0x14, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74};
void objEnemyDraw3_3(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        ((u8*)cg_enemy3_3)[13] = ENEMY_3_3_ANIM_TAB_[p_obj->u_obj_work.enemy.anim_ct & 7];
        vVramDraw3x3(draw_addr, p_obj->b_hit ? cg_enemy_damage3 : cg_enemy3_3);
    }
}
void objEnemyDraw3_4(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        ((u8*)cg_enemy3_4)[4] = CORE_TEXT_TAB_[rand8() & 0x0f];
        vVramDraw3x3(draw_addr, p_obj->b_hit ? cg_enemy_damage3 : cg_enemy3_4);
    }
}

static const u8 ENEMY_3_5_ANIM_TAB_[] = { 0x00, 0x00, 0x00, 0x20, };
void objEnemyDraw3_5(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        ((u8*)cg_enemy3_5)[13] = ENEMY_3_5_ANIM_TAB_[(p_obj->u_obj_work.enemy.anim_ct >> 2) & 3];
        vVramDraw3x3(draw_addr, p_obj->b_hit ? cg_enemy_damage3 : cg_enemy3_5);
    }
}
static const u8 ENEMY_3_6_ANIM_TAB_[] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x40, 0x00,
    0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x00, 0x40,
};
void objEnemyDraw3_6(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        ((u8*)cg_enemy3_6)[13] = ENEMY_3_6_ANIM_TAB_[p_obj->u_obj_work.enemy.anim_ct & 0x0f];
        vVramDraw3x3(draw_addr, p_obj->b_hit ? cg_enemy_damage3_6 : cg_enemy3_6);
    }
}
static const u8 ENEMY_4_1_ANIM_TAB_[] = {
    0xfe, 0xfd, 0xfb, 0xf7,
    0xf6, 0xf9, 0xf9, 0xf6,
};
void objEnemyDraw4_1(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        const u8* p = &ENEMY_4_1_ANIM_TAB_[p_obj->u_obj_work.enemy.anim_ct & 0x04];
        ((u8*)cg_enemy4_1)[ 5] = *p++;
        ((u8*)cg_enemy4_1)[ 6] = *p++;
        ((u8*)cg_enemy4_1)[ 9] = *p++;
        ((u8*)cg_enemy4_1)[10] = *p;
        vVramDraw4x4(draw_addr, p_obj->b_hit ? cg_enemy_damage4 : cg_enemy4_1);
    }
}
static const u8 ENEMY_4_2_ANIM_TAB_[] = { 0x73, 0x63, 0x52, 0x42, 0x32, 0x42, 0x52, 0x63 };
void objEnemyDraw4_2(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        const u8 c = ENEMY_4_2_ANIM_TAB_[p_obj->u_obj_work.enemy.anim_ct & 7];
        ((u8*)cg_enemy4_2)[16 +  5] = c;
        ((u8*)cg_enemy4_2)[16 +  6] = c;
        ((u8*)cg_enemy4_2)[16 +  9] = c;
        ((u8*)cg_enemy4_2)[16 + 10] = c;
        vVramDraw4x4(draw_addr, p_obj->b_hit ? cg_enemy_damage4 : cg_enemy4_2);
    }
}
static const u8 ENEMY_4_3_ANIM_TAB_[] = {
    0xf2, 0xf9, 0xf9, 0xf4,
    0xf4, 0xf9, 0xf9, 0xf2,
    0xf6, 0xf8, 0xf1, 0xf6,
    0xf6, 0xf1, 0xf8, 0xf6,
};
void objEnemyDraw4_3(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        const u8* p = &ENEMY_4_3_ANIM_TAB_[(p_obj->u_obj_work.enemy.anim_ct << 1) & 0x0c];
        ((u8*)cg_enemy4_3)[ 5] = *p++;
        ((u8*)cg_enemy4_3)[ 6] = *p++;
        ((u8*)cg_enemy4_3)[ 9] = *p++;
        ((u8*)cg_enemy4_3)[10] = *p;
        vVramDraw4x4(draw_addr, p_obj->b_hit ? cg_enemy_damage4 : cg_enemy4_3);
    }
}


static const u8 ENEMY_5_1_ANIM_TAB_[] = {
    0xf3, 0xf5, 0xf0, 0xfa, 0xfc,
    0xff, 0xff, 0xf0, 0xff, 0xff,
    0xfc, 0xfa, 0xff, 0xf5, 0xf3,
    0xf0, 0xf0, 0xff, 0xf0, 0xf0,
};
void objEnemyDraw5_1(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        const u8* p = &ENEMY_5_1_ANIM_TAB_[((p_obj->u_obj_work.enemy.anim_ct >> 1) & 0x03) * 5];
        ((u8*)cg_enemy5_1)[ 7] = *p++;
        ((u8*)cg_enemy5_1)[11] = *p++;
        ((u8*)cg_enemy5_1)[12] = *p++;
        ((u8*)cg_enemy5_1)[13] = *p++;
        ((u8*)cg_enemy5_1)[17] = *p;
        vVramDraw5x5(draw_addr, p_obj->b_hit ? cg_enemy_damage5 : cg_enemy5_1);
    }
}

static const u8 ENEMY_5_2_ANIM_TAB_[] = {
    0x06, 0x16, 0x26, 0x36, 0x46, 0x56, 0x66, 0x76,
    0x76, 0x66, 0x56, 0x46, 0x36, 0x26, 0x16, 0x06,
};
void objEnemyDraw5_2(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        const u8 c = ENEMY_5_2_ANIM_TAB_[p_obj->u_obj_work.enemy.anim_ct & 0x0f];
        ((u8*)cg_enemy5_2)[25 +  7] = c;
        ((u8*)cg_enemy5_2)[25 + 11] = c;
        ((u8*)cg_enemy5_2)[25 + 13] = c;
        ((u8*)cg_enemy5_2)[25 + 17] = c;
        ((u8*)cg_enemy5_2)[     12] = (rand8() & 0x0f) + 0xf0;
        vVramDraw5x5(draw_addr, p_obj->b_hit ? cg_enemy_damage5 : cg_enemy5_2);
    }
}

static const u8 ENEMY_5_3_ANIM_TAB_[] = {
    0xf0, 0xd0, 0xd0, 0x90,
    0x70, 0x50, 0x50, 0x10,
};
void objEnemyDraw5_3(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        const u8* p = &ENEMY_5_3_ANIM_TAB_[(p_obj->u_obj_work.enemy.anim_ct >> 1) & 0x04];
        ((u8*)cg_enemy5_3)[25 +  0] = *p++;
        ((u8*)cg_enemy5_3)[25 +  4] = *p++;
        ((u8*)cg_enemy5_3)[25 + 20] = *p++;
        ((u8*)cg_enemy5_3)[25 + 24] = *p;
        vVramDraw5x5(draw_addr, p_obj->b_hit ? cg_enemy_damage5 : cg_enemy5_3);
    }
}


static const u8 ENEMY8_1_ANIM_TAB_[] = {
    0x05, 0x15, 0x25, 0x35, 0x45, 0x55, 0x65, 0x75,
    0x75, 0x65, 0x55, 0x45, 0x35, 0x25, 0x15, 0x05,
};
static void objEnemyDraw8_1Anim_(Obj* const p_obj) __z88dk_fastcall
{
    u8 ct = p_obj->u_obj_work.enemy.anim_ct;
    {
        const u8 atb = (ct & 0x0c) ? 0x60 : 0x20;
        ((u8*)cg_enemy8_1)[64 +  0] = atb;
        ((u8*)cg_enemy8_1)[64 +  7] = atb;
        ((u8*)cg_enemy8_1)[64 + 56] = atb;
        ((u8*)cg_enemy8_1)[64 + 63] = atb;
    }
    {
        const u8 atb = ENEMY8_1_ANIM_TAB_[ct & 0x0f];
        ((u8*)cg_enemy8_1)[64 +  17] = atb;
        ((u8*)cg_enemy8_1)[64 +  22] = atb;
        ((u8*)cg_enemy8_1)[64 +  41] = atb;
        ((u8*)cg_enemy8_1)[64 +  46] = atb;
    }

    // 体力を表示
    u8 f = (p_obj->fitness + 1023) / 1024;
    ((u8*)cg_enemy8_1)[11] = DC_0 + f / 10;
    ((u8*)cg_enemy8_1)[12] = DC_0 + f % 10;
}

void objEnemyDraw8_1(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) {
        const u8* e = cg_enemy8_1;
        if (p_obj->b_hit) {
            e = cg_enemy_damage8;
        }
        objEnemyDraw8_1Anim_(p_obj);
        vVramDrawRect(draw_addr, e, W8H8(8, 8));
    }
}
void objEnemyDraw8_2(Obj* const p_obj, u8* draw_addr)
{
    objEnemyDraw8_1(p_obj, draw_addr);
}
static const u8 ENEMY_DRAW_8_3_TAB_[] = { 0x03, 0x0f, 0x3f, 0x00, 0x00};
void objEnemyDraw8_3(Obj* const p_obj, u8* draw_addr)
{
    if (!(p_obj->ct & 1)) { // 点滅
        const u8* e = cg_enemy8_1;
        if (p_obj->b_hit) {
            e = cg_enemy_damage8;
        } else {// 赤点滅
            u16 f = p_obj->fitness;
            if (f) { // 死んでたら点滅しない
                f /= ((u16)FITNESS_8_3 / 4);
                if ((p_obj->u_obj_work.enemy.anim_ct & ENEMY_DRAW_8_3_TAB_[f]) == 1) { // テーブル値が 0x00 ならば点滅しない
                    e = cg_enemy_red8;
                }
            }
        }
        objEnemyDraw8_1Anim_(p_obj);
        vVramDrawRect(draw_addr, e, W8H8(8, 8));
    }
}

// ---------------------------------------------------------------- 倒した敵の数