/**** This file is made by ../tools/aal80.php.  DO NOT MODIFY! ****/






#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "../src-common/display-code.h"


extern int _vvramVSyncAndWait_;


#define VVRAM_TEXT  0x8000      
#define VVRAM_ATB   0x8400      
#define VVRAM_SIZE  1000        
#define V_CT        262         




#pragma save
#pragma disable_warning 85          
static void DISP_NIBBLE(int A, int HL)  __naked
{
__asm
DISP_NIBBLE macro A, HL
local DISP_NIBBLE__0
  // line 29
  and A, 0 + 0x0f
  cp A, 0 + 10
  jr c, DISP_NIBBLE__0 // if (ge_jr) {
      add A, 0 + DC_A - 10 - DC_0
  DISP_NIBBLE__0: // } endif 
  add A, 0 + DC_0
  // line 30
  ld (HL), A
  // line 31
  inc HL
endm
__endasm;
}

#pragma restore


void main(void)  __naked
{
__asm
  // line 43
  ld HL, 0 + VVRAM_TEXT
  ld DE, 0 + VVRAM_TEXT + 1
  ld BC, 0 + VVRAM_SIZE
  ld (HL), 0 + 0x00
  ldir
  // line 44
  ld HL, 0 + VVRAM_ATB
  ld DE, 0 + VVRAM_ATB  + 1
  ld BC, 0 + VVRAM_SIZE
  ld (HL), 0 + VATB(7, 0, 0)
  ldir
  // line 49
  ld HL, 0 + MMIO_8253_CTRL
  // line 50
  ld (HL), 0 + MMIO_8253_CTRL_CT1(MMIO_8253_CTRL_RL_LH_MASK, MMIO_8253_CTRL_MODE2_MASK)
  // line 51
  ld (HL), 0 + MMIO_8253_CTRL_CT2(MMIO_8253_CTRL_RL_L_MASK, MMIO_8253_CTRL_MODE2_MASK)
  // line 52
  dec L
  // line 53
  ld (HL), 0 + 0x00
  // line 54
  ld (HL), 0 + 0x00
  // line 55
  dec L
  // line 56
  ld DE, 0 + V_CT
  // line 58
  push HL
    // line 59
    ld BC, 0 + 2712 + 100 + 15
    // line 60
    call 0 + _vvramVSyncAndWait_
  pop HL
  // line 63
  ld (HL), E
  // line 64
  ld (HL), D
  // line 67
main__0: // while (true) {
      // line 68
      ld BC, 0 + 250
      // line 69
      ld HL, 0 + VVRAM_TEXT
      ld DE, 0 + VVRAM_ATB
      // line 72
      exx
        // line 73
        ld HL, 0 + MMIO_8255_PORTC
        // line 74
        xor A, A
        // line 75
        main__2: // do {
            or A, (HL)
        jp p, main__2 // } while (p)
        main__3: // loop end
        // line 76
        main__4: // do {
            and A, (HL)
        jp m, main__4 // } while (m)
        main__5: // loop end
      exx
      // line 80
      main__6: // do {
          // line 82
          ld A, (MMIO_8253_CT1)
          // line 83
          ex AF, AF
            // line 84
            ld A, (MMIO_8253_CT1)
            // line 85
#if  0           
            or A, A
            jp nz, main__8 // if (z) {
                // line 87
                ex AF, AF
                cp A, 0 + 0x12
                jp nc, main__9 // if (lt) {
                    // line 88
aaaa:
                    nop
                main__9: // } endif 
                // line 90
                ex AF, AF
            main__8: // } endif 
            // line 92
#endif 
            // line 94
            push AF
              // line 95
              rrca
              rrca
              rrca
              rrca
              // line 96
              DISP_NIBBLE A, HL
            pop AF
            // line 99
            DISP_NIBBLE A, HL
          ex AF, AF
          // line 102
          push AF
            // line 103
            rrca
            rrca
            rrca
            rrca
            // line 104
            DISP_NIBBLE A, HL
          pop AF
          // line 107
          DISP_NIBBLE A, HL
          // line 110
          ld A, (MMIO_8255_PORTC)
          or A, A
          // line 111
          ld A, 0 + VATB(CYAN, 0, 0)
          jp m, main__10 // if (p) {
              ld A, 0 + VATB(CYAN, RED, 0)
          main__10: // } endif 
          // line 114
          bit 0, C
          jr nz, main__11 // if (z_jr) {
              or A, 0 + VATB(WHITE, 0, 0)
          main__11: // } endif 
          // line 115
          ld (DE), A
          inc DE
          ld (DE), A
          inc DE
          // line 116
          ld (DE), A
          inc DE
          ld (DE), A
          inc DE
          // line 118
          dec BC
          ld A, B
          or A, C
      jp nz, main__6 // } while (nz)
      main__7: // loop end
      // line 123
      ld HL, 0 + VVRAM_TEXT
      ld DE, 0 + VRAM_TEXT
      ld BC, 0 + VVRAM_SIZE
      // line 124
      main__12: // do {
          ld A, (HL)
          ld (DE), A
          inc HL
          inc DE
          dec BC
          ld A, B
          or A, C
      jp nz, main__12 // } while (nz)
      main__13: // loop end
      // line 125
      ld HL, 0 + VVRAM_ATB
      ld DE, 0 + VRAM_ATB
      ld BC, 0 + VVRAM_SIZE
      // line 126
      main__14: // do {
          ld A, (HL)
          ld (DE), A
          inc HL
          inc DE
          dec BC
          ld A, B
          or A, C
      jp nz, main__14 // } while (nz)
      main__15: // loop end
  jp main__0 // }
main__1: // loop end
__endasm;
}










static void vvramVSyncAndWait_(void) 
{
__asm
  // line 144
  ld HL, 0 + MMIO_8255_PORTC
  // line 145
  xor A, A
  // line 146
  vvramVSyncAndWait___0: // do {
      or A, (HL)
  jp p, vvramVSyncAndWait___0 // } while (p)
  vvramVSyncAndWait___1: // loop end
  // line 147
  vvramVSyncAndWait___2: // do {
      and A, (HL)
  jp m, vvramVSyncAndWait___2 // } while (m)
  vvramVSyncAndWait___3: // loop end
  // line 148
  vvramVSyncAndWait___4: // do {
      dec BC
      ld A, B
      or A, C
  jr nz, vvramVSyncAndWait___4 // } while (nz_jr)
  vvramVSyncAndWait___5: // loop end
__endasm;
}

