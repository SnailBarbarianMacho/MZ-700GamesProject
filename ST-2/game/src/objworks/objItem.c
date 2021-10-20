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
//#include <string.h> // memcpy() 使わない
#include "objItem.h"

// ---------------------------------------------------------------- 定数, 変数
static u16 sNrGeneretedItems;
static u16 sNrObtainedItems;

static const u8 sTab[] = {
    0x0c, 0x00,  0x36, 0x36,  0x3a, 0x3a,  0x3e, 0x3e,  // text table
    0x71, 0x00,  0x01, 0x10,  0x10, 0x01,  0x10, 0x01,  // pObj->step == 1
    0x72, 0x00,  0x02, 0x20,  0x20, 0x02,  0x20, 0x02,  // pObj->step == 2
    0x73, 0x00,  0x03, 0x30,  0x30, 0x03,  0x30, 0x03,  // pObj->step == 3
    0x04, 0x00,  0x04, 0x40,  0x40, 0x04,  0x40, 0x04,  // pObj->step == 4
    0x05, 0x00,  0x05, 0x50,  0x50, 0x05,  0x50, 0x05,  // pObj->step == 5
    0x06, 0x00,  0x06, 0x60,  0x60, 0x06,  0x60, 0x06,  // pObj->step == 6
    0x07, 0x00,  0x07, 0x70,  0x70, 0x07,  0x70, 0x07,  // pObj->step == 7
};
#define SZ_TAB 0x40

// ---------------------------------------------------------------- サウンド
#define SE_GET_ITEM_CT     5

// アイテム取得音
static void seGetItem(u8 ct)
{
    sdMake((ct + 1) << 8);
}

// ---------------------------------------------------------------- システム
void objItemInitTab()
{
    //memcpy((u8*)ADDR_ITEM_TAB, sTab, SZ_TAB)
__asm
    ld  HL, #_sTab
    ld  DE, #(ADDR_ITEM_TAB)
    ld  BC, #(SZ_TAB)   // sizeof 演算子は流石に使えない
    ldir
__endasm;
}

// ---------------------------------------------------------------- アイテム数
void objItemInitStatistics()
{
    sNrGeneretedItems = 0;
    sNrObtainedItems = 0;
}
u16 objItemGetNrGeneretedItems()
{
    return sNrGeneretedItems;
}
u16 objItemGetNrObtainedItems()
{
    return sNrObtainedItems;
}

// ---------------------------------------------------------------- 初期化
#define ITEM_WIDTH  1
#define ITEM_HEIGHT 2
#define STEP_HOP    0   // 散らばっている時
#define STEP_MOVE   1   // 移動時

void objItemInit(Obj* const pObj, Obj* const pParent)
{
    OBJ_INIT(pObj,
        pParent->uGeo.geo.x + (pParent->uGeo.geo.w << 7),   // parent の中心 x
        pParent->uGeo.geo.y + (pParent->uGeo.geo.h << 7),   // parent の中心 y
        ITEM_WIDTH, ITEM_HEIGHT,
        (s16)rand8_sign() << 6, (s16)rand8_sign() << 6); // 速度は 0xffc0, 0x0000, 0x0040
    pObj->step = STEP_HOP;
    pObj->ct   = rand8() & 0x0f; // 最大 4 キャラ分まで散らばる
    pObj->uObjWork.item.subLevel = rand8() % pParent->uObjWork.enemy.itemSubLevel + 1;
    sNrGeneretedItems++;
}

// ---------------------------------------------------------------- メイン
bool objItemMain(Obj* const pObj)
{
    // アイテム ゲット処理
    if (pObj->bHit) {
        scoreAddSubLevel(pObj->uObjWork.item.subLevel * 2);
        scoreAdd(1);
        sdSetSeSequencer(seGetItem, SD_SE_PRIORITY_2, SE_GET_ITEM_CT);
        sNrObtainedItems++;
        return false;
    }

    // アイテムの移動
    switch (pObj->step) {
    case STEP_HOP:
            if (pObj->ct == 0) {
            pObj->step = STEP_MOVE;
            pObj->uGeo.geo.sx = 0x0000;
            pObj->uGeo.geo.sy = 0x0018;
        }
        pObj->ct--;
        break;
    default: break;
    }

    if (VRAM_HEIGHT <= pObj->uGeo.geo8.yh) {
        return false;
    }

    return true;
}

// ---------------------------------------------------------------- 描画
#pragma disable_warning 85  // pObj 未使用
#pragma save
#if 0   // C 版
void objItemDraw(Obj* const pObj, u8* drawAddr)
{
    switch (pObj->uGeo.geo8.yl / 0x40) {
        case 0:
            *(drawAddr) = 0x0c;
            *(drawAddr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))) = 0x06;
            break;
        case 1:
            *(drawAddr)       = 0x36;
            *(drawAddr+0x100) = 0x36;
            *(drawAddr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))      ) = 0x06;
            *(drawAddr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))+0x100) = 0x60;
            break;
        case 2:
            *(drawAddr)       = 0x3a;
            *(drawAddr+0x100) = 0x3a;
            *(drawAddr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))      ) = 0x60;
            *(drawAddr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))+0x100) = 0x06;
            break;
        case 3:
            *(drawAddr)       = 0x3e;
            *(drawAddr+0x100) = 0x3e;
            *(drawAddr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))      ) = 0x60;
            *(drawAddr + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))+0x100) = 0x06;
            break;
    }
#else   // ASM 版
void objItemDraw(Obj* const pObj, u8* drawAddr) __naked
{
STATIC_ASSERT(3 <= OBJ_OFFSET_GEO8_YL,                                  Asm1); // ※1 を修正
STATIC_ASSERT(4 <= OBJ_OFFSET_WORK_ITEM_SUB_LEVEL - OBJ_OFFSET_GEO8_YL, Asm2); // ※2 を修正
__asm
    pop     HL                          // リターン アドレス(捨てる)
    pop     DE                          // pObj

    // ---- uGeo.geo8.yl からテキストを選ぶ
    ld      A, E                        // ※1
    add     A, #(OBJ_OFFSET_GEO8_YL)    // ※1
    ld      E, A                        // ※1
    ld      A, (DE)                     // A = yl

    and     A, 0xc0                     // A = 0x00, 0x40, 0x80, 0xc0
    jr      z, OBJ_ITEM_DRAW_1          // A = 0x00 の場合は別ルーチン

    rlca                                // A = 0x00, 0x80, 0x01, 0x81
    rlca                                // A = 0x00, 0x01, 0x02, 0x03
    rlca                                // A = 0x00, 0x02, 0x04, 0x06

    ld      H, #(ADDR_ITEM_TAB >> 8)
    ld      L, A
    ld      B, (HL)                     // BC = TEXT
    inc     L
    ld      C, (HL)
    dec     L

    // ---- uGeo.geo8.yl と step から ATB を選ぶ
    ld      A, E                                                        // ※2
    add     A, #(OBJ_OFFSET_WORK_ITEM_SUB_LEVEL - OBJ_OFFSET_GEO8_YL)   // ※2
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
    pop     HL                          // drawAddr

    ld      (HL), B
    inc     H
    ld      (HL), C
    ld      A, L
    add     A, #(VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))
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
    add     A, #(OBJ_OFFSET_WORK_ITEM_SUB_LEVEL - OBJ_OFFSET_GEO8_YL)   // ※2
    ld      E, A                                                        // ※2
    ld      A, (DE)                     // A = step(1, 2, ..., 6)
    add     A, A                        // A =      2, 4, ..., 12
    add     A, A                        // A =      4, 8, ..., 24
    add     A, A                        // A =      8, 16, ..., 48
    ld      H, #(ADDR_ITEM_TAB >> 8)
    ld      L, A
    ld      D, (HL)                     // D = ATB

    // ---- 描画
    pop     HL                      // drawAddr
    ld      (HL), CHAR_L
    ld      A, L
    add     A, #(VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))
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
