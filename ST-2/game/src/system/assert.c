/**
 * assert
 *
 * @author Snail Barbarian Macho (NWK)
 */

#include "assert.h"
#include "../../../../src-common/hard.h"
#include "../../../../src-common/display-code.h"

// ---------------------------------------------------------------- assert
#if DEBUG
#pragma disable_warning 85
#pragma save
void assert(u16 val) __z88dk_fastcall __naked
{
__asm
    BANKH_VRAM_MMIO C
    ld      SP, 0 + VRAM_TEXT + 40

    // -------- TEXT
    push    HL
    ld      HL, 0 + ASSERT_STR
    ld      DE, 0 + VRAM_TEXT
    ld      BC, 7
    ldir
    pop     HL

    // -------- val 表示
    ld      A, H
    call    ASSERT_DISP_NIBBLE_H
    ld      A, H
    call    ASSERT_DISP_NIBBLE_L
    ld      A, L
    call    ASSERT_DISP_NIBBLE_H
    ld      A, L
    call    ASSERT_DISP_NIBBLE_L

    // -------- ATB
    ld      HL, 0 + VRAM_ATB
    ld      B,  7 + 4
ASSERT_ATB_LOOP:
    ld      (HL), 0x02
    inc     L
    djnz    B, ASSERT_ATB_LOOP

    halt

    // -------- 16進数 1 桁表示. A=値, DE=表示先. BC,HL保存
ASSERT_DISP_NIBBLE_H:
    rrca
    rrca
    rrca
    rrca                // fall through
ASSERT_DISP_NIBBLE_L:
    and     A,  0x0f
    cp      A,  10
    jr      c,  ASSERT_DN_100
    add     A,  0 + DC_A - 10 - DC_0
ASSERT_DN_100:
    add     A,  0 + DC_0
    ld      (DE), A
    inc     E
    ret

ASSERT_STR:
    db      DC_A, DC_S, DC_S, DC_E, DC_R, DC_T, DC_COLON
__endasm;
}
#pragma restore
#endif // DEBUG
