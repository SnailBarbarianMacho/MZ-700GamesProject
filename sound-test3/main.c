/**** This file is made by ../tools/aal80.php.  DO NOT MODIFY! ****/






#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "sound.h"

#pragma disable_warning 85
#pragma save
static void puts(u8 const * const str)  __naked
{
__asm
  // line 17
  ld DE, HL
  // line 18
  jp 21
__endasm;
}

#pragma restore


static void bankRomVramMmio(void)  __naked
{
__asm
  // line 29
  BANKL_ROM_BANKH_VRAM_MMIO 
  // line 30
  jp 6
__endasm;
}



#if 1
#include "music/chronos.h"
#else
static u8 const sd6_chronos_0[] = {
#if 0   
    SD6_L_REST(0x100), SD6_B_REST(0x100), SD6_C2_REST(0x100), SD6_D_REST(0x100),
    SD6_L_REST(0x100), SD6_B_REST(0x100), SD6_C2_REST(0x100), SD6_D_REST(0x100),
#endif
#if 0   
    
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
    
    SD6_L_ENV_SPEED_R16,
    SD6_L_UP(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_FL(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_DW(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),

    
    SD6_L_ENV_SPEED_R2,
    SD6_L_UP(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_FL(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
    SD6_L_DW(0x80, SD6_C4), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),

    
    SD6_L_FL(0x20, SD6_D3), SD6_B_REST(0xe0), SD6_C2_REST(0xe0), SD6_D_REST(0xe0),
    SD6_L_FL(0x20, SD6_E3),
    SD6_L_FL(0x20, SD6_F3),
    SD6_L_FL(0x20, SD6_G3),
    SD6_L_FL(0x20, SD6_A3),
    SD6_L_FL(0x20, SD6_B3),
    SD6_L_FL(0x20, SD6_C4),

    
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
#if 0   
    
    SD6_L_FL(0x80, SD6_C4),  SD6_B_REST(0x80),  SD6_C2_REST(0x80),  SD6_D_REST(0x80),

    
    SD6_L_SLIDE_SPEED_R1,
    SD6_L_FL(0x80, SD6_C5), SD6_B_REST(0x100), SD6_C2_REST(0x100), SD6_D_REST(0x100),
    SD6_L_FL(0x80, SD6_C4),

    
    SD6_L_SLIDE_SPEED_R4,
    SD6_L_FL(0xc0, SD6_C5), SD6_B_REST(0xc0), SD6_C2_REST(0xc0), SD6_D_REST(0xc0),
    SD6_L_FL(0xc0, SD6_C4), SD6_B_REST(0xc0), SD6_C2_REST(0xc0), SD6_D_REST(0xc0),

    
    SD6_L_SLIDE_OFF,
    SD6_L_FL(0x80, SD6_C5), SD6_B_REST(0x100), SD6_C2_REST(0x100), SD6_D_REST(0x100),
    SD6_L_FL(0x80, SD6_C4),
#endif
#if 0   
    
    SD6_L_REST(0xc0), SD6_B_UP(0xc0, SD6_C2), SD6_C2_REST(0xc0),               SD6_D_REST(0xc0),
    SD6_L_REST(0x80), SD6_B_FL(0x80, SD6_C2), SD6_C2_REST(0x80),               SD6_D_REST(0x80),
    SD6_L_REST(0xc0), SD6_B_DW(0xc0, SD6_C2), SD6_C2_REST(0xc0),               SD6_D_REST(0xc0),
    SD6_L_REST(0x80), SD6_B_UP(0x80, SD6_C4), SD6_C2_REST(0x80),               SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_FL(0x80, SD6_C4), SD6_C2_REST(0x80),               SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_DW(0x80, SD6_C4), SD6_C2_REST(0x80),               SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_UP(0x80, SD6_C6), SD6_C2_REST(0x80),               SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_FL(0x80, SD6_C6), SD6_C2_REST(0x80),               SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_DW(0x80, SD6_C6), SD6_C2_REST(0x80),               SD6_D_REST(0x80),

    
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
#if 0   
    
    SD6_L_REST(0xc0), SD6_B_REST(0xc0),       SD6_C2_UP(0xc0, SD6_C2, SD6_C2), SD6_D_REST(0xc0),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_FL(0x80, SD6_C2, SD6_C2), SD6_D_REST(0x80),
    SD6_L_REST(0xc0), SD6_B_REST(0xc0),       SD6_C2_DW(0xc0, SD6_C2, SD6_C2), SD6_D_REST(0xc0),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_UP(0x80, SD6_C4, SD6_C4), SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_FL(0x80, SD6_C4, SD6_C4), SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_DW(0x80, SD6_C4, SD6_C4), SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_UP(0x80, SD6_C6, SD6_C6), SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_FL(0x80, SD6_C6, SD6_C6), SD6_D_REST(0x80),
    SD6_L_REST(0x80), SD6_B_REST(0x80),       SD6_C2_DW(0x80, SD6_C6, SD6_C6), SD6_D_REST(0x80),

    
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
    
    SD6_L_REST(0x100), SD6_B_REST(0x100), SD6_C2_REST(0x100),
    SD6_D_1(0x20), SD6_D_2(0x20), SD6_D_3(0x20),
    SD6_D_REST(0x80),
    SD6_D_1(0x04), SD6_D_1(0x04), SD6_D_3(0x04), SD6_D_1(0x04),
    SD6_D_1(0x04), SD6_D_1(0x04), SD6_D_3(0x04), SD6_D_1(0x04),
#endif
#if 0 
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
#if 1 
    
    SD6_L_REP_2,
        SD6_L_FL(0x40, SD6_C3), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
        SD6_L_FL(0x40, SD6_D3),
    SD6_L_ENDR,

    SD6_L_REST(0x20),       SD6_B_REST(0x20), SD6_C2_REST(0x20), SD6_D_REST(0x20),

    
    SD6_L_REP_8,
        SD6_L_FL(0x10, SD6_C3), SD6_B_REST(0x20), SD6_C2_REST(0x20), SD6_D_REST(0x20),
        SD6_L_FL(0x10, SD6_D3),
    SD6_L_ENDR,

    SD6_L_REST(0x20),       SD6_B_REST(0x20), SD6_C2_REST(0x20), SD6_D_REST(0x20),

    
    SD6_L_REP_2,
        SD6_L_REP_3,
            SD6_L_FL(0x40, SD6_E3), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
            SD6_L_FL(0x40, SD6_F3),
        SD6_L_ENDR,
        SD6_L_FL(0x40, SD6_G3), SD6_B_REST(0x80), SD6_C2_REST(0x80), SD6_D_REST(0x80),
        SD6_L_FL(0x40, SD6_A3),
    SD6_L_ENDR,
#endif
    
    SD6_L_END,
};
#endif


void main(void)
{
    sd6Init();
    u8 c = SD6PLAY(chronos, 0, true);

    bankRomVramMmio();
    if (c & KEY9_F1_MASK) {
        puts("ABORTED WITH F1 KEY.\x0d");
        return;
    }
    if (c & KEY9_F2_MASK) {
        puts("ABORTED WITH F2 KEY.\x0d");
        return;
    }
    if (c & KEY9_F3_MASK) {
        puts("ABORTED WITH F3 KEY.\x0d");
        return;
    }
    if (c & KEY9_F4_MASK) {
        puts("ABORTED WITH F4 KEY.\x0d");
        return;
    }
    puts("END.\x0d");
}   
