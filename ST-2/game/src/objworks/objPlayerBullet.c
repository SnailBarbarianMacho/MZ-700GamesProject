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
#include "objPlayerBullet.h"

#define PLAYER_BULLET_W 3
#define PLAYER_BULLET_H 3

// ---------------------------------------------------------------- 初期化
// 攻撃力別の弾のテキストとATB
static const u8 sTextTab[] = {
    // 攻撃力0～ 16～         32～         48～         64～         80～         96～         112～
    0x3d, 0x71,  0x3f, 0x37,  0x7f, 0x7b,  0x3b, 0x7b,  0x7b, 0x7f,  0x37, 0x3f,  0x71, 0x3d,  0x43, 0x43,
};
static const u8 sAtbTab[] = {
    // 攻撃力0～ 16～         32～         48～         64～         80～         96～         112～
    0x20, 0x20,  0x20, 0x20,  0x20, 0x20,  0x20, 0x20,  0x02, 0x02,  0x02, 0x02,  0x02, 0x02,  0x20, 0x20,  // +0, 1, 2
    0x30, 0x30,  0x30, 0x30,  0x30, 0x30,  0x30, 0x30,  0x03, 0x03,  0x03, 0x03,  0x03, 0x03,  0x30, 0x30,  // +3, 4, 5
    0x40, 0x40,  0x40, 0x40,  0x40, 0x40,  0x40, 0x40,  0x04, 0x04,  0x04, 0x04,  0x04, 0x04,  0x40, 0x40,  // +6, 7, 8
    0x50, 0x50,  0x50, 0x50,  0x50, 0x50,  0x50, 0x50,  0x05, 0x05,  0x05, 0x05,  0x05, 0x05,  0x50, 0x50,  // +9, 10, 11
    0x60, 0x60,  0x60, 0x60,  0x60, 0x60,  0x60, 0x60,  0x06, 0x06,  0x06, 0x06,  0x06, 0x06,  0x60, 0x60,  // +12, 13
    0x70, 0x70,  0x70, 0x70,  0x70, 0x70,  0x70, 0x70,  0x07, 0x07,  0x07, 0x07,  0x07, 0x07,  0x70, 0x70,  // +14, 15
};

void objPlayerBulletInit(Obj* const pObj, Obj* const pParent)
{
    OBJ_INIT(pObj, 0, 0, PLAYER_BULLET_W, PLAYER_BULLET_H, 0, -PLAYER_BULLET_H << 8);
    pObj->uGeo.geo8.xh = pParent->uGeo.geo8.xh - 1;
    pObj->uGeo.geo8.yh = pParent->uGeo.geo8.yh - 2;

    // 弾の攻撃力と TEXT/ATB 設定
    u8 offence = scoreGetLevel();
    if (127 <= offence) { offence = 127; }
    pObj->offence = offence;

    static const u8 tab[] = {
        0x00, 0x00, 0x00, 0x10,
        0x10, 0x10, 0x20, 0x20,
        0x20, 0x30, 0x30, 0x30,
        0x40, 0x40, 0x50, 0x50,
    };
    u8 atb  = tab[offence & 0x0f];
    offence = (offence & 0xf0) >> 3;

    const u8* pText = &sTextTab[offence];
    pObj->uObjWork.playerBullet.text1 = *pText++;
    pObj->uObjWork.playerBullet.text2 = *pText;

    const u8* pAtb = &sAtbTab[offence + atb];
    pObj->uObjWork.playerBullet.atb1 = *pAtb++;
    pObj->uObjWork.playerBullet.atb2 = *pAtb;
}


// ---------------------------------------------------------------- メイン
bool objPlayerBulletMain(Obj* const pObj)
{
    if ((pObj->uGeo.geo8.yh <= -PLAYER_BULLET_H) || (pObj->bHit)) {
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
static void drawBullet(u8* drawAddr) __z88dk_fastcall __naked
{
}
#pragma restore


#pragma disable_warning 85  // pObj 未使用
#pragma save
void objPlayerBulletDraw(Obj* const pObj, u8* drawAddr)
{
    STATIC_ASSERT(3 <  OBJ_OFFSET_WORK_PLAYER_BULLET_TEXT1,                                  Asm1); // ※1 を修正
    STATIC_ASSERT(1 == OBJ_OFFSET_WORK_PLAYER_BULLET_TEXT2 - OBJ_OFFSET_WORK_PLAYER_BULLET_TEXT1, Asm2); // ※2 を修正
    STATIC_ASSERT(1 == OBJ_OFFSET_WORK_PLAYER_BULLET_ATB1  - OBJ_OFFSET_WORK_PLAYER_BULLET_TEXT2, Asm3); // ※3 を修正
    STATIC_ASSERT(1 == OBJ_OFFSET_WORK_PLAYER_BULLET_ATB2  - OBJ_OFFSET_WORK_PLAYER_BULLET_ATB1,  Asm4); // ※4 を修正
__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     HL                      // pObj

    // B = pObj->uObjWork.playerBullet.text1
    // C = pObj->uObjWork.playerBullet.text2
    // D = pObj->uObjWork.playerBullet.atb1
    // E = pObj->uObjWork.playerBullet.atb2
    ld      A, L                                // ※1
    add     A, #(OBJ_OFFSET_WORK_PLAYER_BULLET_TEXT1)// ※1
    ld      L, A                                // ※1

    ld      B, (HL)
    inc     L                       // ※2
    ld      C, (HL)
    inc     L                       // ※3
    ld      D, (HL)
    inc     L                       // ※4
    ld      E, (HL)

    pop     HL                      // drawAddr

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
    add     A, #(VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) - 2)
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
