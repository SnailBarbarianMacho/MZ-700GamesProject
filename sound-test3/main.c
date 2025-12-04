/**
 * 三重和音サウンド テスト
 *
 * @author Snail Barbarian Macho (NWK)
 */
#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "sound.h"

#pragma disable_warning 85
#pragma save
static void puts(u8 const * const str) __naked
{
__asm
    ld      DE, HL
    jp      0x0015          // CALL MSG
__endasm;
}
#pragma restore


static void bankRomVramMmio(void) __naked
{
__asm
    BANKL_ROM_BANKH_VRAM_MMIO  C
    jp      0x0006          // CALL LETNL
__endasm;
}


#if 1
#include "music/chronos.h"
#else
static u8 const sd6_chronos_0[] = {
#if 0   // 無音
    SD6_L_REST(0x100), SD6_B_REST(0x100), SD6_C2_REST(0x100), SD6_D_REST(0x100),
    SD6_L_REST(0x100), SD6_B_REST(0x100), SD6_C2_REST(0x100), SD6_D_REST(0x100),
#endif
#if 0   // Lead 音量
    // Lead ◢■◣ デフォルト速度
    SD6_L_UP(0x80, SD6_B1), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_FL(0x80, SD6_B1), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_DW(0x80, SD6_B1), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_UP(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_FL(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_DW(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_UP(0x80, SD6_C6), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_FL(0x80, SD6_C6), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_DW(0x80, SD6_C6), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
#if 0
    // Lead ◢■◣ 速度1/16
    SD6_L_ENV_SPEED_R16,
    SD6_L_UP(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_FL(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_DW(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),

    // Lead ◢■◣ 速度1/2
    SD6_L_ENV_SPEED_R2,
    SD6_L_UP(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_FL(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_DW(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),

    // Lead エコー感の確認
    SD6_L_FL(0x20, SD6_D3), SD6_B_REST(0xe0), SD6_C2_REST(0xe0), SD6_D_REST(0xe0),
    SD6_L_FL(0x20, SD6_E3),
    SD6_L_FL(0x20, SD6_F3),
    SD6_L_FL(0x20, SD6_G3),
    SD6_L_FL(0x20, SD6_A3),
    SD6_L_FL(0x20, SD6_B3),
    SD6_L_FL(0x20, SD6_C4),

    // Lead 高速再生
    SD6_L_FL(0x04, SD6_C3), SD6_B_REST(0x60), SD6_C2_REST(0x60), SD6_D_REST(0x60),
    SD6_L_FL(0x04, SD6_D3),
    SD6_L_FL(0x04, SD6_E3),
    SD6_L_FL(0x04, SD6_F3),
    SD6_L_FL(0x04, SD6_G3),
    SD6_L_FL(0x04, SD6_A3),
    SD6_L_FL(0x04, SD6_B3),
    SD6_L_FL(0x04, SD6_C4),

    SD6_L_UP(0x04, SD6_C3),
    SD6_L_UP(0x04, SD6_D3),
    SD6_L_UP(0x04, SD6_E3),
    SD6_L_UP(0x04, SD6_F3),
    SD6_L_UP(0x04, SD6_G3),
    SD6_L_UP(0x04, SD6_A3),
    SD6_L_UP(0x04, SD6_B3),
    SD6_L_UP(0x04, SD6_C4),

    SD6_L_DW(0x04, SD6_C3),
    SD6_L_DW(0x04, SD6_D3),
    SD6_L_DW(0x04, SD6_E3),
    SD6_L_DW(0x04, SD6_F3),
    SD6_L_DW(0x04, SD6_G3),
    SD6_L_DW(0x04, SD6_A3),
    SD6_L_DW(0x04, SD6_B3),
    SD6_L_DW(0x04, SD6_C4),
#endif
#if 0   // Lead スライド
    // スライドなし
    SD6_L_FL(0x80, SD6_C4),  SD6_B_REST(0x80),  SD6_C2_REST(0x80),  SD6_D_REST(0x80),

    // スライド速度1/1
    SD6_L_SLIDE_SPEED_R1,
    SD6_L_FL(0x80, SD6_C5), SD6_B_REST(0x100), SD6_C2_REST(0x100), SD6_D_REST(0x100),
    SD6_L_FL(0x80, SD6_C4),

    // スライド速度1/4
    SD6_L_SLIDE_SPEED_R4,
    SD6_L_FL(0xc0, SD6_C5), SD6_B_REST(0xc0), SD6_C2_REST(0xc0), SD6_D_REST(0xc0),
    SD6_L_FL(0xc0, SD6_C4), SD6_B_REST(0xc0), SD6_C2_REST(0xc0), SD6_D_REST(0xc0),

    // スライドなし
    SD6_L_SLIDE_OFF,
    SD6_L_FL(0x80, SD6_C5), SD6_B_REST(0x100), SD6_C2_REST(0x100), SD6_D_REST(0x100),
    SD6_L_FL(0x80, SD6_C4),
#endif
#if 0   // Base
    // Base ◢■◣
    SD6_L_REST(0xc0), SD6_B_UP(0xc0, SD6_C2), SD6_C2_REST(0xc0),               SD6_D_REST(0xc0),
    SD6_L_REST(0x80), SD6_B_FL(0x80, SD6_C2), SD6_C2_REST(0x80),               SD6_D_REST(0x80),
    SD6_L_REST(0xc0), SD6_B_DW(0xc0, SD6_C2), SD6_C2_REST(0xc0),               SD6_D_REST(0xc0),
    SD6_L_REST(0x80), SD6_B_UP(0x80, SD6_C4), SD6_C2_REST(0x80),               SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_FL(0x80, SD6_C4), SD6_C2_REST(0x80),               SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_DW(0x80, SD6_C4), SD6_C2_REST(0x80),               SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_UP(0x80, SD6_C6), SD6_C2_REST(0x80),               SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_FL(0x80, SD6_C6), SD6_C2_REST(0x80),               SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_DW(0x80, SD6_C6), SD6_C2_REST(0x80),               SD6_D_REST(0x80),

    // Base 高速再生
    SD6_L_REST(0x60), SD6_B_FL(0x04, SD6_C3), SD6_C2_REST(0x60), SD6_D_REST(0x60),
                      SD6_B_FL(0x04, SD6_D3),
                      SD6_B_FL(0x04, SD6_E3),
                      SD6_B_FL(0x04, SD6_F3),
                      SD6_B_FL(0x04, SD6_G3),
                      SD6_B_FL(0x04, SD6_A3),
                      SD6_B_FL(0x04, SD6_B3),
                      SD6_B_FL(0x04, SD6_C4),

                      SD6_B_UP(0x04, SD6_C3),
                      SD6_B_UP(0x04, SD6_D3),
                      SD6_B_UP(0x04, SD6_E3),
                      SD6_B_UP(0x04, SD6_F3),
                      SD6_B_UP(0x04, SD6_G3),
                      SD6_B_UP(0x04, SD6_A3),
                      SD6_B_UP(0x04, SD6_B3),
                      SD6_B_UP(0x04, SD6_C4),

                      SD6_B_DW(0x04, SD6_C3),
                      SD6_B_DW(0x04, SD6_D3),
                      SD6_B_DW(0x04, SD6_E3),
                      SD6_B_DW(0x04, SD6_F3),
                      SD6_B_DW(0x04, SD6_G3),
                      SD6_B_DW(0x04, SD6_A3),
                      SD6_B_DW(0x04, SD6_B3),
                      SD6_B_DW(0x04, SD6_C4),
#endif
#if 0   // Chord2
    // Chord2 ◢■◣
    SD6_L_REST(0xc0), SD6_B_REST(0xc0),       SD6_C2_UP(0xc0, SD6_C2, SD6_C2), SD6_D_REST(0xc0),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_FL(0x80, SD6_C2, SD6_C2), SD6_D_REST(0x80),
    SD6_L_REST(0xc0), SD6_B_REST(0xc0),       SD6_C2_DW(0xc0, SD6_C2, SD6_C2), SD6_D_REST(0xc0),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_UP(0x80, SD6_C4, SD6_C4), SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_FL(0x80, SD6_C4, SD6_C4), SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_DW(0x80, SD6_C4, SD6_C4), SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_UP(0x80, SD6_C6, SD6_C6), SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_FL(0x80, SD6_C6, SD6_C6), SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_DW(0x80, SD6_C6, SD6_C6), SD6_D_REST(0x80),

    // Chord2 高速再生
    SD6_L_REST(0x60), SD6_B_REST(0x60), SD6_C2_FL(0x04, SD6_C3, SD6_C3), SD6_D_REST(0x60),
                                        SD6_C2_FL(0x04, SD6_D3, SD6_D3),
                                        SD6_C2_FL(0x04, SD6_E3, SD6_E3),
                                        SD6_C2_FL(0x04, SD6_F3, SD6_F3),
                                        SD6_C2_FL(0x04, SD6_G3, SD6_G3),
                                        SD6_C2_FL(0x04, SD6_A3, SD6_A3),
                                        SD6_C2_FL(0x04, SD6_B3, SD6_B3),
                                        SD6_C2_FL(0x04, SD6_C4, SD6_C4),

                                        SD6_C2_UP(0x04, SD6_C3, SD6_C3),
                                        SD6_C2_UP(0x04, SD6_D3, SD6_D3),
                                        SD6_C2_UP(0x04, SD6_E3, SD6_E3),
                                        SD6_C2_UP(0x04, SD6_F3, SD6_F3),
                                        SD6_C2_UP(0x04, SD6_G3, SD6_G3),
                                        SD6_C2_UP(0x04, SD6_A3, SD6_A3),
                                        SD6_C2_UP(0x04, SD6_B3, SD6_B3),
                                        SD6_C2_UP(0x04, SD6_C4, SD6_C4),

                                        SD6_C2_DW(0x04, SD6_C3, SD6_C3),
                                        SD6_C2_DW(0x04, SD6_D3, SD6_D3),
                                        SD6_C2_DW(0x04, SD6_E3, SD6_E3),
                                        SD6_C2_DW(0x04, SD6_F3, SD6_F3),
                                        SD6_C2_DW(0x04, SD6_G3, SD6_G3),
                                        SD6_C2_DW(0x04, SD6_A3, SD6_A3),
                                        SD6_C2_DW(0x04, SD6_B3, SD6_B3),
                                        SD6_C2_DW(0x04, SD6_C3, SD6_C4),
#endif
#endif
#if 0
    // Drum
    SD6_L_REST(0x100), SD6_B_REST(0x100), SD6_C2_REST(0x100),
    SD6_D_1(0x20), SD6_D_2(0x20), SD6_D_3(0x20),
    SD6_D_REST(0x80),
    SD6_D_1(0x04), SD6_D_1(0x04), SD6_D_3(0x04), SD6_D_1(0x04),
    SD6_D_1(0x04), SD6_D_1(0x04), SD6_D_3(0x04), SD6_D_1(0x04),
#endif
#if 0 //とりあえず全部出してみる
    SD6_L_DW(0x40, SD6_C3), SD6_B_REST(0x40),       SD6_C2_REST(0x40),               SD6_D_3(0x40),
    SD6_L_DW(0x40, SD6_C3), SD6_B_REST(0x40),       SD6_C2_REST(0x40),               SD6_D_3(0x40),
    SD6_L_DW(0x40, SD6_C3), SD6_B_DW(0x40, SD6_G3), SD6_C2_REST(0x40),               SD6_D_3(0x40),
    SD6_L_DW(0x40, SD6_C3), SD6_B_DW(0x40, SD6_G3), SD6_C2_DW(0x40, SD6_C4, SD6_C4), SD6_D_3(0x40),
    SD6_L_DW(0x40, SD6_C3), SD6_B_DW(0x40, SD6_G3), SD6_C2_DW(0x40, SD6_C4, SD6_E4), SD6_D_3(0x40),
    SD6_L_DW(0x40, SD6_C3), SD6_B_DW(0x40, SD6_G3), SD6_C2_DW(0x40, SD6_C4, SD6_E4), SD6_D_3(0x40),
    SD6_L_DW(0x40, SD6_C3), SD6_B_DW(0x40, SD6_G3), SD6_C2_DW(0x40, SD6_C4, SD6_E4), SD6_D_3(0x40),
    SD6_L_DW(0x40, SD6_C3), SD6_B_DW(0x40, SD6_G3), SD6_C2_DW(0x40, SD6_C4, SD6_E4), SD6_D_3(0x40),
    SD6_L_DW(0x40, SD6_C3), SD6_B_DW(0x40, SD6_G3), SD6_C2_DW(0x40, SD6_C4, SD6_E4), SD6_D_3(0x40),
    SD6_L_DW(0x40, SD6_C3), SD6_B_DW(0x40, SD6_G3), SD6_C2_DW(0x40, SD6_C4, SD6_E4), SD6_D_3(0x40),
#endif
#if 1 // リピート
    // 2回リピート
    SD6_L_REP_2,
        SD6_L_FL(0x40, SD6_C3), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
        SD6_L_FL(0x40, SD6_D3),
    SD6_L_ENDR,

    SD6_L_REST(0x20),       SD6_B_REST(0x20), SD6_C2_REST(0x20), SD6_D_REST(0x20),

    // 8回リピート
    SD6_L_REP_8,
        SD6_L_FL(0x10, SD6_C3), SD6_B_REST(0x20), SD6_C2_REST(0x20), SD6_D_REST(0x20),
        SD6_L_FL(0x10, SD6_D3),
    SD6_L_ENDR,

    SD6_L_REST(0x20),       SD6_B_REST(0x20), SD6_C2_REST(0x20), SD6_D_REST(0x20),

    // 2重ネスト
    SD6_L_REP_2,
        SD6_L_REP_3,
            SD6_L_FL(0x40, SD6_E3), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
            SD6_L_FL(0x40, SD6_F3),
        SD6_L_ENDR,
        SD6_L_FL(0x40, SD6_G3), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
        SD6_L_FL(0x40, SD6_A3),
    SD6_L_ENDR,
#endif
    // 休符
    SD6_L_END,
};
#endif


void main(void)
{
    sd6Init();
    u8 c = sd6play(((u32)1 << 16) | (u16 const)sd6_chronos_0);

    bankRomVramMmio();
    if (c & KEY9_F2_MASK) {
        puts("ABORTED WITH F2 KEY.\x0d");
        return;
    }
    if (c & KEY9_F4_MASK) {
        puts("ABORTED WITH F4 KEY.\x0d");
        return;
    }
    puts("END.\x0d");
}   // return するとモニターROMに戻る...筈!
