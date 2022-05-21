/**
 * 数学関数
 *
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "addr.h"
#include "math.h"
//#include <string.h> // memcpy() 使わない

//#include "vram.h"//TEST
//#include "print.h"//TEST

// ---------------------------------------------------------------- 三角関数テーブル
static const u8 ATAN2_TAB_[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x20, 0x12, 0x0d, 0x09, 0x08, 0x06, 0x05, 0x05, 0x04, 0x04, 0x03, 0x03, 0x03, 0x02, 0x02,
    0x40, 0x2d, 0x20, 0x17, 0x12, 0x0f, 0x0d, 0x0b, 0x09, 0x08, 0x08, 0x07, 0x06, 0x06, 0x05, 0x05,
    0x40, 0x32, 0x28, 0x20, 0x1a, 0x16, 0x12, 0x10, 0x0e, 0x0d, 0x0b, 0x0a, 0x09, 0x09, 0x08, 0x08,
    0x40, 0x36, 0x2d, 0x25, 0x20, 0x1b, 0x17, 0x15, 0x12, 0x11, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a,
    0x40, 0x37, 0x30, 0x29, 0x24, 0x20, 0x1c, 0x19, 0x16, 0x14, 0x12, 0x11, 0x10, 0x0e, 0x0d, 0x0d,
    0x40, 0x39, 0x32, 0x2d, 0x28, 0x23, 0x20, 0x1c, 0x1a, 0x17, 0x16, 0x14, 0x12, 0x11, 0x10, 0x0f,
    0x40, 0x3a, 0x34, 0x2f, 0x2a, 0x26, 0x23, 0x20, 0x1d, 0x1a, 0x18, 0x17, 0x15, 0x14, 0x12, 0x11,
    0x40, 0x3a, 0x36, 0x31, 0x2d, 0x29, 0x25, 0x22, 0x20, 0x1d, 0x1b, 0x19, 0x17, 0x16, 0x15, 0x13,
    0x40, 0x3b, 0x37, 0x32, 0x2e, 0x2b, 0x28, 0x25, 0x22, 0x20, 0x1d, 0x1b, 0x1a, 0x18, 0x17, 0x16,
    0x40, 0x3b, 0x37, 0x34, 0x30, 0x2d, 0x29, 0x27, 0x24, 0x22, 0x20, 0x1e, 0x1c, 0x1a, 0x19, 0x17,
    0x40, 0x3c, 0x38, 0x35, 0x31, 0x2e, 0x2b, 0x28, 0x26, 0x24, 0x21, 0x20, 0x1e, 0x1c, 0x1b, 0x19,
    0x40, 0x3c, 0x39, 0x36, 0x32, 0x2f, 0x2d, 0x2a, 0x28, 0x25, 0x23, 0x21, 0x20, 0x1e, 0x1c, 0x1b,
    0x40, 0x3c, 0x39, 0x36, 0x33, 0x31, 0x2e, 0x2b, 0x29, 0x27, 0x25, 0x23, 0x21, 0x20, 0x1e, 0x1d,
    0x40, 0x3d, 0x3a, 0x37, 0x34, 0x32, 0x2f, 0x2d, 0x2a, 0x28, 0x26, 0x24, 0x23, 0x21, 0x20, 0x1e,
    0x40, 0x3d, 0x3a, 0x37, 0x35, 0x32, 0x30, 0x2e, 0x2c, 0x29, 0x28, 0x26, 0x24, 0x22, 0x21, 0x20,
};
static const s8 SIN_TAB_[] = {
    0x00, 0x02, 0x03, 0x05, 0x06, 0x08, 0x09, 0x0b, 0x0c, 0x0e, 0x10, 0x11, 0x13, 0x14, 0x16, 0x17,
    0x18, 0x1a, 0x1b, 0x1d, 0x1e, 0x20, 0x21, 0x22, 0x24, 0x25, 0x26, 0x27, 0x29, 0x2a, 0x2b, 0x2c,
    0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x38, 0x39, 0x3a, 0x3b,
    0x3b, 0x3c, 0x3c, 0x3d, 0x3d, 0x3e, 0x3e, 0x3e, 0x3f, 0x3f, 0x3f, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40,
};

// ---------------------------------------------------------------- システム
void mathInit() __z88dk_fastcall
{
    // 除算テーブルの初期化

    for (u16 i = 0; i <= 255; i++) {
        ((u8*)ADDR_DIV256_SIGN_TAB)[i] = (i *  3) / (u16)256 - 1;
        ((u8*)ADDR_DIV256_7_TAB   )[i] = (i *  7) / (u16)256;
        ((u8*)ADDR_DIV256_25_TAB  )[i] = (i * 25) / (u16)256;
        ((u8*)ADDR_DIV256_40_TAB  )[i] = (i * 40) / (u16)256;
    }

    // 三角関数テーブルの初期化
    {
        const u8* p = SIN_TAB_;
        u8*       q = (u8*)ADDR_SIN_TAB;
        u8*       r = (u8*)ADDR_COS_TAB;
        for (u8 i = 0; i < 64; i++) {
            q[i      ] = r[i + 192] =  p[     i];
            q[i +  64] = r[i      ] =  p[64 - i];
            q[i + 128] = r[i +  64] = -p[     i];
            q[i + 192] = r[i + 128] = -p[64 - i];
        }
    }

    //memcpy((u8*)ADDR_ATAN2_TAB, ATAN2_TAB_, 256);
__asm
    ld  HL, #_ATAN2_TAB_
    ld  DE, #(ADDR_ATAN2_TAB)
    ld  BC, #0x0100
    ldir
__endasm;
}

// ---------------------------------------------------------------- 最大最小
// ---------------------------------------------------------------- 飽和演算
#pragma disable_warning 85  // 引数が使われてない
#pragma disable_warning 59  // 戻り値が必要
#pragma save
u16 addSaturateU16(const u16 a, const u16 b) __naked
{
__asm
    pop     HL                  // リターン アドレス(捨てる)
    pop     DE                  // a
    pop     BC                  // b
    ld      HL, -6
    add     HL, SP
    ld      SP, HL

    ex      DE, HL              // a
    add     HL, BC              // a + b
    ret     nc
    ld      HL, 0xffff
    ret
__endasm;
}
u8 addSaturateU8(const u16 ab) __z88dk_fastcall __naked
{
__asm
    ld      A, H
    add     A, L
    ld      L, A
    ret     nc
    ld      L, 0xff
    ret
__endasm;
}
#pragma restore


// ---------------------------------------------------------------- 三角関数
#pragma disable_warning 85  // 引数が使われてない
#pragma disable_warning 59  // 戻り値が必要
#pragma save
u8 atan2(const u16 xy) __z88dk_fastcall
{
#if 0   // C版
    s8 x = xy >> 8;
    s8 y = (s8)xy;

    bool sign_x = false;
    bool sign_y = false;

    if (x < 0) {
         x = -x;
         sign_x = true;
    }

    if (y < 0) {
        y = -y;
        sign_y = true;
    }

    while (16 <= x) {
        x >>= 1;
        y >>= 1;
    }

    while (16 <= y) {
        x >>= 1;
        y >>= 1;
    }

    u16 i = ((u16)y & 0x0f) * 16 + ((u16)x & 0x0f);
    u8  a = ((u8*)ADDR_ATAN2_TAB)[i];
    if (sign_x) {
        a = 128 - a;
    }
    if (sign_y) {
        a = -a;
    }
    return a;
#else   // ASM 版
__asm
    // A, BC, HL 破壊
    ld      BC, 0x0000      // sign_x, sign_y
    ld      A, H            // x
    and     A
    jp      P, ATAN2_XP
    neg                     // x = -x
    inc     B               // sign_x = 1
ATAN2_XP:
    cp      A, 15
    jp      c, ATAN2_XP_END // A <= 15 ならばループ終了
    sra     A               // x >>= 1
    sra     L               // y >>= 1
    jp      ATAN2_XP

ATAN2_XP_END:
    ld      H, A

    ld      A, L            // y
    and     A
    jp      P, ATAN2_YP
    neg
    ld      L, A            // y = -y
    inc     C               // sign_y =1
ATAN2_YP:
    cp      A, 15
    jp      c, ATAN2_YP_END // A <= 15 ならばループ終了
    sra     A               // y >>= 1
    sra     H               // x >>= 1
    jp      ATAN2_YP

ATAN2_YP_END:
    add     A, A            // y *= 16
    add     A, A
    add     A, A
    add     A, A
    add     A, H            // y += x

    ld      L, A
    ld      H, #(ADDR_ATAN2_TAB >> 8)
    ld      A, (HL)         // A = ADDR_ATAN2_TAB[x + y * 16];

    dec     B               // sign_x
    jr      nz, ATAN2_Y
    neg     A
    ld      B, 0x80
    add     A, B            // A = 128 - A

ATAN2_Y:
    ld      L, A            // return value
    dec     C               // sign_y
    ret     nz
    neg     A               // A = -A
    ld      L, A            // return value
__endasm;
#endif
}
#pragma restore

#pragma disable_warning 85
#pragma save
s8 sin(const u8 x) __z88dk_fastcall __naked
{
__asm
    ld      H, #(ADDR_SIN_TAB >> 8)
    ld      L, (HL)
    ret
__endasm;
}
#pragma restore

#pragma disable_warning 85
#pragma save
s8 cos(const u8 x) __z88dk_fastcall __naked
{
__asm
    ld      H, #(ADDR_COS_TAB >> 8)
    ld      L, (HL)
    ret
__endasm;
}
#pragma restore


// ---------------------------------------------------------------- 乱数
u8 rand7r() __z88dk_fastcall __naked
{
    __asm
    ld      A, R
    ld      L, A
    ret
    __endasm;
}

static const u8 M_SEQUENCE_WORK[] = {    // 内容は変化するが, data セクションで OK
    0x8f, 0xe6, 0xc8, 0x29, 0x2a, 0xf2, 0x4e, 0x61,
    0x15, 0x1c, 0xdf, 0x1b, 0x88, 0xfd, 0xe4, 0x72,
    0xc5,
};
static u8* m_sequence1_ = (u8*)(&M_SEQUENCE_WORK[13]);
static u8* m_sequence2_ = (u8*)(&M_SEQUENCE_WORK[0]);

u8 rand8() __z88dk_fastcall __naked
{
    // M 系列乱数 + GFSR法
    //   ------ シフト レジスタ(m bit) ----->
    //   ┌───┐ ┌───┐ ┌───┐  ┌───┐   ┌───┐
    //  ┌┤ 0 ├─┤ 1 ├─┤ 2 ├..┤ta ├┬..┤m-1├┐
    //  │└───┘ └───┘ └───┘  └───┘│  └───┘│
    //  │                       ┌┴───────┴┐
    //  │                       │   XOR   │
    //  │                       └───┬─────┘
    //  └───────────────────────────┤
    //                            出力
    //
    // ビット幅 タップ位置   周期
    // -------------------------
    //    15      13      32767
    //    17      13     131071
    //    18      10     262143
    //    20      16    1048575
    //    21      18    2097151
    __asm
    ld      E, #(_M_SEQUENCE_WORK + 17) & 0xff // テーブルは 127 bytes しかないので, アドレスの比較は, 下 8bit だけで OK
    // -------- *m_sequence2_++
    ld      HL, (#_m_sequence2_)
    ld      BC, HL
    inc     HL
    ld      A, L
    cp      E
    jp      nz, RAND8_2
    ld      HL, #_M_SEQUENCE_WORK
RAND8_2:
    ld      (#_m_sequence2_), HL
    ld      D, (HL)                 // D に保存

    // -------- *m_sequence1_++
    ld      HL, (#_m_sequence1_)
    inc     HL
    ld      A, L
    cp      E
    jp      nz, RAND8_1
    ld      HL, #_M_SEQUENCE_WORK
RAND8_1:
    ld      (#_m_sequence1_), HL
    ld      A, (HL)

    // -------- xor and out
    xor     A, D                    // D から復帰
    ld      (BC), A
    ld      L, A
    ret
    __endasm;
}

s8 rand8_sign() __z88dk_fastcall __naked
{
    __asm
    call    _rand8  // L = rand8
    ld      H, #(ADDR_DIV256_SIGN_TAB >> 8)
    ld      L, (HL);
    ret
    __endasm;
}
u8 rand8_7() __z88dk_fastcall __naked
{
    __asm
    call    _rand8  // L = rand8
    ld      H, #(ADDR_DIV256_7_TAB >> 8)
    ld      L, (HL);
    ret
    __endasm;
}
u8 rand8_40() __z88dk_fastcall __naked
{
    __asm
    call    _rand8  // L = rand8
    ld      H, #(ADDR_DIV256_40_TAB >> 8)
    ld      L, (HL);
    ret
    __endasm;
}

u8 rand8_25() __z88dk_fastcall __naked
{
    __asm
    call    _rand8  // L = rand8
    ld      H, #(ADDR_DIV256_25_TAB >> 8)
    ld      L, (HL);
    ret
    __endasm;
}