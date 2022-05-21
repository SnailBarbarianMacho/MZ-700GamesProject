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
#include "obj_player.h"
#include "obj_enemy_bullet.h"


#define ENEMY_BULLET_W 1
#define ENEMY_BULLET_H 1

// ---------------------------------------------------------------- 初期化
void objEnemyBulletInit(Obj* const p_obj, Obj* const p_parent)
{
    // -------- 位置の確定
    s8 x = p_parent->u_geo.geo8.xh + p_parent->u_geo.geo.w / 2 - 1;
    s8 y = p_parent->u_geo.geo8.yh + p_parent->u_geo.geo.h / 2 - 1;
    p_obj->u_geo.geo8.xh = x;
    p_obj->u_geo.geo8.yh = y;
    p_obj->u_geo.geo8.xl = 0x80;
    p_obj->u_geo.geo8.yl = 0x80;

    // -------- 移動方向はプレーヤー向け
    const Obj* const pPlayerObj = objGetInUsePlayer();
    s8 dx = pPlayerObj->u_geo.geo8.xh     - x;
    s8 dy = pPlayerObj->u_geo.geo8.yh + 1 - y;
    u16 dxdy = ((u16)dx << 8) | ((u16)dy & 0xff);
    u8 a = atan2(dxdy);
    //printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 10));printHex16(dx);printAddAddr(1);printHex16(dy);printAddAddr(1);printHex16(dxdy);printAddAddr(1);printHex16(a);
    p_obj->u_geo.geo.sx = cos(a) * 1;
    p_obj->u_geo.geo.sy = sin(a) * 1;

    // -------- 寸法
    p_obj->u_geo.geo.w = ENEMY_BULLET_W;
    p_obj->u_geo.geo.h = ENEMY_BULLET_H;
    //OBJ_INIT(p_obj, x, y, ENEMY_BULLET_W, ENEMY_BULLET_H, 0, 0); 不要
}

void objEnemyBulletInitWithoutVelocity(Obj* const p_obj, Obj* const p_parent)
{
    // -------- 位置の確定
    p_obj->u_geo.geo8.xh = p_parent->u_geo.geo8.xh + p_parent->u_geo.geo.w / 2 - 1;
    p_obj->u_geo.geo8.yh = p_parent->u_geo.geo8.yh + p_parent->u_geo.geo.h / 2 - 1;
    p_obj->u_geo.geo8.xl = 0x80;
    p_obj->u_geo.geo8.yl = 0x80;
    // -------- 寸法
    p_obj->u_geo.geo.w = ENEMY_BULLET_W;
    p_obj->u_geo.geo.h = ENEMY_BULLET_H;
    // 速度はあとで
}

// ---------------------------------------------------------------- メイン
bool objEnemyBulletMain(Obj* const p_obj)
{
    if (p_obj->u_geo.geo8.xh < 0) { return false; }
    if (p_obj->u_geo.geo8.yh < 0) { return false; }
    if (VRAM_WIDTH  <= p_obj->u_geo.geo8.xh) { return false; }
    if (VRAM_HEIGHT <= p_obj->u_geo.geo8.yh) { return false; }
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
#pragma disable_warning 85  // p_obj 未使用
#pragma save
void objEnemyBulletDraw(Obj* const p_obj, u8* draw_addr)
{
#if 0 // C 版
    if ((u8)p_obj->u_geo.geo8.xl < 0x80) {
        if ((u8)p_obj->u_geo.geo8.yl < 0x80) {
            *draw_addr = 0xff;
            *(draw_addr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) = 0x70;
        } else {
            *draw_addr           = 0xfc;
            *(draw_addr + 0x100) = 0xf3;
            *(draw_addr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)        ) = 0x70;
            *(draw_addr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) + 0x100) = 0x70;
        }
    } else {
        if ((u8)p_obj->u_geo.geo8.yl < 0x80) {
            *draw_addr       = 0xfa;
            *(draw_addr + 1) = 0xf5;
            *(draw_addr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)    ) = 0x70;
            *(draw_addr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) + 1) = 0x70;
        } else {
            *draw_addr           = 0xf8;
            *(draw_addr + 1)     = 0xf4;
            *(draw_addr + 0x100) = 0xf2;
            *(draw_addr + 0x101) = 0xf1;
            *(draw_addr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)        ) = 0x70;
            *(draw_addr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) +     1) = 0x70;
            *(draw_addr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) + 0x100) = 0x70;
            *(draw_addr + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) + 0x101) = 0x70;
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
    pop     DE                      // p_obj
    pop     HL                      // draw_addr
    ld      BC, 0xf070              // B = 疑似グラフィックコード  C = ATB

    inc     E                       // ※1
    inc     E                       // ※1

    ld      A, (DE)                 // p_obj->u_geo.geo8.xl
    cp      A, 0x80
    jp      nc, BULLET_DRAW_2

    ld      A, E                                            // ※2
    add     A, #(OBJ_OFFSET_GEO8_YL - OBJ_OFFSET_GEO8_XL)   // ※2
    ld      E, A                                            // ※2

    ld      A, (DE)                 // p_obj->u_geo.geo8.yl
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

    ld      A, (DE)                 // p_obj->u_geo.geo8.yl
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
void objEnemyBulletSetPosDir(Obj* const p_obj, const u16 xy)
{
#if 0 // C 版
    s8 xh = xy >> 8;
    s8 yh = xy;

    p_obj->u_geo.geo8.xh = xh;
    p_obj->u_geo.geo8.yh = yh;
    p_obj->u_geo.geo8.xl = 0x80;
    p_obj->u_geo.geo8.yl = 0x80;

    const Obj* const pPlayerObj = objGetInUsePlayer();
    s8 dx = pPlayerObj->u_geo.geo8.xh + 1 - xh;
    s8 dy = pPlayerObj->u_geo.geo8.yh + 1 - yh;
    u16 dxdy = ((u16)dx << 8) | ((u16)dy & 0xff);
    u8  a = atan2(dxdy);
    //printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 10));printHex16(dx);printAddAddr(1);printHex16(dy);printAddAddr(1);printHex16(dxdy);printAddAddr(1);printHex16(a);
    p_obj->u_geo.geo.sx = cos(a) * 4;
    p_obj->u_geo.geo.sy = sin(a) * 4;

#else   // ASM 版. p_obj 上にスタックを置いて, push で 16bit 値を書き込んでいく. C 版より 80 bytes 節約
STATIC_ASSERT(3 <  OBJ_OFFSET_GEO_Y,                        Asm1); // ※1 修正
STATIC_ASSERT(3 == OBJ_OFFSET_GEO8_XH,                      Asm2); // ※2 修正
STATIC_ASSERT(3 <  OBJ_OFFSET_GEO8_YH - OBJ_OFFSET_GEO8_XH, Asm3); // ※3 修正
__asm
    ld      (VRAM_ADDR), SP
    pop     HL                      // リターン アドレス(捨てる)
    pop     HL                      // p_obj
    pop     DE                      // xy
    ld      A, #(OBJ_OFFSET_GEO_Y + 2)  // ※1
    add     A, L                        // ※1
    ld      L, A                        // ※1
    ld      SP, HL                      // p_obj->u_geo.geo.y + 2 位置にスタックを置く. ワークが call で壊れるけどヨシ!

    // -------- プレーヤーのdxdy を求める
    call    _objPlayerGet               // HL =  pObjPlayer. 多分他のレジスタは大丈夫な筈. p_obj->u_geo.geo.sy 破壊
    // HL += 3
    inc     L                           // ※2
    inc     L                           // ※2
    inc     L                           // ※2
    // B = dx
    ld      A, (HL)                     //       pObjPlayer->u_geo.geo8.xh
    inc     A                           //       pObjPlayer->u_geo.geo8.xh + 1
    sub     A, D                        //       pObjPlayer->u_geo.geo8.xh + 1 - x
    ld      B, A                        // dx =  pObjPlayer->u_geo.geo8.xh + 1 - x
    // HL += 5
    ld      A, #(OBJ_OFFSET_GEO8_YH - OBJ_OFFSET_GEO8_XH)   // ※3
    add     A, L                                            // ※3
    ld      L, A                                            // ※3
    // L = dy
    ld      A, (HL)                     //       pObjPlayer->u_geo.geo8.yh
    inc     A                           //       pObjPlayer->u_geo.geo8.yh + 1
    sub     A, E                        //       pObjPlayer->u_geo.geo8.yh + 1 - y
    ld      L, A                        // dy =  pObjPlayer->u_geo.geo8.yh + 1 - y
    // HL = dxdy
    ld      H, B

    // -------- y 書込
    ld      B, E
    ld      C, 0x00
    push    BC                          // write p_obj->u_geo.geo.y

    // -------- atan2, cos, sin
    call    _atan2;                     // L:角度. A, BC, HL 破壊. p_obj->u_geo.geo.sy 破壊
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
    push    HL                          // write p_obj->u_geo.geo.sy
    dec     SP                          // skip  p_obj->u_geo.geo.w

    // -------- x 書込
    ld      E, 0x00                     // D = x
    push    DE                          // write p_obj->u_geo.geo.x

    // -------- sx 書込
    ld      A, B                        // B = sx
    // 符号拡張 A -> HL
    ld      L, A
    rlca
    sbc     A, A
    ld      H, A
    // 2 倍
    add     HL, HL
    push    HL                          // write p_obj->u_geo.geo.sx

    // スタックポインタを戻して ret
    ld      SP, (VRAM_ADDR)
__endasm;
#endif
}
#endif