/**
 * プレイヤー弾オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/addr.h"
#include "../system/obj.h"
#include "../system/vram.h"
#include "../system/sound.h"
#include "../game/score.h"
#include "obj_player_bullet.h"

#define PLAYER_BULLET_W 3
#define PLAYER_BULLET_H 3

// ---------------------------------------------------------------- 初期化
// 攻撃力別の弾のテキストとATB
static const u8 TEXT_TAB_[] = {
    // 攻撃力0～ 16～         32～         48～         64～         80～         96～         112～
    0x3d, 0x71,  0x3f, 0x37,  0x7f, 0x7b,  0x3b, 0x7b,  0x7b, 0x7f,  0x37, 0x3f,  0x71, 0x3d,  0x43, 0x43,
};
static const u8 ATB_TAB_[] = {
    // 攻撃力0～ 16～         32～         48～         64～         80～         96～         112～
    0x20, 0x20,  0x20, 0x20,  0x20, 0x20,  0x20, 0x20,  0x02, 0x02,  0x02, 0x02,  0x02, 0x02,  0x20, 0x20,  // +0, 1, 2
    0x30, 0x30,  0x30, 0x30,  0x30, 0x30,  0x30, 0x30,  0x03, 0x03,  0x03, 0x03,  0x03, 0x03,  0x30, 0x30,  // +3, 4, 5
    0x40, 0x40,  0x40, 0x40,  0x40, 0x40,  0x40, 0x40,  0x04, 0x04,  0x04, 0x04,  0x04, 0x04,  0x40, 0x40,  // +6, 7, 8
    0x50, 0x50,  0x50, 0x50,  0x50, 0x50,  0x50, 0x50,  0x05, 0x05,  0x05, 0x05,  0x05, 0x05,  0x50, 0x50,  // +9, 10, 11
    0x60, 0x60,  0x60, 0x60,  0x60, 0x60,  0x60, 0x60,  0x06, 0x06,  0x06, 0x06,  0x06, 0x06,  0x60, 0x60,  // +12, 13
    0x70, 0x70,  0x70, 0x70,  0x70, 0x70,  0x70, 0x70,  0x07, 0x07,  0x07, 0x07,  0x07, 0x07,  0x70, 0x70,  // +14, 15
};
static const u8 PLAYER_BULLET_TAB_[] = {
    0x00, 0x00, 0x00, 0x10,
    0x10, 0x10, 0x20, 0x20,
    0x20, 0x30, 0x30, 0x30,
    0x40, 0x40, 0x50, 0x50,
};
void objPlayerBulletInit(Obj* const p_obj, Obj* const p_parent)
{
    OBJ_INIT(p_obj, 0, 0, PLAYER_BULLET_W, PLAYER_BULLET_H, 0, -PLAYER_BULLET_H << 8);
    p_obj->u_geo.geo8.xh = p_parent->u_geo.geo8.xh - 1;
    p_obj->u_geo.geo8.yh = p_parent->u_geo.geo8.yh - 2;

    // 弾の攻撃力と TEXT/ATB 設定
    u8 offence = scoreGetLevel();
    if (127 <= offence) { offence = 127; }
    p_obj->offence = offence;

    u8 atb  = PLAYER_BULLET_TAB_[offence & 0x0f];
    offence = (offence & 0xf0) >> 3;

    const u8* p_text = &TEXT_TAB_[offence];
    p_obj->u_obj_work.player_bullet.text1 = *p_text++;
    p_obj->u_obj_work.player_bullet.text2 = *p_text;

    const u8* p_atb = &ATB_TAB_[offence + atb];
    p_obj->u_obj_work.player_bullet.atb1 = *p_atb++;
    p_obj->u_obj_work.player_bullet.atb2 = *p_atb;
}


// ---------------------------------------------------------------- メイン
bool objPlayerBulletMain(Obj* const p_obj)
{
    if ((p_obj->u_geo.geo8.yh <= -PLAYER_BULLET_H) || (p_obj->b_hit)) {
        return false;
    }
    return true;
}

// ---------------------------------------------------------------- 描画
// レベル  レーザーの太さ レーザーの色
//  1～    1           2,2,2,3, 3,4,4,4, 5,5,6,6, 6,7,7,7
//  16～   2
//  32～   3
//  48～   4
//  64～   5
//  80～   6
//  96～   7
//  112～  8
#pragma disable_warning 85
#pragma save
static void drawBullet(u8* draw_addr) __z88dk_fastcall __naked
{
}
#pragma restore


#pragma disable_warning 85  // p_obj 未使用
#pragma save
void objPlayerBulletDraw(Obj* const p_obj, u8* draw_addr)
{
    STATIC_ASSERT(3 <  OBJ_OFFSET_WORK_PLAYER_BULLET_TEXT1,                                       "Asm1"); // ※1 を修正
    STATIC_ASSERT(1 == OBJ_OFFSET_WORK_PLAYER_BULLET_TEXT2 - OBJ_OFFSET_WORK_PLAYER_BULLET_TEXT1, "Asm2"); // ※2 を修正
    STATIC_ASSERT(1 == OBJ_OFFSET_WORK_PLAYER_BULLET_ATB1  - OBJ_OFFSET_WORK_PLAYER_BULLET_TEXT2, "Asm3"); // ※3 を修正
    STATIC_ASSERT(1 == OBJ_OFFSET_WORK_PLAYER_BULLET_ATB2  - OBJ_OFFSET_WORK_PLAYER_BULLET_ATB1,  "Asm4"); // ※4 を修正
__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     HL                      // p_obj

    // B = p_obj->u_obj_work.playerBullet.text1
    // C = p_obj->u_obj_work.playerBullet.text2
    // D = p_obj->u_obj_work.playerBullet.atb1
    // E = p_obj->u_obj_work.playerBullet.atb2
    ld      A, L                                // ※1
    add     A, 0 + OBJ_OFFSET_WORK_PLAYER_BULLET_TEXT1// ※1
    ld      L, A                                // ※1

    ld      B, (HL)
    inc     L                       // ※2
    ld      C, (HL)
    inc     L                       // ※3
    ld      D, (HL)
    inc     L                       // ※4
    ld      E, (HL)

    pop     HL                      // draw_addr

    // TEXT
    ld      (HL), B
    inc     H
    ld      (HL), B
    inc     L
    inc     L
    ld      (HL), C
    dec     H
    ld      (HL), C

    // L += addr
    ld      A, L
    add     A, 0 + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) - 2
    ld      L, A

    // ATB
    ld      (HL), D
    inc     H
    ld      (HL), D
    inc     L
    inc     L
    ld      (HL), E
    dec     H
    ld      (HL), E

    // スタックポインタを戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
__endasm;
}
#pragma restore
