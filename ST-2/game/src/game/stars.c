/**
 * 背景の星
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/sys.h"
#include "../system/addr.h"
#include "../system/obj.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/math.h"
#include "stars.h"

// ------------------------------- 変数, マクロ, 構造体
bool b_stars_enabled_;
u8   stars_counter_;

#define NR_NEAR_STARS 10   // 最小 1
#define NR_FAR_STARS  50   // 最小 1, 最大 合計 85 - 2 = 83

/*
 * 星 1 個 1 個 Obj で処理するのは重いので,
 * 専用のワークで高速化する
 */
typedef struct s_Star
{
    u8  col;
    u8  x, y;
} Star;

static Star* const stars_far_  = (Star*)ADDR_STARS;
static Star* const stars_near_ = (Star*)(ADDR_STARS + (NR_FAR_STARS + 1) * sizeof(Star));

// ---------------------------------------------------------------- 初期化
void starsInit(void)
{
    b_stars_enabled_ = true;
    stars_counter_  = 0;

    stars_near_[NR_NEAR_STARS].col = 0;
    stars_far_[NR_FAR_STARS].col = 0;
    Star* p;

    p = stars_far_;
    for (u8 i = NR_FAR_STARS; 0 < i; i--, p++)
    {
        p->x   = rand8_40();
        p->y   = rand8_25();
        p->col = rand8_7() * 0x10 + 0x10;
    }

    p = stars_near_;
    for (u8 i = NR_NEAR_STARS; 0 < i; i--, p++)
    {
        p->x   = rand8_40();
        p->y   = rand8_25();
        p->col = (rand8() & 3) * 0x10 + 0x40;
    }
    return;
}


// ---------------------------------------------------------------- メイン
void starsMain(void)
{
//printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 10)); printU8(stars_near_[0].col); printU8(stars_near_[0].x); printU8(stars_near_[0].y); printU8(stars_near_[1].col);
//printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 11)); printU16((u16)stars_far_); printU16((u16)stars_near_);
#if 0
    if (sbStarsTimer & 1) {
        for (Star* p = stars_far_; p->col; p++) {
            p->y++;
            if (VRAM_HEIGHT <= p->y) {
                p->x = rand8() % VRAM_WIDTH;
                p->y = 0;
            }
        }
    }
    for (Star* p = stars_far_; p->col; p++) {
        u8* addrF = (u8*)VVRAM_TEXT_ADDR(p->x, p->y);
        *addrF = DC_1DOT;
        addrF += VRAM_WIDTH + VRAM_GAP;
        *addrF = p->col;
    }
    for (Star* p = stars_near_; p->col; p++) {
        p->y++;
        if (VRAM_HEIGHT <= p->y) {
            p->x = rand8() % VRAM_WIDTH;
            p->y = 0;
        }
        u8* addrN = (u8*)VVRAM_TEXT_ADDR(p->x, p->y);
        *addrN = DC_PERIOD;
        addrN += VRAM_WIDTH + VRAM_GAP;
        *addrN = p->col;
    }
#else
__asm
    // ---------------- 星の描画許可
    ld      a, (_b_stars_enabled_)
    and     a
    ld      a, 1
    ld      (_b_stars_enabled_), a
    ret     z
__endasm;

__asm
    exx
        ld  DE, 0 + VVRAM_TEXT_ADDR(0, 0)       // 表示に使う
        ld  BC, 0 + VRAM_WIDTH + VVRAM_GAPX     // 表示に使う
    exx
__endasm;

    // --------------- 遠景の星々(移動)
__asm
    ld      HL, _stars_counter_                 // stars_counter_++
    ld      A, (HL)
    inc     A
    ld      (HL), A
    and     A, 1
    jp      z, STARS_MAIN_FAR_DRAW
    ld      HL, (_stars_far_)

    // ---- ループ
STARS_MAIN_FAR_MOVE_LOOP:
    inc     L
    ld      E, (HL)         // E = x
    inc     L
    ld      A, (HL)         // y

    // ---- 移動
    inc     A               // y++
    cp      A, VRAM_HEIGHT  // y == VRAM_HEIGHT?
    ld      D, A            // D = y
    jp      nz, STARS_MAIN_FAR_MOVE
    ld      D, 0x00         // y = 0
    // x 位置の修正
    push    de
    push    hl
        call _rand8_40      // L = [0, 40)
        ld   A, L           // A = [0, 40)
    pop     hl
    pop     de
    dec     L
    ld      (HL), A         // x
    ld      E, A            // E = x
    inc     L
STARS_MAIN_FAR_MOVE:
    ld      (HL), D         // D = y

    // ---- 次のループ
    inc     L
    ld      A, (HL)         // col
    or      A
    jp      nz, STARS_MAIN_FAR_MOVE_LOOP
__endasm;

    // --------------- 遠景の星々(表示)
__asm
STARS_MAIN_FAR_DRAW:
    ld      HL, (_stars_far_)

    // ---- ループ
    ld      A, (HL)         // col
STARS_MAIN_FAR_DRAW_LOOP:
    inc     L
    ld      E, (HL)         // E = x
    inc     L
    ld      D, (HL)         // D = y

    // ---- 表示
    push    DE              // yx
    exx
        pop HL              // yx
        add HL, DE          // VRAM
        ld  (HL), DC_1DOT
        add HL, BC          // ATB
        ld  (HL), A         // col
    exx

    // ---- 次のループ
    inc     L
    ld      A, (HL)         // col
    or      A
    jp      nz, STARS_MAIN_FAR_DRAW_LOOP
__endasm;

    // --------------- 近景の星々(移動+表示)
__asm
    ld      HL, (_stars_near_)

    // ---- ループ
    ld      A, (HL)         // col
STARS_MAIN_NEAR_LOOP:
    ld      C, A            // col
    inc     L
    ld      E, (HL)         // E = x
    inc     L
    ld      A, (HL)         // y

    // ---- 移動
    inc     A               // y++
    cp      A, VRAM_HEIGHT  // y == VRAM_HEIGHT?

    ld      D, A            // D = y
    jp      nz, MAIN_NEAR_MOVE
    ld      D, 0x00         // y = 0
    // x 位置の修正
    push    bc
    push    de
    push    hl
        call _rand8_40      // L = [0, 40)
        ld   A, L           // A = [0, 40)
    pop     hl
    pop     de
    pop     bc
    dec     L
    ld      (HL), A         // x
    ld      E, A            // E = x
    inc     L
MAIN_NEAR_MOVE:
    ld      (HL), D         // D = y

    // ---- 表示
    LD      A, C            // col
    push    DE              // yx
    exx
        pop HL              // yx
        add HL, DE          // VRAM
        ld  (HL), DC_PERIOD
        add HL, BC          // ATB
        ld  (HL), A         // col
    exx

    // ---- 次のループ
    inc     L
    ld      A, (HL)         // col
    or      A
    jp      nz, STARS_MAIN_NEAR_LOOP

__endasm;
#endif
    return;
}

// ---------------------------------------------------------------- 制御
