/**
 * 入力
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "input.h"
#include "vram.h"//TEST
#include "print.h"//TEEST

// ---------------------------------------------------------------- 変数
static u8   sInput;                 // 入力生データ
static u8   sInputOld;              // 入力生データ
static u8   sInputTrig;             // 入力 OFF->ON エッジ データ

// ---------------------------------------------------------------- システム
void inputInit() __z88dk_fastcall
{
    sInput     = 0;
    sInputOld  = 0;
    sInputTrig = 0;
}

static const u8 sInputTab[] = {
    // strobe, count, mask, input, mask, input, ...
    0xf1,      1, /* Z */0x40, INPUT_MASK_A,
    0xf2,      5, /* S */0x20, INPUT_MASK_D, /* U */0x08, INPUT_MASK_U, /* V */0x04, INPUT_MASK_A, /* W */0x02, INPUT_MASK_U, /* X */0x01, INPUT_MASK_B,
    0xf3,      3, /* J */0x40, INPUT_MASK_B, /* K */0x20, INPUT_MASK_R, /* M */0x08, INPUT_MASK_D,
    0xf4,      4, /* A */0x80, INPUT_MASK_L, /* B */0x40, INPUT_MASK_B, /* D */0x10, INPUT_MASK_R, /* H */0x01, INPUT_MASK_L,
    0xf6,      1, /* Space */0x10, INPUT_MASK_A,
    0xf7,      4, /* ↑ */0x20, INPUT_MASK_U, /* ↓ */0x10, INPUT_MASK_D, /* → */0x08, INPUT_MASK_R, /* ← */0x04, INPUT_MASK_L,
    0xf9,      2, /* F1 */0x80, INPUT_MASK_P, /* F2 */0x40, INPUT_MASK_S,
    0,
};

void inputMain() __z88dk_fastcall __naked
{
__asm
    ld      A, (#_sInput)
    ld      (#_sInputOld), A

    BANK_VRAM_IO                // バンク切替

    ld      HL, #_sInputTab
    ld      DE, #MIO_8255_PORTA
    ld      C,  0x00            // sInput

    // -------- write strobe
STROBE_LOOP:
    ld      A, (HL)
    or      A
    jr      z, STROBE_LOOP_END
    inc     HL
    ld      (DE), A             // strobe
    inc     E                   // DE = MIO_8255_PORTB
    ld      A, (DE)             // key data
    ld      E, A

    ld      B, (HL)             // count
    inc     HL

    // -------- read key
KEY_LOOP:
    ld      A, (HL)             // 該当ビットが 0 になる
    inc     HL
    and     A, E
    jp      nz, KEY_LOOP_END
    ld      A, (HL)
    or      A, C                // sInput
    ld      C, A                // sInput
KEY_LOOP_END:
    inc     HL
    djnz    B, KEY_LOOP
    ld      E, #(MIO_8255_PORTA & 0xff)
    jp      STROBE_LOOP

STROBE_LOOP_END:
    ld      A, C                // sInput
    ld      (#_sInput), A
    ld      E, A                // sInput 保存

    // -------- ジョイスティックの読み取り(1) JA2 立下がり検出
    ld      HL, #MIO_ETC
    ld      BC, 0x0e00 | MIO_ETC_JA2_MASK

    // H になるまで待つ. 90 cycles 以上待って L のままならば無視
    ld      A, C                //           C = MIO_ETC_JA2_MASK
    and     A, (HL)             // 7      7  if (MIO_ETC & JA2 == 0) { loop }
    jp      NZ, JLOOP_H_END     // 10/10  17
    ld      A, C                // 4      21
    and     A, (HL)             // 7      28
    jp      NZ, JLOOP_H_END     // 10/10  38
    ld      A, C                // 4      42
    and     A, (HL)             // 7      49
    jp      NZ, JLOOP_H_END     // 10/10  59
    ld      A, C                // 4      63
    and     A, (HL)             // 7      70
    jp      NZ, JLOOP_H_END     // 10/10  80
    ld      A, C                // 4      84
    and     A, (HL)             // 7      91
    jr      Z, JEND
JLOOP_H_END:

    // L になるまで待つ. 98 cycles 以上待って H のままならば無視
    and     A, (HL)             // 7      7  if (MIO_ETC & JA2 != 0) { loop }
    jr      Z, JLOOP_L_END      // 12/7   14
    and     A, (HL)             // 7      21
    jr      Z, JLOOP_L_END      // 12/7   28
    and     A, (HL)             // 7      35
    jr      Z, JLOOP_L_END      // 12/7   42
    and     A, (HL)             // 7      49
    jr      Z, JLOOP_L_END      // 12/7   56
    and     A, (HL)             // 7      63
    jr      Z, JLOOP_L_END      // 12/7   70
    and     A, (HL)             // 7      77
    jr      Z, JLOOP_L_END      // 12/7   84
    and     A, (HL)             // 7      91
    jr      Z, JLOOP_L_END      // 12/7   98
    and     A, (HL)             // 7      105
    jr      NZ, JEND            // 12/7   112

JLOOP_L_END:
    // この時点で, JA2 の立ち下がりから最短 7(jr), 最悪 7+7+12(jr+ld+jr) = 26 cycles 遅れてます

    // -------- ジョイスティックの読み取り(2) JA1(buttonA), JA2(0) 1つ読み取り
    ld      A, (HL)             // 7    ****_*0A*
    //ld A, #0x02 // TEST
    rrca                        // 4    ****_**0A
    rrca                        // 4    ****_***0   c = buttonA
    rl      B                   // 8    0001_110A

    ld      A, #MIO_ETC_JA_MASK // 7    0000_0110
    //                          計 7+4+4+8+7 = 30 cycles

    // -------- ジョイスティックの読み取り(2) JA1(buttonB), JA2(Y1) 1つ読み取り
    and     A, (HL)             // 7    0000_0YB0   Y = Y1
    //ld A, #0x06 // TEST
    add     A, B                // 4    00ww_wwBA   wwww = (Y1 == 0) ? 0111 : 1000
    and     A, #0x33            // 7    00yy_00BA   yy = Y1 + 1
    rrca                        // 4    A00y_y00B
    rrca                        // 4    BA00_yy00
    ld      B, A                // 4    BA00_yy00
    //                          計 7+4+7+4+4+4 = 30 cycles

    // -------- ジョイスティックの読み取り(3) JA1(R), JA2(Y0) 1つ読み取り
    ld      A, (HL)             // 7    ****_*YR*    Y = Y0
    //ld A, #0x06 // TEST
    and     A, #MIO_ETC_JA_MASK // 7    0000_0YR0
    add     A, B                // 4    BA00_yyR0    yy == Y1 + Y0 + 1 == UD
    rlca                        // 4    A00U_DR0B
    rlca                        // 4    00UD_R0BA
    ld      B, A                // 4    00UD_R0BA
    //                          計 7+7+4+4+4+4 = 30 cycles

    // -------- ジョイスティックの読み取り(4) JA1(L), JA2(1) 1つ読み取り
    ld      A, (HL)             //      ****_*1L*
    //ld A, #0x06 // TEST
    rlca                        //      ****_1L**
    and     A, C                //      0000_0L00
    or      A, B                //      00UD_RLBA(負論理)

    // -------- ジョイスティックの読み取り(5) Select/Start の場合
    or      A, 0xc0             //      11UD_RLBA
    ld      C, A
    and     A, 0x0c             //      0000_RL00
    ld      A, C                //      11UD_RLBA
    jp      nz, JEND2           // 左右同時押し無し
                                //  Play       Select     Play + Select
                                //  1111_0011  1110_0011  1101_0011
    and     A, 0x30             //  0011_0000  0010_0000  0001_0000
    dec     A                   //  0010_1111  0001_1111  0000_1111
    add     A, A                //  0101_1110  0011_1110  0001_1110
    add     A, A                //  1011_1100  0111_1100  0011_1100
    or      A, B                //  1011_11BA  0111_11BA  0011_11BA(負論理)
JEND2:
    cpl     A                   //      SPUD_RLBA(静論理)

    // -------- ジョイスティック入力があったなら合成
    or      A, E
    ld      (#_sInput), A
    ld      E, A

JEND:
    BANK_RAM                    // バンク切替

    // -------- sInputTrig = sInput & ~sInputOld;
    ld      A, (#_sInputOld)
    cpl     A                   // ビット反転
    and     A, E                // sInput

    ld      (#_sInputTrig), A

    ret
__endasm;
}

// ---------------------------------------------------------------- 入力
u8 inputGet() __z88dk_fastcall
{
    return sInput;
}
u8 inputGetTrigger() __z88dk_fastcall
{
    return sInputTrig;
}
