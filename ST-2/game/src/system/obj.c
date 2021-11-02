/**
 * オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "addr.h"
#include "obj.h"
#include "vram.h"
#include "sound.h"

// ---------------------------------------------------------------- 変数, マクロ
// 未使用オブジェクト. 片方向リンク
Obj* _pObjFreePlayer;
Obj* _pObjFreePlayerBullet;
Obj* _pObjFreeEnemy;
Obj* _pObjFreeEnemyBullet;
Obj* _pObjFreeItem;
Obj* _pObjFreeEtc;
// 使用中のオブジェクト. 双方向リンク
Obj* _pObjInUsePlayer;
Obj* _pObjInUsePlayerBullet;
Obj* _pObjInUseEnemy;
Obj* _pObjInUseEnemyBullet;
Obj* _pObjInUseItem;
Obj* _pObjInUseEtc;

// 各 Obj の数. 多すぎるとエラー
#define NR_OBJS_PLAYER          8
#define NR_OBJS_PLAYER_BULLET   8
#define NR_OBJS_ENEMY           16
#define NR_OBJS_ENEMY_BULLET    32
#define NR_OBJS_ITEM            32
#define NR_OBJS_ETC             32
#define OBJ_ADDR_PLAYER         ADDR_OBJ
#define OBJ_ADDR_PLAYER_BULLET  (OBJ_ADDR_PLAYER        + NR_OBJS_PLAYER        * sizeof(Obj))
#define OBJ_ADDR_ENEMY          (OBJ_ADDR_PLAYER_BULLET + NR_OBJS_PLAYER_BULLET * sizeof(Obj))
#define OBJ_ADDR_ENEMY_BULLET   (OBJ_ADDR_ENEMY         + NR_OBJS_ENEMY         * sizeof(Obj))
#define OBJ_ADDR_ITEM           (OBJ_ADDR_ENEMY_BULLET  + NR_OBJS_ENEMY_BULLET  * sizeof(Obj))
#define OBJ_ADDR_ETC            (OBJ_ADDR_ITEM          + NR_OBJS_ITEM          * sizeof(Obj))

// ---------------------------------------------------------------- システム(初期化)
static void objInitSub(Obj* pObj, Obj** ppObjFree, Obj** ppObjInUse, u8 nrObjs)
{
    *ppObjFree  = pObj;
    *ppObjInUse = nullptr;

    for (u8 i = nrObjs; 1 < i; --i) {
        pObj->pNext  = pObj + 1;
        pObj++;
    }
    pObj->pNext = nullptr;
}

void objInit() __z88dk_fastcall
{
    STATIC_ASSERT(sizeof(Obj) == 32, InvalidObjSize);   // Obj 構造体が, 0x100 境界を跨いではいけません
    STATIC_ASSERT(NR_OBJS_PLAYER + NR_OBJS_PLAYER_BULLET + NR_OBJS_ENEMY + NR_OBJS_ENEMY_BULLET + NR_OBJS_ITEM + NR_OBJS_ETC == 128,
        InvalidNrObjs); // Obj 総数は 128 個でなければなりません

#if DEBUG// ゴミでうめてみる
    u8* addr = (u8*)ADDR_OBJ;
    for (u16 i = 0; i < 0x1000; i++) {
        *addr++ = 0xff;
    }
#endif

    objInitSub((Obj*)OBJ_ADDR_PLAYER,        &_pObjFreePlayer,       &_pObjInUsePlayer,       NR_OBJS_PLAYER);
    objInitSub((Obj*)OBJ_ADDR_PLAYER_BULLET, &_pObjFreePlayerBullet, &_pObjInUsePlayerBullet, NR_OBJS_PLAYER_BULLET);
    objInitSub((Obj*)OBJ_ADDR_ENEMY,         &_pObjFreeEnemy,        &_pObjInUseEnemy,        NR_OBJS_ENEMY);
    objInitEnemyBullet();
    objInitItem();
    objInitSub((Obj*)OBJ_ADDR_ETC,           &_pObjFreeEtc,          &_pObjInUseEtc,          NR_OBJS_ETC);
}

void objInitEnemyBullet()
{
    objInitSub((Obj*)OBJ_ADDR_ENEMY_BULLET,  &_pObjFreeEnemyBullet,  &_pObjInUseEnemyBullet,  NR_OBJS_ENEMY_BULLET);
}
void objInitItem()
{
    objInitSub((Obj*)OBJ_ADDR_ITEM,          &_pObjFreeItem,         &_pObjInUseItem,         NR_OBJS_ITEM);
}

// ---------------------------------------------------------------- システム(メイン)
static inline void objMainSub(Obj** ppObjInUse, Obj** ppObjFree)
{
    Obj* pObj = *ppObjInUse;
    while (pObj) {
        bool ret = pObj->mainFunc(pObj);
        Obj* const pObjNext = pObj->pNext;
        pObj->bHit = false;
        // Obj をプールに格納します
        if (!ret) {
            /* objInUse から切り離し */
            Obj* const pObjPrev    = pObj->pPrev;
            if (pObjNext) {/* 末端部を除く */
                pObjNext->pPrev = pObjPrev;
            }
            if (pObjPrev) {/* 根元部を除く */
                pObjPrev->pNext = pObjNext;
            } else {/* 最元部 */
                *ppObjInUse = pObjNext;
            }
            /* objFree に挿入 */
            pObj->pNext = *ppObjFree;
            *ppObjFree  = pObj;
        }
        pObj = pObjNext;
    }
}

// ---------------------------------------------------------------- システム(描画)
extern u8* funcPtr; // crt0

// 表示アドレスを計算し,
// draw() を呼び出し,
// 座標を移動します
#pragma disable_warning 85          // pObj 未使用
#pragma save
static void moveDrawSub(Obj* const pObj) __z88dk_fastcall __naked
{
    STATIC_ASSERT(3 == OBJ_OFFSET_GEO8_XH,                        Asm1); // ※1 を修正
    STATIC_ASSERT(3 <  OBJ_OFFSET_GEO8_YH   - OBJ_OFFSET_GEO8_XH, Asm2); // ※2 を修正
    STATIC_ASSERT(3 <  OBJ_OFFSET_DRAW_FUNC - OBJ_OFFSET_GEO8_YH, Asm3); // ※3 を修正
    STATIC_ASSERT(3 == OBJ_OFFSET_GEO_SY    - OBJ_OFFSET_GEO_X,   Asm4); // ※4 を修正
__asm
    ld      DE, HL                  // pObj 保存

    // --- draw addr x の計算
    // HL += GEO8_XH
    inc     L                       // ※1
    inc     L                       // ※1
    inc     L                       // ※1
    // C = VRAM_TEXT_ADDR(X, 0)
    ld      A, (HL)                 // pObj->uGeo.geo8.xh
    add     A, #(VVRAM_TEXT_ADDR(0, 0) & 0xff)
    ld      C, A

    // --- draw addr y の計算
    // HL += GEO8_YH - GEO8_XH
    ld      A, L                                            // ※2
    add     A, #(OBJ_OFFSET_GEO8_YH - OBJ_OFFSET_GEO8_XH)   // ※2
    ld      L, A                                            // ※2
    // B = VRAM_TEXT_ADDR(0, Y) >> 8
#if ((VVRAM_TEXT_ADDR(0, 0) >> 8) & 0xff) != 0
    ld      A, (HL)             // pObj->uGeo.geo8.yh
    add     A, #((VVRAM_TEXT_ADDR(0, 0) >> 8) & 0xff)
    ld      B, A
#else // 仮想 VRAM が 0x0000 からある場合
    ld      B, (HL)
#endif

    // ---- pObj->draw(pObj, drawAddr);
    // HL += DRAW_FUNC - GEL8_YH
    ld      A, L                                            // ※3
    add     A, #(OBJ_OFFSET_DRAW_FUNC - OBJ_OFFSET_GEO8_YH) // ※3
    ld      L, A                                            // ※3

    // HL = (HL)
    ld      A, (HL)
    inc     L
    ld      H, (HL)
    ld      L, A                    // HL = draw() のアドレス
    // HL の指すアドレスに呼び出す
    push    BC                      // drawAddr
    push    DE                      // pObj
    call    _funcPtr                // crt0 に, 「jp (HL)」 があります
    pop     HL                      // pObj
    pop     BC                      // drawAddr (捨てる)

    // ---- pObj->uGeo.geo.x += pObj->uGeo.geo.sx;
    ld      (MOVE_DRAW_SUB_SP_RESTORE + 1), SP// 20 SP 保存(自己書換)
    ld      SP, HL                  //  6
    pop     BC                      // 10 BC = pObj->uGeo.geo.sx
    pop     HL                      // 10 HL = pObj->uGeo.geo.x
    add     HL, BC                  // 11
    push    HL                      // 11

    // ---- pObj->uGeo.geo.y += pObj->uGeo.geo.sy;
    inc     SP                      //  6 ※4
    inc     SP                      //  6 ※4
    inc     SP                      //  6 ※4

    pop     BC                      // BC = pObj->uGeo.geo.sy
    pop     HL                      // HL = pObj->uGeo.geo.y
    add     HL, BC
    push    HL
MOVE_DRAW_SUB_SP_RESTORE:
    ld      SP, #0x0000             // SP 復帰
    ret
__endasm;
}
#pragma restore

#if 0 // C 版
static inline void objMoveDraw(Obj* pObjInUse)
{
    Obj* pObj = pObjInUse;
    while (pObj) {
        void (*drawFunc)(Obj* const, u8*) = pObj->drawFunc;
        if (drawFunc) {
            moveDrawSub(pObj);
        }
        pObj = pObj->pNext;
    }
}
#else // ASM 版
static void objMoveDraw(Obj* pObjInUse) __z88dk_fastcall __naked
{
    STATIC_ASSERT(3 <  OBJ_OFFSET_DRAW_FUNC                            , Asm1); // ※1 を修正
    STATIC_ASSERT(3 < (OBJ_OFFSET_P_NEXT - (OBJ_OFFSET_DRAW_FUNC + 1)) , Asm2); // ※2 を修正
__asm
    // ---- while (pObj)
    ld      A, H
    or      A, L
    ret     z
OBJ_MOVE_DRAW_LOOP:
    ld      C, L                                    // HL 保存

    // ---- drawFunc = pObj->drawFunc;
    // HL += DRAW_FUNC
    ld      A, L
    add     A, #OBJ_OFFSET_DRAW_FUNC                // ※1
    ld      L, A

    // DE = (HL)
    ld      E, (HL)
    inc     L
    ld      D, (HL)

    // ---- if (drawFunc) { moveDrawSub(pObj); }
    // if (DE == 0) {goto LOOP_END }
    ld      A, D
    or      A, E
    jr      z, OBJ_MOVE_DRAW_LOOP_END

    // moveDrawSub(HL)
    push    HL
    ld      L, C                                    // HL 復帰
    call    _moveDrawSub
    pop     HL

    // ---- pObj = pObj->pNext;
OBJ_MOVE_DRAW_LOOP_END:
    // HL += P_NEXT - DRAW_FUNC
    ld      A, L
    add     A, #(OBJ_OFFSET_P_NEXT - (OBJ_OFFSET_DRAW_FUNC + 1)) // ※2
    ld      L, A

    // HL = (HL)
    ld      A, (HL)
    inc     L
    ld      H, (HL)
    ld      L, A

    // if (HL != 0) { goto DRAW_LOOP }
    or      A, H
    jp      nz, OBJ_MOVE_DRAW_LOOP
    ret
__endasm;
}
#endif


// ---------------------------------------------------------------- システム(衝突判定)
// 相互に衝突フラグが付きます
#if 0 // C 版
static void objCollision(Obj* const pObjInUse1, Obj* const pObjInUse2)
{
    // x0+--------+x1    x0'+--------+x1'
    //         xx0+---------+xx1
    //
    // xx0 < x1 かつ x0 < xx1 ならば衝突
    // アセンブラ化が望ましい
    for (Obj* pObj1 = pObjInUse1; pObj1; pObj1 = pObj1->pNext) {
        u8 w1 = pObj1->uGeo.geo.w;
        if (!w1) { continue; }
        s8 x0 = pObj1->uGeo.geo8.xh;
        s8 y0 = pObj1->uGeo.geo8.yh;
        s8 x1 = x0 + w1;
        s8 y1 = y0 + pObj1->uGeo.geo.h;
        Obj* pObj2 = pObjInUse2;
        for (Obj* pObj2 = pObjInUse2; pObj2; pObj2 = pObj2->pNext) {
            u8 w2 = pObj2->uGeo.geo.w;
            if (!w2) { continue; }
            s8 c = pObj2->uGeo.geo8.xh;
            if (c >= x1) { continue; }
            c += w2;
            if (x0 >= c) { continue; }
            c = pObj2->uGeo.geo8.yh;
            if (c >= y1) { continue; }
            c += pObj2->uGeo.geo.h;
            if (y0 >= c) { continue; }
            pObj1->bHit = true;
            pObj2->bHit = true;
            pObj2->fitness -= pObj1->offence;// 繰り下がりしてしまうバグあり
            pObj1->fitness -= pObj2->offence;// 繰り下がりしてしまうバグあり
            return;
        } // for (pObj2)
    } // for (pObj1)
#else // ASM 版
static void objCollision(Obj* const pObjInUse1, Obj* const pObjInUse2) __naked
{
    STATIC_ASSERT(3 <  OBJ_OFFSET_GEO8_W,                       Asm1); // ※1 を修正
    STATIC_ASSERT(1 == OBJ_OFFSET_GEO8_W  - OBJ_OFFSET_GEO8_XH, Asm2); // ※2 を修正
    STATIC_ASSERT(3 <  OBJ_OFFSET_GEO8_YH - OBJ_OFFSET_GEO8_XH, Asm3); // ※3 を修正
    STATIC_ASSERT(1 == OBJ_OFFSET_GEO8_H  - OBJ_OFFSET_GEO8_YH, Asm4); // ※4 を修正
    STATIC_ASSERT(3 <  OBJ_OFFSET_B_HIT   - OBJ_OFFSET_GEO8_H,  Asm5); // ※5 を修正
    STATIC_ASSERT(1 == OBJ_OFFSET_FITNESS - OBJ_OFFSET_B_HIT,   Asm6); // ※6 を修正
    STATIC_ASSERT(2 == OBJ_OFFSET_OFFENCE - OBJ_OFFSET_FITNESS, Asm7); // ※7 を修正
    STATIC_ASSERT(3 <  OBJ_OFFSET_P_NEXT,                       Asm8); // ※8 を修正
    STATIC_ASSERT(3 <  OBJ_OFFSET_P_NEXT  - OBJ_OFFSET_GEO8_W,  Asm9); // ※9 を修正
    STATIC_ASSERT(3 <  OBJ_OFFSET_OFFENCE - OBJ_OFFSET_GEO8_W,  Asm10);// ※10 を修正
    STATIC_ASSERT(3 <  OBJ_OFFSET_GEO8_H - OBJ_OFFSET_GEO8_W,   Asm11);// ※11 を修正
__asm
    pop     HL                  // リターン アドレス(捨てる)
    pop     HL                  // pObj1

    // pObjInUse1 が null なら何もしない
    ld      A, H
    or      L
    jp      z, OBJ_COLLISION_RET

    // pObjInUse2 が null でも何もしない
    pop     DE
    push    DE

    ld      A, D
    or      E
    jp      z, OBJ_COLLISION_RET

OBJ_COLLISION_LOOP1:
    // w1 = pObj1->uGeo.geo.w;
    ld      A, L                    // ※1
    add     A, #(OBJ_OFFSET_GEO8_W) // ※1
    ld      L, A                    // ※1
    ld      A, (HL)

    // if (!w1) { continue; }
    and     A
    jp      z, OBJ_COLLISION_LOOP1_END

    // D = x0 = pObj1->uGeo.geo8.xh;
    dec     L                       // ※2
    ld      D, (HL)
    // E = x1 = x0 + w1;
    add     A, D
    ld      E, A

    // B = y0 = pObj1->uGeo.geo8.yh;
    ld      A, L                                            // ※3
    add     A, #(OBJ_OFFSET_GEO8_YH - OBJ_OFFSET_GEO8_XH)   // ※3
    ld      L, A                                            // ※3
    ld      B, (HL)
    // C = y1 = y0 + pObj1->uGeo.geo.h;
    inc     L                       // ※4
    ld      A, (HL)
    add     A, B
    ld      C, A

    // HL の位置を uGeo.geo.w に移動する
    ld      A, L                                        // ※11
    sub     A, #(OBJ_OFFSET_GEO8_H - OBJ_OFFSET_GEO8_W) // ※11
    ld      L, A                                        // ※11

    exx
        pop  HL                     // pObj2
        push HL                     // pObj2

OBJ_COLLISION_LOOP2:
        ld  E, L                    // HL 保存

        // A = w2 = pObj2->uGeo.geo.w;
        ld  A, L                    // ※1
        add A, #(OBJ_OFFSET_GEO8_W) // ※1
        ld  L, A                    // ※1
        ld  A, (HL)
        // if (!w2) { continue; }
        and A
        jp  z, OBJ_COLLISION_LOOP2_END
        // B = w2
        ld  B, A

        // A = c = pObj2->uGeo.geo8.xh;
        dec L                       // ※2
        ld  A, (HL)                 // A = c
        // if (c >= x1) { continue; }
    exx
    cp      A, E                    // E = x1
    exx
        jp  p, OBJ_COLLISION_LOOP2_END

        //  C = c + w2;
        add A, B
        ld  C, A

        // if (x0 >= c) { continue; }
    exx
    ld      A, D                    // D = x0
    exx
        cp  A, C
        jp  p, OBJ_COLLISION_LOOP2_END

        // A = c = pObj2->uGeo.geo8.yh;
        ld  A, L                                            // ※3
        add A, #(OBJ_OFFSET_GEO8_YH - OBJ_OFFSET_GEO8_XH)   // ※3
        ld  L, A                                            // ※3
        ld  A, (HL)

        // if (c >= y1) { continue; }
    exx
    cp      A, C                    // C = y1
    exx
        jp  p, OBJ_COLLISION_LOOP2_END

        // A = c += pObj2->uGeo.geo.h;
        inc L                       // ※4
        add A, (HL)
        ld  C, A

        // if (y0 >= c) { continue; }
    exx
    ld      A, B                    // B = y0
    exx
        cp  A, C
        jp  p, OBJ_COLLISION_LOOP2_END

#if 1 // ヒット処理
        // -------- ヒットした
        // HL' = pObj2->bHit    から加算していく
        // HL  = pObj1->offence から減算していく
        // pObj2->bHit = true;
        ld  A, L                                            // ※5
        add A, #(OBJ_OFFSET_B_HIT - OBJ_OFFSET_GEO8_H)      // ※5
        ld  L, A                                            // ※5
        ld  (HL), H                 // 0 以外の値
        // pObj2->fitness -= pObj1->offence;
    exx
    ld      A, L                                            // ※10
    add     A, #(OBJ_OFFSET_OFFENCE - OBJ_OFFSET_GEO8_W)    // ※10
    ld      L, A                                            // ※10
    ld      A, (HL)                 // A = pObj1->offence
    exx
        ld  C, A                    // C = pObj1->offence

        inc L                       // ※6

        // BA = pObj1->fitness
        ld  A, (HL)
        inc L
        ld  B, (HL)

        // AC = BA - C
        sub A, C
        ld  C, A
        ld  A, B
        sbc A, 0
        jr  nc, OBJ_COLLISION_HIT2
        xor A
        ld  C, A                    // AC = 0x0000
OBJ_COLLISION_HIT2:

        // pObj2->fitness = AC
        dec L
        ld  (HL), C
        inc L
        ld  (HL), A

        // A = pObj2->offence
        inc L                       // ※7
        ld  A, (HL)
    exx
    ld      C, A                    // C = pObj2->offence

    // BA = pObj1->fitness
    dec     L                       // ※7
    ld      B, (HL)
    dec     L
    ld      A, (HL)
    // AC = BA - C
    sub     A, C
    ld      C, A
    ld      A, B
    sbc     A, 0
    jr      nc, OBJ_COLLISION_HIT1
    xor     A
    ld      C, A                    // AC = 0x0000
OBJ_COLLISION_HIT1:
    // pObj1->fitness = AC
    inc     L
    ld      (HL), A
    dec     L
    ld      (HL), C
    // pObj1->bHit = 0 以外の値
    dec     L                       // ※6
    ld      (HL), H

    // スタックポインタを戻して ret
    ld      HL, -4
    add     HL, SP
    ld      SP, HL
    ret
#endif
OBJ_COLLISION_LOOP2_END:
        // pObj = pObj->pNext;
        ld  A, E                    // HL 復活              // ※8
        add A, #(OBJ_OFFSET_P_NEXT)                         // ※8
        ld  L, A                                            // ※8

        ld  A, (HL)
        inc L
        ld  H, (HL)
        ld  L, A

        // if (pObj2) goto OBJ_COLLISION_LOOP2
        or  H
        jp  nz, OBJ_COLLISION_LOOP2
    exx

OBJ_COLLISION_LOOP1_END:
    // pObj = pObj->pNext;
    ld      A, L                                            // ※9
    add     A, #(OBJ_OFFSET_P_NEXT - OBJ_OFFSET_GEO8_W)     // ※9
    ld      L, A                                            // ※9

    ld      A, (HL)
    inc     L
    ld      H, (HL)
    ld      L, A

    // if (pObj1) goto OBJ_COLLISION_LOOP1
    or      H
    jp      nz, OBJ_COLLISION_LOOP1

    // スタックポインタを戻して ret
OBJ_COLLISION_RET:
    ld      HL, -4
    add     HL, SP
    ld      SP, HL
    ret
__endasm;
#endif
}

// アイテムとプレーヤーの衝突判定. アイテムのみフラグが付きます
#define ITEM_W 1
#define ITEM_H 2
#define PLAYER_W 3
#define PLAYER_H 3
#if 0 // C 版
static void objCollisionItem() __z88dk_fastcall
{
    // プレーヤーは最大1個だけなので, ループは1重でいい
    Obj* pObjPlayer = _pObjInUsePlaye;
    if (!pObjPlayer) {
        return;
    }

    s8 px = pObjPlayer->uGeo.geo8.xh;
    s8 py = pObjPlayer->uGeo.geo8.yh;
    for (Obj* pObj = _pObjInUseItem; pObj; pObj = pObj->pNext) {
        s8 c = pObj->uGeo.geo8.xh;
        if (c <  px) { continue; }
        c -= PLAYER_W;
        if (c >= px) { continue; }
        c = pObj->uGeo.geo8.yh;
        if (c <  py) { continue; }
        c -= PLAYER_H;
        if (c >= py) { continue; }
        pObj->bHit = true;
    }
#else // ASM 版
static void objCollisionItem() __z88dk_fastcall __naked
{
    STATIC_ASSERT(3 == OBJ_OFFSET_GEO8_XH,                      Asm1); // ※1 を修正
    STATIC_ASSERT(3 <  OBJ_OFFSET_GEO8_YH - OBJ_OFFSET_GEO8_W,  Asm2); // ※2 を修正
    STATIC_ASSERT(3 <  OBJ_OFFSET_P_NEXT  - OBJ_OFFSET_GEO_SX,  Asm3); // ※3 を修正
    STATIC_ASSERT(3 <  OBJ_OFFSET_STEP    - OBJ_OFFSET_GEO8_YH, Asm4); // ※4 を修正
__asm
    // if (!_pObjInUsePlayer) { return; }
    ld      HL, (__pObjInUsePlayer);
    ld      A, H
    or      L
    ret     z

    // px = pObjPlayer->uGeo.geo8.xh;
    inc     L                       // ※1
    inc     L                       // ※1
    inc     L                       // ※1
    ld      B, (HL)                 // B = px = pObjPlayer->uGeo.geo8.xh

    // py = pObjPlayer->uGeo.geo8.yh;
    ld      A, L                                            // ※2
    add     A, #(OBJ_OFFSET_GEO8_YH - OBJ_OFFSET_GEO8_XH)   // ※2
    ld      L, A                                            // ※2
    ld      C, (HL)                 // C = py = pObjPlayer->uGeo.geo8.yh

    // if (pObjPlayer->step != OBJ_PLAYER_STEP_NORMAL) { return; } 死亡してたら衝突判定なし
    add     A, #(OBJ_OFFSET_STEP - OBJ_OFFSET_GEO8_YH)      // ※4
    ld      L, A
    ld      A, (HL)
    and     A
    ret     nz

    // if (!_pObjInUseItem) { return; }
    ld      HL, (__pObjInUseItem);
    ld      A, H
    or      L
    ret     z

    dec     B                       // B = px = pObjPlayer->uGeo.geo8.xh - 1

OBJ_COLLISION_ITEM_LOOP:
    ld      E, L                    // HL 保存

    // if (x < px) continue;
    inc     L                       // ※1
    inc     L                       // ※1
    inc     L                       // ※1
    ld      A, (HL)                 // x = pObj->uGeo.geo8.xh
    cp      A, B                    // x - px
    jp      m, OBJ_COLLISION_ITEM_LOOP_END

    // if (x - PLAYER_W >= px) continue;
    sub     A, PLAYER_W             // x -= PLAYER_W
    cp      A, B                    // x - px
    jp      p, OBJ_COLLISION_ITEM_LOOP_END

    // if (y < py) continue;
    ld      A, L                                            // ※2
    add     A, #(OBJ_OFFSET_GEO8_YH - OBJ_OFFSET_GEO8_XH)   // ※2
    ld      L, A                                            // ※2
    ld      A, (HL)                 // y = pObj->uGeo.geo8.yh
    cp      A, C                    // y - py
    jp      m, OBJ_COLLISION_ITEM_LOOP_END

    // if (y - PLAYER_W >= py) continue;
    sub     A, PLAYER_H             // y -= PLAYER_H
    cp      A, C                    // y - py
    jp      p, OBJ_COLLISION_ITEM_LOOP_END

    // pObj->bHit = true;
    ld      A, L
    add     A, #(OBJ_OFFSET_B_HIT - OBJ_OFFSET_GEO8_YH)
    ld      L, A
    ld      (HL), H // 0 以外の値

OBJ_COLLISION_ITEM_LOOP_END:
    // pObj = pObj->pNext;
    ld      A, E                    // HL 復活              // ※3
    add     A, #(OBJ_OFFSET_P_NEXT - OBJ_OFFSET_GEO_SX)     // ※3
    ld      L, A                                            // ※3

    ld      A, (HL)
    inc     L
    ld      H, (HL)
    ld      L, A

    // if (pObj) goto OBJ_COLLISION_ITEM_LOOP
    ld      A, H
    or      L
    jp      nz, OBJ_COLLISION_ITEM_LOOP

    ret
__endasm;
#endif
}

// ---------------------------------------------------------------- システム(メイン)
void objMain() __z88dk_fastcall
{
    // メイン
    objMainSub(&_pObjInUsePlayer,       &_pObjFreePlayer);
    objMainSub(&_pObjInUsePlayerBullet, &_pObjFreePlayerBullet);
    objMainSub(&_pObjInUseEnemy,        &_pObjFreeEnemy);
    objMainSub(&_pObjInUseEnemyBullet,  &_pObjFreeEnemyBullet);
    objMainSub(&_pObjInUseItem,         &_pObjFreeItem);
    objMainSub(&_pObjInUseEtc,          &_pObjFreeEtc);

    // 衝突判定
    objCollision(_pObjInUsePlayer,       _pObjInUseEnemy);       // プレーヤーと敵(相互にフラグが立つ)
    objCollision(_pObjInUsePlayer,       _pObjInUseEnemyBullet); // プレーヤーと敵弾(相互にフラグが立つ)
    objCollision(_pObjInUsePlayerBullet, _pObjInUseEnemy);       // プレーヤー弾と敵(相互にフラグが立つ)
    objCollisionItem(); // プレーヤーとアイテム(アイテムのみフラグが立つ)

    // 移動と表示
    objMoveDraw(_pObjInUseItem);
    objMoveDraw(_pObjInUseEnemy);
    objMoveDraw(_pObjInUsePlayerBullet);
    objMoveDraw(_pObjInUsePlayer);
    objMoveDraw(_pObjInUseEnemyBullet);
    objMoveDraw(_pObjInUseEtc);
}


// ---------------------------------------------------------------- 生成
#define OBJ_CREATE(pObjFree, pObjInUse, initFunc, mainFunc, drawFunc, pParent)\
    /* 未使用の Obj を探して切り離し */\
    Obj* pObj = pObjFree;           \
    if (!pObj) {                    \
        return nullptr; /* 全て使用済 */\
    }                               \
    pObjFree = pObj->pNext;         \
                                    \
    {   /* pObjInUse に挿入 */      \
        Obj* pInUse = pObjInUse;   /* 最初の場合は, nullptr */\
        if (pInUse) {               \
            pInUse->pPrev = pObj;   \
        }                           \
        pObj->pNext = pInUse;       \
        pObj->pPrev = nullptr;      \
        pObjInUse = pObj;           \
    }                               \
                                    \
    /* 関数の登録と実行 */          \
    pObj->mainFunc = mainFunc;      \
    pObj->drawFunc = drawFunc;      \
    pObj->bHit     = false;         \
    if (initFunc) {                 \
        initFunc(pObj, pParent);    \
    }                               \
    return pObj;


Obj* objCreatePlayer(
    void (*initFunc)(Obj* const, Obj* const),
    bool (*mainFunc)(Obj* const),
    void (*drawFunc)(Obj* const, u8* drawAddr),
    Obj* const pParent)
{
    OBJ_CREATE(_pObjFreePlayer, _pObjInUsePlayer, initFunc, mainFunc, drawFunc, pParent);
}
Obj* objCreatePlayerBullet(
    void (*initFunc)(Obj* const, Obj* const),
    bool (*mainFunc)(Obj* const),
    void (*drawFunc)(Obj* const, u8* drawAddr),
    Obj* const pParent)
{
    OBJ_CREATE(_pObjFreePlayerBullet, _pObjInUsePlayerBullet, initFunc, mainFunc, drawFunc, pParent);
}
Obj* objCreateEnemy(
    void (*initFunc)(Obj* const, Obj* const),
    bool (*mainFunc)(Obj* const),
    void (*drawFunc)(Obj* const, u8* drawAddr),
    Obj* const pParent)
{
    OBJ_CREATE(_pObjFreeEnemy, _pObjInUseEnemy, initFunc, mainFunc, drawFunc, pParent);
}
Obj* objCreateEnemyBullet(
    void (*initFunc)(Obj* const, Obj* const),
    bool (*mainFunc)(Obj* const),
    void (*drawFunc)(Obj* const, u8* drawAddr),
    Obj* const pParent)
{
    OBJ_CREATE(_pObjFreeEnemyBullet, _pObjInUseEnemyBullet, initFunc, mainFunc, drawFunc, pParent);
}
Obj* objCreateItem(
    void (*initFunc)(Obj* const, Obj* const),
    bool (*mainFunc)(Obj* const),
    void (*drawFunc)(Obj* const, u8* drawAddr),
    Obj* const pParent)
{
    OBJ_CREATE(_pObjFreeItem, _pObjInUseItem, initFunc, mainFunc, drawFunc, pParent);
}
Obj* objCreateEtc(
    void (*initFunc)(Obj* const, Obj* const),
    bool (*mainFunc)(Obj* const),
    void (*drawFunc)(Obj* const, u8* drawAddr),
    Obj* const pParent)
{
    OBJ_CREATE(_pObjFreeEtc, _pObjInUseEtc, initFunc, mainFunc, drawFunc, pParent);
}


// ---------------------------------------------------------------- ユーティリティ
