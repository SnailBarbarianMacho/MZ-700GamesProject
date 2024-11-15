/**
 * アイテム オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/addr.h"
#include "../system/obj.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/sound.h"
#include "../system/math.h"
#include "../game/score.h"
#include "../game/se.h"
#include "../game/game_mode.h"
//#include <string.h> // memcpy() 使わない
#include "obj_item.h"


// ---------------------------------------------------------------- 定数, 変数
u16 obj_item_nr_generated_;
u16 obj_item_nr_obtained_;

static const u8 ITEM_TAB_[] = {
    0x0c, 0x00,  0x36, 0x36,  0x3a, 0x3a,  0x3e, 0x3e,  // text table
    0x71, 0x00,  0x01, 0x10,  0x10, 0x01,  0x10, 0x01,  // p_obj->step == 1
    0x72, 0x00,  0x02, 0x20,  0x20, 0x02,  0x20, 0x02,  // p_obj->step == 2
    0x73, 0x00,  0x03, 0x30,  0x30, 0x03,  0x30, 0x03,  // p_obj->step == 3
    0x04, 0x00,  0x04, 0x40,  0x40, 0x04,  0x40, 0x04,  // p_obj->step == 4
    0x05, 0x00,  0x05, 0x50,  0x50, 0x05,  0x50, 0x05,  // p_obj->step == 5
    0x06, 0x00,  0x06, 0x60,  0x60, 0x06,  0x60, 0x06,  // p_obj->step == 6
    0x07, 0x00,  0x07, 0x70,  0x70, 0x07,  0x70, 0x07,  // p_obj->step == 7
};
#define SZ_TAB 0x40

// ---------------------------------------------------------------- システム
void objItemInitTab(void)
{
    STATIC_ASSERT(sizeof(ITEM_TAB_) == SZ_TAB, "ITEM_TAB");
    //memcpy((u8*)ADDR_ITEM_TAB, sTab, SZ_TAB)
__asm
    ld  HL, 0 + _ITEM_TAB_
    ld  DE, 0 + ADDR_ITEM_TAB
    ld  BC, 0 + SZ_TAB  // 流石にインラインアセンブラ内では sizeof 演算子は使えなかった
    ldir
__endasm;
}

// ---------------------------------------------------------------- アイテム数

// ---------------------------------------------------------------- 初期化
#define ITEM_WIDTH  1
#define ITEM_HEIGHT 2
#define STEP_HOP    0   // 散らばっている時
#define STEP_MOVE   1   // 移動時

void objItemInit(Obj* const p_obj, Obj* const p_parent)
{
    OBJ_INIT(p_obj,
        p_parent->u_geo.geo.x + (p_parent->u_geo.geo.w << 7),   // parent の中心 x
        p_parent->u_geo.geo.y + (p_parent->u_geo.geo.h << 7),   // parent の中心 y
        ITEM_WIDTH, ITEM_HEIGHT,
        (s16)rand8_sign() << 6, (s16)rand8_sign() << 6); // 速度は 0xffc0, 0x0000, 0x0040
    p_obj->step = STEP_HOP;
    p_obj->ct   = rand8() & 0x0f; // 最大 4 キャラ分まで散らばる
    p_obj->u_obj_work.item.sub_level = rand8() % p_parent->u_obj_work.enemy.item_sub_level + 1;
    obj_item_nr_generated_++;
}

// ---------------------------------------------------------------- メイン
bool objItemMain(Obj* const p_obj)
{
    // アイテム ゲット処理
    if (p_obj->b_hit) {
        u8 addSubLev = p_obj->u_obj_work.item.sub_level * 2;
        if (gameIsCaravan()) {
            addSubLev *= 2;
        }
        scoreAddSubLevel(addSubLev);
        scoreAdd(1);
        sdPlaySe(SE_GET_ITEM);
        obj_item_nr_obtained_++;
        return false;
    }

    // アイテムの移動
    switch (p_obj->step) {
    case STEP_HOP:
            if (p_obj->ct == 0) {
            p_obj->step = STEP_MOVE;
            p_obj->u_geo.geo.sx = 0x0000;
            p_obj->u_geo.geo.sy = 0x0018;
        }
        p_obj->ct--;
        break;
    default: break;
    }

    if (VRAM_HEIGHT <= p_obj->u_geo.geo8.yh) {
        return false;
    }

    return true;
}

// ---------------------------------------------------------------- 描画
#pragma disable_warning 85  // p_obj 未使用
#pragma save
#if 0   // C 版
void objItemDraw(Obj* const p_obj, u8* draw_addr)
{
    switch (p_obj->u_geo.geo8.yl / 0x40) {
        case 0:
            *(draw_addr) = 0x0c;
            *(draw_addr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))) = 0x06;
            break;
        case 1:
            *(draw_addr)       = 0x36;
            *(draw_addr+0x100) = 0x36;
            *(draw_addr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))      ) = 0x06;
            *(draw_addr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))+0x100) = 0x60;
            break;
        case 2:
            *(draw_addr)       = 0x3a;
            *(draw_addr+0x100) = 0x3a;
            *(draw_addr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))      ) = 0x60;
            *(draw_addr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))+0x100) = 0x06;
            break;
        case 3:
            *(draw_addr)       = 0x3e;
            *(draw_addr+0x100) = 0x3e;
            *(draw_addr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))      ) = 0x60;
            *(draw_addr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))+0x100) = 0x06;
            break;
    }
#else   // ASM 版
void objItemDraw(Obj* const p_obj, u8* draw_addr) __naked
{
    STATIC_ASSERT(3 <= OBJ_OFFSET_GEO8_YL,                                  "Asm1"); // ※1 を修正
    STATIC_ASSERT(4 <= OBJ_OFFSET_WORK_ITEM_SUB_LEVEL - OBJ_OFFSET_GEO8_YL, "Asm2"); // ※2 を修正
__asm
    pop     HL                          // リターン アドレス(捨てる)
    pop     DE                          // p_obj

    // ---- u_geo.geo8.yl からテキストを選ぶ
    ld      A, E                        // ※1
    add     A, 0 + OBJ_OFFSET_GEO8_YL   // ※1
    ld      E, A                        // ※1
    ld      A, (DE)                     // A = yl

    and     A, 0xc0                     // A = 0x00, 0x40, 0x80, 0xc0
    jr      z, OBJ_ITEM_DRAW_1          // A = 0x00 の場合は別ルーチン

    rlca                                // A = 0x00, 0x80, 0x01, 0x81
    rlca                                // A = 0x00, 0x01, 0x02, 0x03
    rlca                                // A = 0x00, 0x02, 0x04, 0x06

    ld      H, 0 + ADDR_ITEM_TAB >> 8
    ld      L, A
    ld      B, (HL)                     // BC = TEXT
    inc     L
    ld      C, (HL)
    dec     L

    // ---- u_geo.geo8.yl と step から ATB を選ぶ
    ld      A, E                                                        // ※2
    add     A, 0 + OBJ_OFFSET_WORK_ITEM_SUB_LEVEL - OBJ_OFFSET_GEO8_YL  // ※2
    ld      E, A                                                        // ※2
    ld      A, (DE)                     // A = step(1, 2, ..., 6)
    add     A, A                        // A =      2, 4, ..., 12
    add     A, A                        // A =      4, 8, ..., 24
    add     A, A                        // A =      8, 16, ..., 48
    add     A, L
    ld      L, A
    ld      D, (HL)                     // DE = ATB
    inc     L
    ld      E, (HL)

    // ---- 描画
    pop     HL                          // draw_addr

    ld      (HL), B
    inc     H
    ld      (HL), C
    ld      A, L
    add     A, 0 + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
    ld      L, A
    ld      (HL), E
    dec     H
    ld      (HL), D

    // ---- スタックポインタを戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL

    ret

    // 1x1 のみの場合の描画
OBJ_ITEM_DRAW_1:
    // ---- step から ATB を選ぶ
    ld      A, E                                                        // ※2
    add     A, 0 + OBJ_OFFSET_WORK_ITEM_SUB_LEVEL - OBJ_OFFSET_GEO8_YL  // ※2
    ld      E, A                                                        // ※2
    ld      A, (DE)                     // A = step(1, 2, ..., 6)
    add     A, A                        // A =      2, 4, ..., 12
    add     A, A                        // A =      4, 8, ..., 24
    add     A, A                        // A =      8, 16, ..., 48
    ld      H, ADDR_ITEM_TAB >> 8
    ld      L, A
    ld      D, (HL)                     // D = ATB

    // ---- 描画
    pop     HL                      // draw_addr
    ld      (HL), DC_L
    ld      A, L
    add     A, 0 + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
    ld      L, A
    ld      (HL), D

    // ---- スタックポインタを戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL

    ret
__endasm;
#endif
}

#pragma restore
