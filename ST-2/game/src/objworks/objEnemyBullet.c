/**
 * プレイヤー弾オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */
#include "../../../../src-common/common.h"
#include "../system/addr.h"
#include "../system/obj.h"
#include "../system/vram.h"
#include "../system/sound.h"
#include "../system/math.h"
//#include "../system/print.h"//TEST
#include "objPlayer.h"
#include "objEnemyBullet.h"


#define ENEMY_BULLET_W 1
#define ENEMY_BULLET_H 1

// ---------------------------------------------------------------- 初期化
void objEnemyBulletInit(Obj* const pObj, Obj* const pParent)
{
    // -------- 位置の確定
    s8 x = pParent->uGeo.geo8.xh + pParent->uGeo.geo.w / 2 - 1;
    s8 y = pParent->uGeo.geo8.yh + pParent->uGeo.geo.h / 2 - 1;
    pObj->uGeo.geo8.xh = x;
    pObj->uGeo.geo8.yh = y;
    pObj->uGeo.geo8.xl = 0x80;
    pObj->uGeo.geo8.yl = 0x80;

    // -------- 移動方向はプレーヤー向け
    const Obj* const pPlayerObj = objGetInUsePlayer();
    s8 dx = pPlayerObj->uGeo.geo8.xh     - x;
    s8 dy = pPlayerObj->uGeo.geo8.yh + 1 - y;
    u16 dxdy = ((u16)dx << 8) | ((u16)dy & 0xff);
    u8  a = atan2(dxdy);
    //printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 10));printHex16(dx);printAddAddr(1);printHex16(dy);printAddAddr(1);printHex16(dxdy);printAddAddr(1);printHex16(a);
    pObj->uGeo.geo.sx = cos(a) * 1;
    pObj->uGeo.geo.sy = sin(a) * 1;

    // -------- 寸法
    pObj->uGeo.geo.w = ENEMY_BULLET_W;
    pObj->uGeo.geo.h = ENEMY_BULLET_H;
    //OBJ_INIT(pObj, x, y, ENEMY_BULLET_W, ENEMY_BULLET_H, 0, 0); 不要
}

void objEnemyBulletInitWithoutVelocity(Obj* const pObj, Obj* const pParent)
{
    // -------- 位置の確定
    pObj->uGeo.geo8.xh = pParent->uGeo.geo8.xh + pParent->uGeo.geo.w / 2 - 1;
    pObj->uGeo.geo8.yh = pParent->uGeo.geo8.yh + pParent->uGeo.geo.h / 2 - 1;
    pObj->uGeo.geo8.xl = 0x80;
    pObj->uGeo.geo8.yl = 0x80;
    // -------- 寸法
    pObj->uGeo.geo.w = ENEMY_BULLET_W;
    pObj->uGeo.geo.h = ENEMY_BULLET_H;
    // 速度はあとで
}

// ---------------------------------------------------------------- メイン
bool objEnemyBulletMain(Obj* const pObj)
{
    if (pObj->uGeo.geo8.xh < 0) { return false; }
    if (pObj->uGeo.geo8.yh < 0) { return false; }
    if (VRAM_WIDTH  <= pObj->uGeo.geo8.xh) { return false; }
    if (VRAM_HEIGHT <= pObj->uGeo.geo8.yh) { return false; }
    return true;
}

// ---------------------------------------------------------------- 描画
// 次の 4 パターン
//
// 0＊＊．． 1．＊＊．
//  ＊＊．．  ．＊＊．
//  ．．．．  ．．．．
//  ．．．．  ．．．．
//
// 2．．．． 3．．．．
//  ＊＊．．  ．＊＊．
//  ＊＊．．  ．＊＊．
//  ．．．．  ．．．．
//
#pragma disable_warning 85  // pObj 未使用
#pragma save
void objEnemyBulletDraw(Obj* const pObj, u8* drawAddr)
{
#if 0 // C 版
    if ((u8)pObj->uGeo.geo8.xl < 0x80) {
        if ((u8)pObj->uGeo.geo8.yl < 0x80) {
            *drawAddr = 0xff;
            *(drawAddr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) = 0x70;
        } else {
            *drawAddr           = 0xfc;
            *(drawAddr + 0x100) = 0xf3;
            *(drawAddr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)        ) = 0x70;
            *(drawAddr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) + 0x100) = 0x70;
        }
    } else {
        if ((u8)pObj->uGeo.geo8.yl < 0x80) {
            *drawAddr       = 0xfa;
            *(drawAddr + 1) = 0xf5;
            *(drawAddr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)    ) = 0x70;
            *(drawAddr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) + 1) = 0x70;
        } else {
            *drawAddr           = 0xf8;
            *(drawAddr + 1)     = 0xf4;
            *(drawAddr + 0x100) = 0xf2;
            *(drawAddr + 0x101) = 0xf1;
            *(drawAddr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)        ) = 0x70;
            *(drawAddr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) +     1) = 0x70;
            *(drawAddr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) + 0x100) = 0x70;
            *(drawAddr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) + 0x101) = 0x70;
        }
    }
#else // ASM 版(重ね合わせ処理付き)
#define DRAW_BULLET(label, pat)     \
    ld      A, (HL)                 \
    cmp     A, B        ; B = 0xf0  \
    jr      nc, label               \
    ld      A, B        ; B = 0xf0  \
    ;                               \
label:                              \
    or      A, pat                  \
    ld      (HL), A

STATIC_ASSERT(2 == OBJ_OFFSET_GEO8_XL,                      Asm1); // ※1 修正
STATIC_ASSERT(3 <  OBJ_OFFSET_GEO8_YL - OBJ_OFFSET_GEO8_XL, Asm2); // ※2 修正

__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     DE                      // pObj
    pop     HL                      // drawAddr
    ld      BC, 0xf070              // B = 疑似グラフィックコード  C = ATB

    inc     E                       // ※1
    inc     E                       // ※1

    ld      A, (DE)                 // pObj->uGeo.geo8.xl
    cp      A, 0x80
    jp      nc, BULLET_DRAW_2

    ld      A, E                                            // ※2
    add     A, #(OBJ_OFFSET_GEO8_YL - OBJ_OFFSET_GEO8_XL)   // ※2
    ld      E, A                                            // ※2

    ld      A, (DE)                 // pObj->uGeo.geo8.yl
    cp      A, 0x80
    jp      nc, BULLET_DRAW_1

    // -------- パターン 0
    // TEXT
    ld      (HL), 0xff
    ld      A, #(VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))
    add     A, L
    ld      L, A

    // ATB
    ld      (HL), C     // C = ATB

    // スタックポインタを戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
    ret

BULLET_DRAW_1:
    // -------- パターン 1
    // TEXT
    DRAW_BULLET(BULLET_DRAW_1_1, 0x0c)
    inc     H
    DRAW_BULLET(BULLET_DRAW_1_2, 0x03)
    ld      A, #(VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))
    add     A, L
    ld      L, A

    // ATB
    ld      (HL), C     // C = ATB
    dec     H
    ld      (HL), C     // C = ATB

    // スタックポインタを戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
    ret

BULLET_DRAW_2:
    ld      A, E                                            // ※2
    add     A, #(OBJ_OFFSET_GEO8_YL - OBJ_OFFSET_GEO8_XL)   // ※2
    ld      E, A                                            // ※2

    ld      A, (DE)                 // pObj->uGeo.geo8.yl
    cp      A, 0x80
    jp      nc, BULLET_DRAW_3

    // -------- パターン 2
    // TEXT
    DRAW_BULLET(BULLET_DRAW_2_1, 0x0a)
    inc     L
    DRAW_BULLET(BULLET_DRAW_2_2, 0x05)
    ld      A, #(VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))
    add     A, L
    ld      L, A

    // ATB
    ld      (HL), C     // C = ATB
    dec     L
    ld      (HL), C     // C = ATB

    // スタックポインタを戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
    ret

BULLET_DRAW_3:
    // -------- パターン 3
    // TEXT
    DRAW_BULLET(BULLET_DRAW_3_1, 0x08)
    inc     L
    DRAW_BULLET(BULLET_DRAW_3_2, 0x04)
    inc     H
    DRAW_BULLET(BULLET_DRAW_3_3, 0x01)
    dec     L
    DRAW_BULLET(BULLET_DRAW_3_4, 0x02)
    ld      A, #(VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))
    add     A, L
    ld      L, A

    // ATB
    ld      (HL), C     // C = ATB
    inc     L
    ld      (HL), C     // C = ATB
    dec     H
    ld      (HL), C     // C = ATB
    dec     L
    ld      (HL), C     // C = ATB

    // スタックポインタを戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
__endasm;
#endif
}
#pragma restore


// ---------------------------------------------------------------- ユーティリティ
#if 0 // 当面使わない
void objEnemyBulletSetPosDir(Obj* const pObj, const u16 xy)
{
#if 0 // C 版
    s8 xh = xy >> 8;
    s8 yh = xy;

    pObj->uGeo.geo8.xh = xh;
    pObj->uGeo.geo8.yh = yh;
    pObj->uGeo.geo8.xl = 0x80;
    pObj->uGeo.geo8.yl = 0x80;

    const Obj* const pPlayerObj = objGetInUsePlayer();
    s8 dx = pPlayerObj->uGeo.geo8.xh + 1 - xh;
    s8 dy = pPlayerObj->uGeo.geo8.yh + 1 - yh;
    u16 dxdy = ((u16)dx << 8) | ((u16)dy & 0xff);
    u8  a = atan2(dxdy);
    //printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 10));printHex16(dx);printAddAddr(1);printHex16(dy);printAddAddr(1);printHex16(dxdy);printAddAddr(1);printHex16(a);
    pObj->uGeo.geo.sx = cos(a) * 4;
    pObj->uGeo.geo.sy = sin(a) * 4;

#else   // ASM 版. pObj 上にスタックを置いて, push で 16bit 値を書き込んでいく. C 版より 80 bytes 節約
STATIC_ASSERT(3 <  OBJ_OFFSET_GEO_Y,                        Asm1); // ※1 修正
STATIC_ASSERT(3 == OBJ_OFFSET_GEO8_XH,                      Asm2); // ※2 修正
STATIC_ASSERT(3 <  OBJ_OFFSET_GEO8_YH - OBJ_OFFSET_GEO8_XH, Asm3); // ※3 修正
__asm
    ld      (VRAM_ADDR), SP
    pop     HL                      // リターン アドレス(捨てる)
    pop     HL                      // pObj
    pop     DE                      // xy
    ld      A, #(OBJ_OFFSET_GEO_Y + 2)  // ※1
    add     A, L                        // ※1
    ld      L, A                        // ※1
    ld      SP, HL                      // pObj->uGeo.geo.y + 2 位置にスタックを置く. ワークが call で壊れるけどヨシ!

    // -------- プレーヤーのdxdy を求める
    call    _objPlayerGet               // HL =  pObjPlayer. 多分他のレジスタは大丈夫な筈. pObj->uGeo.geo.sy 破壊
    // HL += 3
    inc     L                           // ※2
    inc     L                           // ※2
    inc     L                           // ※2
    // B = dx
    ld      A, (HL)                     //       pObjPlayer->uGeo.geo8.xh
    inc     A                           //       pObjPlayer->uGeo.geo8.xh + 1
    sub     A, D                        //       pObjPlayer->uGeo.geo8.xh + 1 - x
    ld      B, A                        // dx =  pObjPlayer->uGeo.geo8.xh + 1 - x
    // HL += 5
    ld      A, #(OBJ_OFFSET_GEO8_YH - OBJ_OFFSET_GEO8_XH)   // ※3
    add     A, L                                            // ※3
    ld      L, A                                            // ※3
    // L = dy
    ld      A, (HL)                     //       pObjPlayer->uGeo.geo8.yh
    inc     A                           //       pObjPlayer->uGeo.geo8.yh + 1
    sub     A, E                        //       pObjPlayer->uGeo.geo8.yh + 1 - y
    ld      L, A                        // dy =  pObjPlayer->uGeo.geo8.yh + 1 - y
    // HL = dxdy
    ld      H, B

    // -------- y 書込
    ld      B, E
    ld      C, 0x00
    push    BC                          // write pObj->uGeo.geo.y

    // -------- atan2, cos, sin
    call    _atan2;                     // L:角度. A, BC, HL 破壊. pObj->uGeo.geo.sy 破壊
    ld      H, #(ADDR_COS_TAB >> 8)
    ld      B, (HL)                     // B = sx
    ld      H, #(ADDR_SIN_TAB >> 8)
    ld      A, (HL)                     // A = sy

    // -------- sy 書込
    // 符号拡張 A -> HL
    ld      L, A
    rlca
    sbc     A, A
    ld      H, A
    // 2 倍
    add     HL, HL
    push    HL                          // write pObj->uGeo.geo.sy
    dec     SP                          // skip  pObj->uGeo.geo.w

    // -------- x 書込
    ld      E, 0x00                     // D = x
    push    DE                          // write pObj->uGeo.geo.x

    // -------- sx 書込
    ld      A, B                        // B = sx
    // 符号拡張 A -> HL
    ld      L, A
    rlca
    sbc     A, A
    ld      H, A
    // 2 倍
    add     HL, HL
    push    HL                          // write pObj->uGeo.geo.sx

    // スタックポインタを戻して ret
    ld      SP, (VRAM_ADDR)
__endasm;
#endif
}
#endif