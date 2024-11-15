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
static void puts(const u8* const str) __naked
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
    BANK_ROM_VRAM_MMIO  C
    jp      0x0006          // CALL LETNL
__endasm;
}


#if 1
#include "music/chronos.h"
#else
static const u8 score0[] = {
#if 0   // 無音
    SD4_L_REST(0x100), SD4_B_REST(0x100), SD4_C2_REST(0x100), SD4_D_REST(0x100),
    SD4_L_REST(0x100), SD4_B_REST(0x100), SD4_C2_REST(0x100), SD4_D_REST(0x100),
#endif
#if 0   // Lead 音量
    // Lead ◢■◣ デフォルト速度
    SD4_L_UP(0x80, SD4_B1), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_FL(0x80, SD4_B1), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_DW(0x80, SD4_B1), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_UP(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_FL(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_DW(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_UP(0x80, SD4_C6), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_FL(0x80, SD4_C6), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_DW(0x80, SD4_C6), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),

    // Lead ◢■◣ 速度1/8
    SD4_L_ENV_SPEED_R8,
    SD4_L_UP(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_FL(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_DW(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),

    // Lead ◢■◣ 速度1/2
    SD4_L_ENV_SPEED_R2,
    SD4_L_UP(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_FL(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_DW(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),

    // Lead 高速再生
    SD4_L_FL(0x04, SD4_C3), SD4_B_REST(0x60), SD4_C2_REST(0x60), SD4_D_REST(0x60),
    SD4_L_FL(0x04, SD4_D3),
    SD4_L_FL(0x04, SD4_E3),
    SD4_L_FL(0x04, SD4_F3),
    SD4_L_FL(0x04, SD4_G3),
    SD4_L_FL(0x04, SD4_A3),
    SD4_L_FL(0x04, SD4_B3),
    SD4_L_FL(0x04, SD4_C4),

    SD4_L_UP(0x04, SD4_C3),
    SD4_L_UP(0x04, SD4_D3),
    SD4_L_UP(0x04, SD4_E3),
    SD4_L_UP(0x04, SD4_F3),
    SD4_L_UP(0x04, SD4_G3),
    SD4_L_UP(0x04, SD4_A3),
    SD4_L_UP(0x04, SD4_B3),
    SD4_L_UP(0x04, SD4_C4),

    SD4_L_DW(0x04, SD4_C3),
    SD4_L_DW(0x04, SD4_D3),
    SD4_L_DW(0x04, SD4_E3),
    SD4_L_DW(0x04, SD4_F3),
    SD4_L_DW(0x04, SD4_G3),
    SD4_L_DW(0x04, SD4_A3),
    SD4_L_DW(0x04, SD4_B3),
    SD4_L_DW(0x04, SD4_C4),
#endif
#if 0   // Lead スライド
    // スライドなし
    SD4_L_FL(0x80, SD4_C4),  SD4_B_REST(0x80),  SD4_C2_REST(0x80),  SD4_D_REST(0x80),

    // スラー
    //SD4_L_SLIDE_SPEED_R1,
    SD4_L_SLIDE_OFF,
    SD4_L_UP(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_FL(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_FL(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
    SD4_L_FL(0x80, SD4_C4), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),

    // スライド速度1/1
    SD4_L_SLIDE_SPEED_R1,
    SD4_L_FL(0x80, SD4_C5), SD4_B_REST(0x100), SD4_C2_REST(0x100), SD4_D_REST(0x100),
    SD4_L_FL(0x80, SD4_C4),

    // スライド速度1/4
    SD4_L_SLIDE_SPEED_R4,
    SD4_L_FL(0xc0, SD4_C5), SD4_B_REST(0xc0), SD4_C2_REST(0xc0), SD4_D_REST(0xc0),
    SD4_L_FL(0xc0, SD4_C4), SD4_B_REST(0xc0), SD4_C2_REST(0xc0), SD4_D_REST(0xc0),

    // スライドなし
    SD4_L_SLIDE_OFF,
    SD4_L_FL(0x80, SD4_C5), SD4_B_REST(0x100), SD4_C2_REST(0x100), SD4_D_REST(0x100),
    SD4_L_FL(0x80, SD4_C4),
#endif
#if 0   // Base
    // Base ◢■◣
    SD4_L_REST(0xc0), SD4_B_UP(0xc0, SD4_C2), SD4_C2_REST(0xc0),               SD4_D_REST(0xc0),
    SD4_L_REST(0x80), SD4_B_FL(0x80, SD4_C2), SD4_C2_REST(0x80),               SD4_D_REST(0x80),
    SD4_L_REST(0xc0), SD4_B_DW(0xc0, SD4_C2), SD4_C2_REST(0xc0),               SD4_D_REST(0xc0),
    SD4_L_REST(0x80), SD4_B_UP(0x80, SD4_C4), SD4_C2_REST(0x80),               SD4_D_REST(0x80),
    SD4_L_REST(0x80), SD4_B_FL(0x80, SD4_C4), SD4_C2_REST(0x80),               SD4_D_REST(0x80),
    SD4_L_REST(0x80), SD4_B_DW(0x80, SD4_C4), SD4_C2_REST(0x80),               SD4_D_REST(0x80),
    SD4_L_REST(0x80), SD4_B_UP(0x80, SD4_C6), SD4_C2_REST(0x80),               SD4_D_REST(0x80),
    SD4_L_REST(0x80), SD4_B_FL(0x80, SD4_C6), SD4_C2_REST(0x80),               SD4_D_REST(0x80),
    SD4_L_REST(0x80), SD4_B_DW(0x80, SD4_C6), SD4_C2_REST(0x80),               SD4_D_REST(0x80),

    // Base 高速再生
    SD4_L_REST(0x60), SD4_B_FL(0x04, SD4_C3), SD4_C2_REST(0x60), SD4_D_REST(0x60),
    /**/              SD4_B_FL(0x04, SD4_D3),
    /**/              SD4_B_FL(0x04, SD4_E3),
    /**/              SD4_B_FL(0x04, SD4_F3),
    /**/              SD4_B_FL(0x04, SD4_G3),
    /**/              SD4_B_FL(0x04, SD4_A3),
    /**/              SD4_B_FL(0x04, SD4_B3),
    /**/              SD4_B_FL(0x04, SD4_C4),

    /**/              SD4_B_UP(0x04, SD4_C3),
    /**/              SD4_B_UP(0x04, SD4_D3),
    /**/              SD4_B_UP(0x04, SD4_E3),
    /**/              SD4_B_UP(0x04, SD4_F3),
    /**/              SD4_B_UP(0x04, SD4_G3),
    /**/              SD4_B_UP(0x04, SD4_A3),
    /**/              SD4_B_UP(0x04, SD4_B3),
    /**/              SD4_B_UP(0x04, SD4_C4),

    /**/              SD4_B_DW(0x04, SD4_C3),
    /**/              SD4_B_DW(0x04, SD4_D3),
    /**/              SD4_B_DW(0x04, SD4_E3),
    /**/              SD4_B_DW(0x04, SD4_F3),
    /**/              SD4_B_DW(0x04, SD4_G3),
    /**/              SD4_B_DW(0x04, SD4_A3),
    /**/              SD4_B_DW(0x04, SD4_B3),
    /**/              SD4_B_DW(0x04, SD4_C4),
#endif
#if 0   // Chord2
    // Chord2 ◢■◣
    SD4_L_REST(0xc0), SD4_B_REST(0xc0),       SD4_C2_UP(0xc0, SD4_C2, SD4_C2), SD4_D_REST(0xc0),
    SD4_L_REST(0x80), SD4_B_REST(0x80),       SD4_C2_FL(0x80, SD4_C2, SD4_C2), SD4_D_REST(0x80),
    SD4_L_REST(0xc0), SD4_B_REST(0xc0),       SD4_C2_DW(0xc0, SD4_C2, SD4_C2), SD4_D_REST(0xc0),
    SD4_L_REST(0x80), SD4_B_REST(0x80),       SD4_C2_UP(0x80, SD4_C4, SD4_C4), SD4_D_REST(0x80),
    SD4_L_REST(0x80), SD4_B_REST(0x80),       SD4_C2_FL(0x80, SD4_C4, SD4_C4), SD4_D_REST(0x80),
    SD4_L_REST(0x80), SD4_B_REST(0x80),       SD4_C2_DW(0x80, SD4_C4, SD4_C4), SD4_D_REST(0x80),
    SD4_L_REST(0x80), SD4_B_REST(0x80),       SD4_C2_UP(0x80, SD4_C6, SD4_C6), SD4_D_REST(0x80),
    SD4_L_REST(0x80), SD4_B_REST(0x80),       SD4_C2_FL(0x80, SD4_C6, SD4_C6), SD4_D_REST(0x80),
    SD4_L_REST(0x80), SD4_B_REST(0x80),       SD4_C2_DW(0x80, SD4_C6, SD4_C6), SD4_D_REST(0x80),

    // Chord2 アルペジオ
    SD4_L_REST(0x80), SD4_B_REST(0x80),       SD4_C2_UP(0x80, SD4_C4, SD4_E4), SD4_D_REST(0x80),
    SD4_L_REST(0x80), SD4_B_REST(0x80),       SD4_C2_FL(0x80, SD4_C4, SD4_E4), SD4_D_REST(0x80),
    SD4_L_REST(0x80), SD4_B_REST(0x80),       SD4_C2_DW(0x80, SD4_C4, SD4_E4), SD4_D_REST(0x80),

    // Chord2 高速再生
    SD4_L_REST(0x60), SD4_B_REST(0x60), SD4_C2_FL(0x04, SD4_C3, SD4_C3), SD4_D_REST(0x60),
                                        SD4_C2_FL(0x04, SD4_D3, SD4_D3),
                                        SD4_C2_FL(0x04, SD4_E3, SD4_E3),
                                        SD4_C2_FL(0x04, SD4_F3, SD4_F3),
                                        SD4_C2_FL(0x04, SD4_G3, SD4_G3),
                                        SD4_C2_FL(0x04, SD4_A3, SD4_A3),
                                        SD4_C2_FL(0x04, SD4_B3, SD4_B3),
                                        SD4_C2_FL(0x04, SD4_C4, SD4_C4),

                                        SD4_C2_UP(0x04, SD4_C3, SD4_C3),
                                        SD4_C2_UP(0x04, SD4_D3, SD4_D3),
                                        SD4_C2_UP(0x04, SD4_E3, SD4_E3),
                                        SD4_C2_UP(0x04, SD4_F3, SD4_F3),
                                        SD4_C2_UP(0x04, SD4_G3, SD4_G3),
                                        SD4_C2_UP(0x04, SD4_A3, SD4_A3),
                                        SD4_C2_UP(0x04, SD4_B3, SD4_B3),
                                        SD4_C2_UP(0x04, SD4_C4, SD4_C4),

                                        SD4_C2_DW(0x04, SD4_C3, SD4_C3),
                                        SD4_C2_DW(0x04, SD4_D3, SD4_D3),
                                        SD4_C2_DW(0x04, SD4_E3, SD4_E3),
                                        SD4_C2_DW(0x04, SD4_F3, SD4_F3),
                                        SD4_C2_DW(0x04, SD4_G3, SD4_G3),
                                        SD4_C2_DW(0x04, SD4_A3, SD4_A3),
                                        SD4_C2_DW(0x04, SD4_B3, SD4_B3),
                                        SD4_C2_DW(0x04, SD4_C4, SD4_C4),
    SD4_L_REST(0x40), SD4_B_REST(0x40), SD4_C2_REST(0x40),               SD4_D_REST(0x40),
#endif
#if 0
    // Drum
    SD4_L_REST(0x40),  SD4_B_REST(0x40),  SD4_C2_REST(0x40), SD4_D_REST(0x40),

    SD4_L_REST(0x100), SD4_B_REST(0x100), SD4_C2_REST(0x100),
    SD4_D_1(0x20), SD4_D_1(0x20), SD4_D_1(0x20), SD4_D_1(0x20),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),

    //SD4_L_FL(0x100, SD4_C3), SD4_B_FL(0x100, SD4_C3), SD4_C2_FL(0x100, SD4_C3, SD4_C3),
    SD4_L_FL(0x100, SD4_C3), SD4_B_REST(0x100), SD4_C2_REST(0x100),
    SD4_D_1(0x20), SD4_D_1(0x20), SD4_D_1(0x20), SD4_D_1(0x20),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_1(0x04),

    SD4_L_REST(0x100), SD4_B_REST(0x100), SD4_C2_REST(0x100),
    SD4_D_2(0x20), SD4_D_2(0x20), SD4_D_2(0x20), SD4_D_2(0x20),
    SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04),
    SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04),
    SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04),
    SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04),
    SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04),
    SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04),
    SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04),
    SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04), SD4_D_2(0x04),

    SD4_L_REST(0x100), SD4_B_REST(0x100), SD4_C2_REST(0x100),
    SD4_D_3(0x20), SD4_D_3(0x20), SD4_D_3(0x20), SD4_D_3(0x20),
    SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04),
    SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04),
    SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04),
    SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04),
    SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04),
    SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04),
    SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04),
    SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04), SD4_D_3(0x04),

    SD4_L_REST(0x100), SD4_B_REST(0x100), SD4_C2_REST(0x100),
    SD4_D_1(0x20), SD4_D_2(0x20), SD4_D_3(0x20),
    SD4_D_REST(0x80),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_3(0x04), SD4_D_1(0x04),
    SD4_D_1(0x04), SD4_D_1(0x04), SD4_D_3(0x04), SD4_D_1(0x04),
#endif
#if 0 //とりあえず全部出してみる
    SD4_L_REST(0x40),       SD4_B_REST(0x40),       SD4_C2_REST(0x40),               SD4_D_REST(0x40),
    SD4_L_DW(0x40, SD4_C3), SD4_B_REST(0x40),       SD4_C2_REST(0x40),               SD4_D_1(0x40),
    SD4_L_DW(0x40, SD4_C3), SD4_B_REST(0x40),       SD4_C2_REST(0x40),               SD4_D_2(0x40),
    SD4_L_DW(0x40, SD4_C3), SD4_B_DW(0x40, SD4_G3), SD4_C2_REST(0x40),               SD4_D_3(0x40),
    SD4_L_DW(0x40, SD4_C3), SD4_B_DW(0x40, SD4_G3), SD4_C2_DW(0x40, SD4_C4, SD4_C4), SD4_D_1(0x40),
    SD4_L_DW(0x40, SD4_C3), SD4_B_DW(0x40, SD4_G3), SD4_C2_DW(0x40, SD4_C4, SD4_E4), SD4_D_2(0x40),
    SD4_L_DW(0x40, SD4_C3), SD4_B_DW(0x40, SD4_G3), SD4_C2_DW(0x40, SD4_C4, SD4_E4), SD4_D_3(0x40),
    SD4_L_DW(0x40, SD4_C3), SD4_B_DW(0x40, SD4_G3), SD4_C2_DW(0x40, SD4_C4, SD4_E4), SD4_D_1(0x40),
    SD4_L_DW(0x40, SD4_C3), SD4_B_DW(0x40, SD4_G3), SD4_C2_DW(0x40, SD4_C4, SD4_E4), SD4_D_2(0x40),
    SD4_L_DW(0x40, SD4_C3), SD4_B_DW(0x40, SD4_G3), SD4_C2_DW(0x40, SD4_C4, SD4_E4), SD4_D_3(0x40),
    SD4_L_FL(0x80, SD4_C3), SD4_B_FL(0x80, SD4_G3), SD4_C2_FL(0x80, SD4_C4, SD4_E4), SD4_D_1(0x80),
    SD4_L_FL(0x80, SD4_C3), SD4_B_FL(0x80, SD4_G3), SD4_C2_FL(0x80, SD4_C4, SD4_E4), SD4_D_1(0x80),
    SD4_L_FL(0x80, SD4_C3), SD4_B_FL(0x80, SD4_G3), SD4_C2_FL(0x80, SD4_C4, SD4_E4), SD4_D_1(0x80),
    SD4_L_FL(0x80, SD4_C3), SD4_B_FL(0x80, SD4_G3), SD4_C2_FL(0x80, SD4_C4, SD4_E4), SD4_D_1(0x80),
#endif
#if 1 // リピート
    // 2回リピート
    SD4_L_REP_2,
        SD4_L_FL(0x40, SD4_C3), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
        SD4_L_FL(0x40, SD4_D3),
    SD4_L_ENDR,

    SD4_L_REST(0x20),       SD4_B_REST(0x20), SD4_C2_REST(0x20), SD4_D_REST(0x20),

    // 8回リピート
    SD4_L_REP_8,
        SD4_L_FL(0x10, SD4_C3), SD4_B_REST(0x20), SD4_C2_REST(0x20), SD4_D_REST(0x20),
        SD4_L_FL(0x10, SD4_D3),
    SD4_L_ENDR,

    SD4_L_REST(0x20),       SD4_B_REST(0x20), SD4_C2_REST(0x20), SD4_D_REST(0x20),

    // 2重ネスト
    SD4_L_REP_2,
        SD4_L_REP_3,
            SD4_L_FL(0x40, SD4_E3), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
            SD4_L_FL(0x40, SD4_F3),
        SD4_L_ENDR,
        SD4_L_FL(0x40, SD4_G3), SD4_B_REST(0x80), SD4_C2_REST(0x80), SD4_D_REST(0x80),
        SD4_L_FL(0x40, SD4_A3),
    SD4_L_ENDR,
#endif
    // 終了
    SD4_L_END,
};
#endif


void main(void)
{
    sd4Init();
    u8 c = sd4play(((u32)1 << 16) | (const u16)score0);

    bankRomVramMmio();
    if (c & KEY9_F1_MASK) {
        puts("ABORTED WITH F1 KEY.\x0d");
        return;
    }
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
