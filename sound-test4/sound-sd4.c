/**** This file is made by ../tools/aal80.php.  DO NOT MODIFY! ****/





#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "../src-common/asm-macros.h"

#include "addr.h"
#include "sound.h"



    STATIC_ASSERT(SD4_B1 < 256 + 1, "valueOverflow"); 
    STATIC_ASSERT(SD4_C6 >= 15,     "valueUnderflow");

#if 0
    STATIC_ASSERT(SD4_WL(SD_FREQ_C2) < 128.0f, "valueOverflow");    
    STATIC_ASSERT(SD4_WL(SD_FREQ_C5) > 15.0f,  "valueUnderflow");   
    #define MMIO_8253_CT0_PWM   MMIO_8253_CTRL_CT0(MMIO_8253_CTRL_RL_L_MASK, MMIO_8253_CTRL_MODE0_MASK)
#endif




#ifndef SD4_EXCEPT_LEN
#define SD4_EXCEPT_LEN      0
#endif

#define TEMPO               0       
#define CHORD_VOL_SPEED_R   7       



#define LEAD_VOL_MAX        22
#define LEAD_VOL_MIN_DW     8   
#define LEAD_VOL_MIN_UP     5   
#define CHORD_VOL_MAX       21
#define CHORD_VOL_MIN_DW    7   
#define BASE_VOL_MAX        20
#define BASE_VOL_MIN_DW     7   






























#pragma save
#pragma disable_warning 85          
#pragma disable_warning 59          



static u8 const SD4_DATA_[] = {
    
    
    
    0,56,55,54, 54,53,52,51,  50,49,48,47, 47,46,45,44,
    43,42,41,40, 40,39,38,37, 36,35,34,33, 33,32,31,30,
    29,28,27,26, 26,25,24,23, 22,21,20,19, 19,18,17,16,
    15,14,13,12, 12,11,10,9, 8,7,6,5, 4,3,2,1,





    0x03, 0x00, 0, 0,
    0x07, 0x00, 0, 0,
    0x0f, 0x01, 0, 0,
    0x1f, 0x03, 0, 0,

#include "music/drum1.h"
#include "music/drum2.h"
#include "music/drum3.h"
};



void sd4Init(void) 
{
__asm
  // line 107
  extern _SD4_DATA_
  // line 108
  extern sd4Init_dataLoop
  // line 111
  ld HL, 0 + _SD4_DATA_
  // line 112
  ld DE, 0 + ADDR_SD4_VTAB
  // line 113
  ld BC, 0 + SZ_SD4_VTAB+SZ_SD4_TAB
  // line 114
  ldir
  // line 116
  ld DE, 0 + ADDR_SD4_DRUM
  // line 117
  ld BC, 0 + SZ_SD4_DRUM
  // line 118
  ldir
  // line 121
  BANKH_VRAM_MMIO 
  // line 122
  ld HL, 0 + MMIO_ETC
  // line 123
  ld (HL), 0 + MMIO_ETC_GATE_MASK
  // line 124
  dec L
  // line 125
  ld (HL), 0 + MMIO_8253_CT0_MODE3
  // line 126
  BANKH_RAM 
__endasm;
}






























void SD4PLAY_NOTE_LEAD(
    int H_wlc, int L_hwl, int HL_wlchwl, int D_vol, int IXH_slc,
    int inc_vol, int dec_vol,
    int addr_rep_1,
    int addr_slide_0,
    int addr_mod_start_1,
    int addr_mod_vol, int addr_mod_vol_speed_1, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_mod_wl,  int addr_mod_wl_speed_1,  int addr_mod_wl_end_1,  int addr_mod_wl_inc_0,
    int addr_wl_1,
    int addr_mod_end,
    int addr_end
)  __naked
{
__asm
SD4PLAY_NOTE_LEAD macro H_wlc, L_hwl, HL_wlchwl, D_vol, IXH_slc, inc_vol, dec_vol, addr_rep_1, addr_slide_0, addr_mod_start_1, addr_mod_vol, addr_mod_vol_speed_1, addr_mod_vol_end_1, addr_mod_vol_inc_0, addr_mod_wl, addr_mod_wl_speed_1, addr_mod_wl_end_1, addr_mod_wl_inc_0, addr_wl_1, addr_mod_end, addr_end
local SD4PLAY_NOTE_LEAD__else0, SD4PLAY_NOTE_LEAD__endif0, SD4PLAY_NOTE_LEAD__else1, SD4PLAY_NOTE_LEAD__endif1, SD4PLAY_NOTE_LEAD__else2, SD4PLAY_NOTE_LEAD__endif2, SD4PLAY_NOTE_LEAD__else3, SD4PLAY_NOTE_LEAD__endif3, SD4PLAY_NOTE_LEAD__else4, SD4PLAY_NOTE_LEAD__endif4, SD4PLAY_NOTE_LEAD__else5, SD4PLAY_NOTE_LEAD__endif5, SD4PLAY_NOTE_LEAD__else6, SD4PLAY_NOTE_LEAD__endif6, SD4PLAY_NOTE_LEAD__else7, SD4PLAY_NOTE_LEAD__endif7, SD4PLAY_NOTE_LEAD__else8, SD4PLAY_NOTE_LEAD__endif8, SD4PLAY_NOTE_LEAD__else9, SD4PLAY_NOTE_LEAD__endif9, SD4PLAY_NOTE_LEAD__else10, SD4PLAY_NOTE_LEAD__endif10, SD4PLAY_NOTE_LEAD__else11, SD4PLAY_NOTE_LEAD__endif11, SD4PLAY_NOTE_LEAD__else12, SD4PLAY_NOTE_LEAD__endif12, SD4PLAY_NOTE_LEAD__else13, SD4PLAY_NOTE_LEAD__endif13, SD4PLAY_NOTE_LEAD__else14, SD4PLAY_NOTE_LEAD__endif14
  // line 171
  extern sd4playNoteLead_pop0, sd4playNoteLead_pop, sd4playNoteLead_rep_2, sd4playNoteLead_initDw, sd4playNoteLead_initDw_1, sd4playNoteLead_initEnd, sd4playNoteLead_initEndSlur
  // line 178
  dec IXH_slc
  jp nz, SD4PLAY_NOTE_LEAD__endif0
      // line 179
sd4playNoteLead_pop:
      // line 180
      pop HL_wlchwl
      // line 183
      ld A, L_hwl
      add A, A
      jr c, SD4PLAY_NOTE_LEAD__endif1
          // line 184
          add A, A
          ld IXH_slc, A
          jr nc, SD4PLAY_NOTE_LEAD__endif2
              // line 187
              cp A, 0 + SD4_EXCEPT_LEN
              jr nz, SD4PLAY_NOTE_LEAD__endif3
                  // line 188
addr_rep_1:
                  ld HL, 0
                  dec (HL)
                  jr nz, SD4PLAY_NOTE_LEAD__endif4
                      // line 189
                      dec L
                      dec L
                      dec L
                      ld (addr_rep_1+1), HL
                      // line 190
                      dec SP
                      // line 191
                      jp 0 + sd4playNoteLead_pop0
                  SD4PLAY_NOTE_LEAD__endif4:

                  // line 194
                  dec L
                  ld A, (HL)
                  dec L
                  ld L, (HL)
                  ld H, A
                  ld SP, HL
                  // line 195
                  jp 0 + sd4playNoteLead_pop0
              SD4PLAY_NOTE_LEAD__endif3:

              // line 199
                // line 200
                cp A, 0 + SD4_EXCEPT_LEN+4
                jp z, 0 + addr_end
              // line 204
                // line 205
                ld A, (VRAM_TEXT)
                ex AF, AF
                ld (MMIO_8253_CT0), A
                ex AF, AF
                // line 206
                ld A, H_wlc
                ld (addr_wl_1+1), A
                // line 207
                ld HL_wlchwl, 0 + addr_mod_vol
                ld (addr_mod_start_1+1), HL_wlchwl
                // line 208
                ld HL_wlchwl, 0 + (LEAD_VOL_MAX<<8)|inc_vol
                // line 209
                ld D_vol, 0 + LEAD_VOL_MIN_UP
                // line 210
                jp 0 + sd4playNoteLead_initDw
          SD4PLAY_NOTE_LEAD__endif2:

          // line 215
          cp A, 0 + SD4_EXCEPT_LEN
          jp c, SD4PLAY_NOTE_LEAD__endif5
              // line 216
              cp A, 0 + SD4_EXCEPT_LEN+4*7
              jp nc, SD4PLAY_NOTE_LEAD__endif6
                  // line 217
                  dec SP
                  // line 218
                  sub A, 0 + SD4_EXCEPT_LEN-8
                  // line 219
                  rrca
                  rrca
                  // line 220
                  ld HL, (addr_rep_1+1)
                  // line 221
                  inc L
                  ld (sd4playNoteLead_rep_2+2), HL
sd4playNoteLead_rep_2:
                  ld (0x0000), SP
                  // line 222
                  inc L
                  inc L
                  ld (HL), A
                  // line 223
                  ld (addr_rep_1+1), HL
                  // line 224
                  jp 0 + sd4playNoteLead_pop0
              SD4PLAY_NOTE_LEAD__endif6:

          SD4PLAY_NOTE_LEAD__endif5:

          // line 229
            // line 230
            dec SP
            // line 231
            xor A, A
            ld (addr_wl_1+1), A
            // line 232
            ld HL_wlchwl, 0 + addr_mod_end
            ld (addr_mod_start_1+1), HL_wlchwl
            // line 233
            ld L_hwl, A
            // line 234
            jp 0 + sd4playNoteLead_initEndSlur
      SD4PLAY_NOTE_LEAD__endif1:

      // line 237
      add A, A
      ld IXH_slc, A
      jr nc, SD4PLAY_NOTE_LEAD__endif7
          // line 238
          cp A, 0 + SD4_EXCEPT_LEN
          jp c, SD4PLAY_NOTE_LEAD__endif8
              // line 239
              jr nz, SD4PLAY_NOTE_LEAD__endif9
                  // line 242
                  ld A, 0 + OPCODE_JR
                  ld (addr_slide_0+0), A
                  // line 243
                  dec SP
                  // line 244
                  jp 0 + sd4playNoteLead_pop0
              SD4PLAY_NOTE_LEAD__endif9:

              // line 247
              cp A, 0 + SD4_EXCEPT_LEN+4*4
              jp nc, SD4PLAY_NOTE_LEAD__endif10
                  // line 249
                  sub A, 0 + SD4_EXCEPT_LEN-(SZ_SD4_VTAB+1)
                  // line 250
                  ld H_wlc, 0 + ADDR_SD4_VTAB/256
                  ld L_hwl, A
                  ld A, (HL_wlchwl)
                  // line 251
                  ld (addr_mod_wl_speed_1+1), A
                  // line 252
                  ld A, 0 + OPCODE_LD_A_N
                  ld (addr_slide_0+0), A
                  // line 253
                  dec SP
                  // line 254
                  jp 0 + sd4playNoteLead_pop0
              SD4PLAY_NOTE_LEAD__endif10:

          SD4PLAY_NOTE_LEAD__endif8:

          // line 258
          ld A, (VRAM_TEXT)
          ex AF, AF
          ld (MMIO_8253_CT0), A
          ex AF, AF
          // line 259
addr_slide_0:
          jr nz, SD4PLAY_NOTE_LEAD__endif11
              // line 262
              ld A, H_wlc
              ld (addr_mod_wl_end_1+1), A
              // line 264
              ld A, (addr_wl_1+1)
              cp A, H_wlc
              ld A, 0 + OPCODE_INC_A
              jr c, SD4PLAY_NOTE_LEAD__endif12
                  inc A
              SD4PLAY_NOTE_LEAD__endif12:

              // line 265
              ld (addr_mod_wl_inc_0+0), A
              // line 266
              ld HL_wlchwl, 0 + addr_mod_wl
              ld (addr_mod_start_1+1), HL_wlchwl
              // line 267
              ld A, (addr_wl_1+1)
              srl A
              ld L_hwl, A
              // line 268
              jp 0 + sd4playNoteLead_initEndSlur
          SD4PLAY_NOTE_LEAD__endif11:

          // line 272
            // line 273
            ld A, H_wlc
            ld (addr_wl_1+1), A
            // line 274
            ld HL_wlchwl, 0 + addr_mod_end
            ld (addr_mod_start_1+1), HL_wlchwl
            // line 275
            srl A
            ld L_hwl, A
            // line 276
            ld D_vol, 0 + LEAD_VOL_MAX
            // line 277
            jp 0 + sd4playNoteLead_initEnd
      SD4PLAY_NOTE_LEAD__endif7:

      // line 281
      cp A, 0 + SD4_EXCEPT_LEN
      jr c, SD4PLAY_NOTE_LEAD__endif13
          // line 282
          cp A, 0 + SD4_EXCEPT_LEN+4*4
          jp nc, SD4PLAY_NOTE_LEAD__endif14
              // line 285
              sub A, 0 + SD4_EXCEPT_LEN-SZ_SD4_VTAB
              // line 286
              ld H_wlc, 0 + ADDR_SD4_VTAB/256
              ld L_hwl, A
              ld A, (HL_wlchwl)
              // line 287
              ld (addr_mod_vol_speed_1+1), A
              // line 288
              dec SP
              // line 289
sd4playNoteLead_pop0:
              // line 290
              ld A, (VRAM_TEXT)
              ex AF, AF
              ld (MMIO_8253_CT0), A
              ex AF, AF
              // line 291
              jp 0 + sd4playNoteLead_pop
          SD4PLAY_NOTE_LEAD__endif14:

      SD4PLAY_NOTE_LEAD__endif13:

      // line 298
        // line 299
        ld A, (VRAM_TEXT)
        ex AF, AF
        ld (MMIO_8253_CT0), A
        ex AF, AF
        // line 300
        ld A, H_wlc
        ld (addr_wl_1+1), A
        // line 301
        ld HL_wlchwl, 0 + addr_mod_vol
        ld (addr_mod_start_1+1), HL_wlchwl
        // line 302
        ld HL_wlchwl, 0 + (LEAD_VOL_MIN_DW<<8)|dec_vol
        // line 303
        ld D_vol, 0 + LEAD_VOL_MAX
      // line 305
sd4playNoteLead_initDw:
      // line 306
      srl A
      ld (sd4playNoteLead_initDw_1+1), A
      // line 307
      ld A, H_wlc
      ld (addr_mod_vol_end_1+1), A
      // line 308
      ld A, L_hwl
      ld (addr_mod_vol_inc_0+0), A
      // line 309
sd4playNoteLead_initDw_1:
      ld L_hwl, 0
      // line 311
sd4playNoteLead_initEnd:
      // line 312
      ld H_wlc, 1
      // line 314
sd4playNoteLead_initEndSlur:
      // line 315
      ld A, (VRAM_TEXT)
      ex AF, AF
      ld (MMIO_8253_CT0), A
      ex AF, AF
  SD4PLAY_NOTE_LEAD__endif0:

endm
__endasm;
}




















void SD4PLAY_MOD_LEAD(
    int L_hwl, int D_vol, int IXH_slc,
    int addr_mod_start_1,
    int addr_mod_vol, int addr_mod_vol_speed_1, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_mod_wl,  int addr_mod_wl_speed_1,  int addr_mod_wl_end_1,  int addr_mod_wl_inc_0,
    int addr_wl_1,
    int addr_mod_end
)  __naked
{
__asm
SD4PLAY_MOD_LEAD macro L_hwl, D_vol, IXH_slc, addr_mod_start_1, addr_mod_vol, addr_mod_vol_speed_1, addr_mod_vol_end_1, addr_mod_vol_inc_0, addr_mod_wl, addr_mod_wl_speed_1, addr_mod_wl_end_1, addr_mod_wl_inc_0, addr_wl_1, addr_mod_end
local SD4PLAY_MOD_LEAD__else0, SD4PLAY_MOD_LEAD__endif0, SD4PLAY_MOD_LEAD__else1, SD4PLAY_MOD_LEAD__endif1
  // line 350
addr_mod_start_1:
  jp 0 + addr_mod_end
  // line 353
addr_mod_vol:
  // line 354
  ld A, IXH_slc
addr_mod_vol_speed_1:
  and A, 0
  jr nz, SD4PLAY_MOD_LEAD__endif0
      // line 355
      ld A, D_vol
addr_mod_vol_end_1:
      cp A, 0
      // line 356
      jr z, SD4PLAY_MOD_LEAD__endif1
addr_mod_vol_inc_0:
          inc D_vol
      SD4PLAY_MOD_LEAD__endif1:

  SD4PLAY_MOD_LEAD__endif0:

  // line 358
  jp 0 + addr_mod_end
  // line 361
addr_mod_wl:
  // line 362
  ld A, IXH_slc
addr_mod_wl_speed_1:
  and A, 0
  jr nz, 0 + addr_mod_end
    // line 363
    ld A, (addr_wl_1+1)
addr_mod_wl_end_1:
    cp A, 0
    // line 364
    jr z, 0 + addr_mod_end
addr_mod_wl_inc_0:
    inc A
    ld (addr_wl_1+1), A
    // line 365
    srl A
    ld L_hwl, A
  // line 370
addr_mod_end:
endm
__endasm;
}

















void SD4PLAY_NOTE_BASE(
    int B_wlc, int C_hwl, int BC_wlwl, int E_vol, int IXL_slc,
    int inc_vol, int dec_vol,
    int addr_mod_start_0, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_wl_1
)  __naked
{
__asm
SD4PLAY_NOTE_BASE macro B_wlc, C_hwl, BC_wlwl, E_vol, IXL_slc, inc_vol, dec_vol, addr_mod_start_0, addr_mod_vol_end_1, addr_mod_vol_inc_0, addr_wl_1
local SD4PLAY_NOTE_BASE__else0, SD4PLAY_NOTE_BASE__endif0, SD4PLAY_NOTE_BASE__else1, SD4PLAY_NOTE_BASE__endif1, SD4PLAY_NOTE_BASE__else2, SD4PLAY_NOTE_BASE__endif2, SD4PLAY_NOTE_BASE__else3, SD4PLAY_NOTE_BASE__endif3
  // line 398
  extern sd4playNoteBase_fl, sd4playNoteBase_initEnd
  // line 401
  dec IXL_slc
  jr nz, SD4PLAY_NOTE_BASE__endif0
      // line 402
      pop BC_wlwl
      // line 405
      ld A, C_hwl
      add A, A
      jr c, SD4PLAY_NOTE_BASE__else1
          // line 406
          add A, A
          ld IXL_slc, A
          jr nc, SD4PLAY_NOTE_BASE__endif2
              // line 409
              ld A, B_wlc
              ld (addr_wl_1+1), A
              // line 410
              srl A
              ld C_hwl, A
              // line 411
              ld E_vol, 0
              // line 412
              ld A, 0 + BASE_VOL_MAX
              ld (addr_mod_vol_end_1+1), A
              // line 413
              ld A, 0 + inc_vol
              ld (addr_mod_vol_inc_0+0), A
              // line 414
              ld A, 0 + OPCODE_JR_NZ
              // line 415
              jp 0 + sd4playNoteBase_initEnd
          SD4PLAY_NOTE_BASE__endif2:

          // line 419
          dec SP
          // line 420
          xor A, A
          ld (addr_wl_1+1), A
          // line 421
          ld C_hwl, A
          // line 422
          ld A, 0 + OPCODE_JR
      jp SD4PLAY_NOTE_BASE__endif1
      SD4PLAY_NOTE_BASE__else1:
          // line 424
          add A, A
          ld IXL_slc, A
          // line 425
          ld A, B_wlc
          ld (addr_wl_1+1), A
          // line 426
          ld E_vol, 0 + BASE_VOL_MAX
          // line 428
          jr nc, SD4PLAY_NOTE_BASE__else3
              // line 431
              srl A
              ld C_hwl, A
              // line 432
              ld A, 0 + OPCODE_JR
          jp SD4PLAY_NOTE_BASE__endif3
          SD4PLAY_NOTE_BASE__else3:
              // line 436
              rra
              ld C_hwl, A
              // line 437
              ld A, 0 + BASE_VOL_MIN_DW
              ld (addr_mod_vol_end_1+1), A
              // line 438
              ld A, 0 + dec_vol
              ld (addr_mod_vol_inc_0+0), A
              // line 439
              ld A, 0 + OPCODE_JR_NZ
          SD4PLAY_NOTE_BASE__endif3:

      SD4PLAY_NOTE_BASE__endif1:

      // line 442
sd4playNoteBase_initEnd:
      // line 443
      ld (addr_mod_start_0+0), A
      // line 444
      ld B_wlc, 1
      // line 446
      ld A, (VRAM_TEXT)
      ex AF, AF
      ld (MMIO_8253_CT0), A
      ex AF, AF
  SD4PLAY_NOTE_BASE__endif0:

endm
__endasm;
}











void SD4PLAY_MOD_BASE(
    int E_vol, int IXL_slc,
    int addr_mod_start_0, int addr_mod_vol_end_1, int addr_mod_vol_inc_0
)  __naked
{
__asm
SD4PLAY_MOD_BASE macro E_vol, IXL_slc, addr_mod_start_0, addr_mod_vol_end_1, addr_mod_vol_inc_0
local SD4PLAY_MOD_BASE__else0, SD4PLAY_MOD_BASE__endif0
  // line 467
  extern sd4playModBase_end
  // line 470
  ld A, IXL_slc
  and A, 1
  // line 471
addr_mod_start_0:
  jr nz, 0 + sd4playModBase_end
  // line 472
  ld A, E_vol
addr_mod_vol_end_1:
  cp A, 0
  // line 473
  jr z, SD4PLAY_MOD_BASE__endif0
addr_mod_vol_inc_0:
      inc E_vol
  SD4PLAY_MOD_BASE__endif0:

  // line 477
sd4playModBase_end:
endm
__endasm;
}





















void SD4PLAY_NOTE_CHORD2(
    int B_wl0, int C_wl1, int BC_wl01, int D_vol, int IYH_slc,
    int inc_vol, int dec_vol,
    int addr_mod_wl_start_0,  int addr_mod_wl0_1,     int addr_mod_wl1_1,
    int addr_mod_vol_start_0, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_beep_wl_1
)  __naked
{
__asm
SD4PLAY_NOTE_CHORD2 macro B_wl0, C_wl1, BC_wl01, D_vol, IYH_slc, inc_vol, dec_vol, addr_mod_wl_start_0, addr_mod_wl0_1, addr_mod_wl1_1, addr_mod_vol_start_0, addr_mod_vol_end_1, addr_mod_vol_inc_0, addr_beep_wl_1
local SD4PLAY_NOTE_CHORD2__else0, SD4PLAY_NOTE_CHORD2__endif0, SD4PLAY_NOTE_CHORD2__else1, SD4PLAY_NOTE_CHORD2__endif1, SD4PLAY_NOTE_CHORD2__else2, SD4PLAY_NOTE_CHORD2__endif2, SD4PLAY_NOTE_CHORD2__else3, SD4PLAY_NOTE_CHORD2__endif3
  // line 510
  extern sd4playNoteChord2_initDw, sd4playNoteChord2_initEnd
  // line 513
  dec IYH_slc
  jp nz, SD4PLAY_NOTE_CHORD2__endif0
      // line 514
      dec SP
      pop AF
      // line 517
      add A, A
      jr c, SD4PLAY_NOTE_CHORD2__endif1
          // line 518
          add A, A
          ld IYH_slc, A
          jr nc, SD4PLAY_NOTE_CHORD2__endif2
              // line 523
              pop BC_wl01
              // line 524
              ld A, B_wl0
              ld (addr_mod_wl0_1+1), A
              // line 525
              ld A, C_wl1
              ld (addr_mod_wl1_1+1), A
              // line 526
              ld D_vol, 0
              // line 527
              ld BC_wl01, 0 + (CHORD_VOL_MAX<<8)|inc_vol
              // line 528
              jp 0 + sd4playNoteChord2_initDw
          SD4PLAY_NOTE_CHORD2__endif2:

          // line 532
          xor A, A
          ld (addr_beep_wl_1+1), A
          // line 533
          ld C_wl1, A
          // line 534
          ld A, 0 + OPCODE_JR
          // line 535
          jp 0 + sd4playNoteChord2_initEnd
      SD4PLAY_NOTE_CHORD2__endif1:

      // line 537
      add A, A
      ld IYH_slc, A
      // line 538
      pop BC_wl01
      // line 539
      ld A, B_wl0
      ld (addr_mod_wl0_1+1), A
      // line 540
      ld A, C_wl1
      ld (addr_mod_wl1_1+1), A
      // line 541
      ld D_vol, 0 + CHORD_VOL_MAX
      // line 542
      jp nc, SD4PLAY_NOTE_CHORD2__else3
          // line 545
          ld A, 0 + OPCODE_JR
          ld (addr_mod_vol_start_0+0), A
          // line 546
          ld A, 0 + OPCODE_LD_A_N
      jp SD4PLAY_NOTE_CHORD2__endif3
      SD4PLAY_NOTE_CHORD2__else3:
          // line 550
          ld BC_wl01, 0 + (CHORD_VOL_MIN_DW<<8)|dec_vol
          // line 551
sd4playNoteChord2_initDw:
          // line 552
          ld A, (VRAM_TEXT)
          ex AF, AF
          ld (MMIO_8253_CT0), A
          ex AF, AF
          // line 553
          ld A, B_wl0
          ld (addr_mod_vol_end_1+1), A
          // line 554
          ld A, C_wl1
          ld (addr_mod_vol_inc_0+0), A
          // line 555
          ld A, 0 + OPCODE_JR_NZ
          ld (addr_mod_vol_start_0+0), A
          // line 556
          ld A, 0 + OPCODE_LD_A_N
      SD4PLAY_NOTE_CHORD2__endif3:

      // line 558
sd4playNoteChord2_initEnd:
      // line 559
      ld (addr_mod_wl_start_0+0), A
      // line 560
      ld B_wl0, 1
      // line 562
      ld A, (VRAM_TEXT)
      ex AF, AF
      ld (MMIO_8253_CT0), A
      ex AF, AF
  SD4PLAY_NOTE_CHORD2__endif0:

endm
__endasm;
}

















void SD4PLAY_MOD_CHORD2(
    int C_hwl, int D_vol, int IYH_slc,
    int addr_mod_wl_start_0,  int addr_mod_wl0_1,     int addr_mod_wl1_1,
    int addr_mod_vol_start_0, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_beep_wl_1
)  __naked
{
__asm
SD4PLAY_MOD_CHORD2 macro C_hwl, D_vol, IYH_slc, addr_mod_wl_start_0, addr_mod_wl0_1, addr_mod_wl1_1, addr_mod_vol_start_0, addr_mod_vol_end_1, addr_mod_vol_inc_0, addr_beep_wl_1
local SD4PLAY_MOD_CHORD2__else0, SD4PLAY_MOD_CHORD2__endif0, SD4PLAY_MOD_CHORD2__else1, SD4PLAY_MOD_CHORD2__endif1
  // line 591
  extern sd4playModChord_end
  // line 594
addr_mod_wl_start_0:
  jr 0 + sd4playModChord_end
  // line 597
  ld A, IYH_slc
  and A, 2
addr_mod_wl0_1:
  ld A, 0
  // line 598
  jr nz, SD4PLAY_MOD_CHORD2__endif0
addr_mod_wl1_1:
      ld A, 0
  SD4PLAY_MOD_CHORD2__endif0:

  // line 599
  ld (addr_beep_wl_1+1), A
  rra
  ld C_hwl, A
  // line 602
  ld A, IYH_slc
  and A, 1
  // line 603
addr_mod_vol_start_0:
  jr nz, 0 + sd4playModChord_end
  // line 604
  ld A, D_vol
addr_mod_vol_end_1:
  cp A, 0
  // line 605
  jr z, SD4PLAY_MOD_CHORD2__endif1
addr_mod_vol_inc_0:
      inc D_vol
  SD4PLAY_MOD_CHORD2__endif1:

  // line 608
sd4playModChord_end:
endm
__endasm;
}











void SD4PLAY_NOTE_DRUM(int IYL_sl, int L_tmp,
    int addr_drum_ct_1, int addr_drum_nr_1, int addr_drum_add_0)  __naked
{
__asm
SD4PLAY_NOTE_DRUM macro IYL_sl, L_tmp, addr_drum_ct_1, addr_drum_nr_1, addr_drum_add_0
local SD4PLAY_NOTE_DRUM__else0, SD4PLAY_NOTE_DRUM__endif0, SD4PLAY_NOTE_DRUM__else1, SD4PLAY_NOTE_DRUM__endif1, SD4PLAY_NOTE_DRUM__else2, SD4PLAY_NOTE_DRUM__endif2, SD4PLAY_NOTE_DRUM__else3, SD4PLAY_NOTE_DRUM__endif3
  // line 626
  extern sd4PlayModDrum_ct_1, sd4PlayModDrum_end
  // line 628
  dec IYL_sl
  jp nz, SD4PLAY_NOTE_DRUM__endif0
      // line 629
      dec SP
      pop AF
      // line 630
      ld L_tmp, A
      and A, 252
      ld IYL_sl, A
      // line 631
      xor A, L_tmp
      // line 632
      jr z, SD4PLAY_NOTE_DRUM__endif1
          // line 633
          add A, 0 + (ADDR_SD4_DRUM/256-1)
          ld (addr_drum_nr_1+1), A
          // line 634
          ld A, (VRAM_TEXT)
          ex AF, AF
          ld (MMIO_8253_CT0), A
          ex AF, AF
          // line 635
          ld A, 0 + OPCODE_RES_0_L
          ld (addr_drum_ct_1+1), A
          // line 636
          ld A, 0 + OPCODE_OR_HL
          ld (addr_drum_add_0+0), A
          // line 637
          ld A, 2
          ld (sd4PlayModDrum_ct_1+1), A
      SD4PLAY_NOTE_DRUM__endif1:

      // line 640
      jp 0 + sd4PlayModDrum_end
  SD4PLAY_NOTE_DRUM__endif0:

  // line 643
sd4PlayModDrum_ct_1:
  ld A, 0
  or A, A
  jr z, SD4PLAY_NOTE_DRUM__endif2
      // line 644
      dec A
      ld (sd4PlayModDrum_ct_1+1), A
      // line 645
      jr z, SD4PLAY_NOTE_DRUM__else3
          // line 646
          ld A, 0 + OPCODE_SET_0_L
          ld (addr_drum_ct_1+1), A
      jp SD4PLAY_NOTE_DRUM__endif3
      SD4PLAY_NOTE_DRUM__else3:
          // line 648
          xor A, A
          ld (addr_drum_add_0+0), A
      SD4PLAY_NOTE_DRUM__endif3:

  SD4PLAY_NOTE_DRUM__endif2:

  // line 652
sd4PlayModDrum_end:
endm
__endasm;
}











void SD4PLAY_BEEPER_WAVE_R(int H_wlc, int L_hwl, int D_vol, int addr_wl_1)  __naked
{
__asm
SD4PLAY_BEEPER_WAVE_R macro H_wlc, L_hwl, D_vol, addr_wl_1
local SD4PLAY_BEEPER_WAVE_R__else0, SD4PLAY_BEEPER_WAVE_R__endif0, SD4PLAY_BEEPER_WAVE_R__else1, SD4PLAY_BEEPER_WAVE_R__endif1
  // line 671
  dec H_wlc
  jp nz, SD4PLAY_BEEPER_WAVE_R__else0
      // line 672
addr_wl_1:
      ld H_wlc, 0
  jp SD4PLAY_BEEPER_WAVE_R__endif0
  SD4PLAY_BEEPER_WAVE_R__else0:
      // line 675
      ld A, H_wlc
      cp A, L_hwl
      // line 676
      jr nc, SD4PLAY_BEEPER_WAVE_R__endif1
          ex AF, AF
          add A, D_vol
          ex AF, AF
      SD4PLAY_BEEPER_WAVE_R__endif1:

  SD4PLAY_BEEPER_WAVE_R__endif0:

endm
__endasm;
}










void SD4PLAY_BEEPER_WAVE_BR(int B_wl, int C_hwl, int E_vol, int addr_wl_1)  __naked
{
__asm
SD4PLAY_BEEPER_WAVE_BR macro B_wl, C_hwl, E_vol, addr_wl_1
local SD4PLAY_BEEPER_WAVE_BR__else0, SD4PLAY_BEEPER_WAVE_BR__endif0
  // line 694
  local sd4play_beeperWaveBr
  local sd4play_beeperWaveBrEnd
  // line 695
  extern sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd
  // line 698
  djnz B,0 + sd4play_beeperWaveBr
    // line 699
addr_wl_1:
    ld B_wl, 0
    jr 0 + sd4play_beeperWaveBrEnd
  // line 701
sd4play_beeperWaveBr:
    // line 703
    ld A, B_wl
    cp A, C_hwl
    // line 704
    jr nc, SD4PLAY_BEEPER_WAVE_BR__endif0
        ex AF, AF
        add A, E_vol
        ex AF, AF
    SD4PLAY_BEEPER_WAVE_BR__endif0:

  // line 706
sd4play_beeperWaveBrEnd:
endm
__endasm;
}










void SD4PLAY_BEEPER_WAVE_B(int B_wlc, int C_hwl, int addr_vol_1, int addr_wl_1)  __naked
{
__asm
SD4PLAY_BEEPER_WAVE_B macro B_wlc, C_hwl, addr_vol_1, addr_wl_1
local SD4PLAY_BEEPER_WAVE_B__else0, SD4PLAY_BEEPER_WAVE_B__endif0
  // line 723
  local sd4play_beeperWaveBr
  local sd4play_beeperWaveBrEnd
  // line 724
  extern sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd
  // line 727
  djnz B,0 + sd4play_beeperWaveBr
    // line 728
addr_wl_1:
    ld B_wlc, 0
    jr 0 + sd4play_beeperWaveBrEnd
  // line 730
sd4play_beeperWaveBr:
    // line 732
    ld A, B_wlc
    cp A, C_hwl
    // line 733
    jr nc, SD4PLAY_BEEPER_WAVE_B__endif0
        ex AF, AF
addr_vol_1:
        add A, 0
        ex AF, AF
    SD4PLAY_BEEPER_WAVE_B__endif0:

  // line 735
sd4play_beeperWaveBrEnd:
endm
__endasm;
}














void SD4PLAY_BEEPER_DRUM(
    int H_wav_h, int L_wav_l, int HL_wav_hl, int E_ct,
    int addr_drum_ct_1, int addr_drum_nr_1, int addr_drum_add_0)  __naked
{
__asm
SD4PLAY_BEEPER_DRUM macro H_wav_h, L_wav_l, HL_wav_hl, E_ct, addr_drum_ct_1, addr_drum_nr_1, addr_drum_add_0
  // line 758
  ld L_wav_l, E_ct
addr_drum_ct_1:
  set 0, L_wav_l
  // line 759
addr_drum_nr_1:
  ld H_wav_h, 0 + (ADDR_SD4_DRUM-1)/256+1
  // line 760
  ex AF, AF
addr_drum_add_0:
  nop
endm
__endasm;
}




u8 sd4play(u32 param) 
{
__asm
  // line 772
  extern sd4play_tempo_1
  // line 773
  extern sd4play_waitUntilKeyOff2
  // line 774
  extern sd4play_vblk0, sd4play_vblk1
  // line 775
  extern sd4play_setVols, sd4play_loop, sd4play_beeperLoop
  // line 776
  extern sd4play_rep_1, sd4play_leadSlide_0
  // line 777
  extern sd4play_modLead_start
  // line 778
  extern sd4play_modLead_vol, sd4play_modLead_volSpeed_1, sd4play_modLead_vol_1, sd4play_modLead_volEnd_1, sd4play_modLead_volInc_0
  // line 779
  extern sd4play_modLead_wl, sd4play_modLead_wlSpeed_1, sd4play_modLead_wlEnd_1, sd4play_modLead_wlInc_0
  // line 780
  extern sd4play_modLead_end
  // line 781
  extern sd4play_modBase_start, sd4play_modBase_volEnd_1, sd4play_modBase_volInc_0
  // line 782
  extern sd4play_modChord_wlStart, sd4play_modChord_wl0, sd4play_modChord_wl1
  // line 783
  extern sd4play_modChord_volStart, sd4play_modChord_volEnd, sd4play_modChord_volInc
  // line 784
  extern sd4play_beeper0_wl_1
  // line 785
  extern sd4play_beeper1_wl_1
  // line 786
  extern sd4play_beeper2_wl_1
  // line 787
  extern sd4play_beeper3_drumCt, sd4play_beeper3_drumNr, sd4play_beeper3_drumAdd
  // line 788
  extern sd4play_end
  // line 789
  extern sd4play_restoreSP
  // line 790
  extern tmp
  // line 793
  push IX
  // line 794
  BANKH_VRAM_MMIO 
  // line 797
  ld A, 249
  ld (MMIO_8255_PORTA), A
  // line 798
  sd4play__loop_top0: // do {
      // line 799
      ld A, (MMIO_8255_PORTB)
      cpl
      and A, 0 + KEY9_F1_MASK|KEY9_F2_MASK|KEY9_F3_MASK|KEY9_F4_MASK|KEY9_F5_MASK
  sd4play__loop_end0:
  jr nz, sd4play__loop_top0 // } while (nz_jr)
  sd4play__loop_exit0: // loop exit

  // line 803
  bit 0, D
  jr nz, sd4play__endif1
      // line 804
      ld A, 250
      ld (MMIO_8255_PORTA), A
  sd4play__endif1:

  // line 808
  ld A, E
  ld (sd4play_tempo_1+1), A
  // line 811
  ld A, 0 + MMIO_8253_CTRL_CT0(MMIO_8253_CTRL_RL_L_MASK,MMIO_8253_CTRL_MODE0_MASK)
  // line 812
  ld (MMIO_8253_CTRL), A
  // line 814
#if  0   
L00:
  ld A, (VRAM_TEXT)
  // line 816
  ld A, 0 + 56/2
  ld (MMIO_8253_CT0), A
  // line 817
  ex (SP), HL
  ex (SP), HL
  ex (SP), HL
  ex (SP), HL
  // line 818
  jr 0 + L00
  // line 819
#endif 
  // line 833
  ld (sd4play_restoreSP+1), SP
  ld SP, HL
  // line 834
  xor A, A
  // line 835
  ld (sd4PlayModDrum_ct_1+1), A
  // line 836
  ld (sd4play_beeper3_drumAdd+0), A
  // line 837
  ld (sd4play_modLead_volSpeed_1+1), A
  // line 839
  inc A
  // line 840
  ld IXH, A
  ld IXL, A
  ld IYL, A
  ld IYH, A
  // line 841
  ld HL, 0 + ADDR_SD4_REP+0xff
  ld (sd4play_rep_1+1), HL
  // line 842
  ld A, 0 + OPCODE_JR
  ld (sd4play_leadSlide_0+0), A
  // line 845
  xor A, A
  ld HL, 0 + MMIO_8255_PORTC
  // line 846
sd4play_vblk1:
  or A, (HL)
  jp p, 0 + sd4play_vblk1
  // line 847
  ld A, (VRAM_TEXT)
  // line 848
  xor A, A
  ex AF, AF
  // line 852
sd4play_loop:
  // line 853
  ld A, (VRAM_TEXT)
  ex AF, AF
  ld (MMIO_8253_CT0), A
  ex AF, AF
  // line 857
  SD4PLAY_NOTE_LEAD H, L, HL, D, IXH, OPCODE_INC_D, OPCODE_DEC_D, sd4play_rep_1, sd4play_leadSlide_0, sd4play_modLead_start, sd4play_modLead_vol, sd4play_modLead_volSpeed_1, sd4play_modLead_volEnd_1, sd4play_modLead_volInc_0, sd4play_modLead_wl, sd4play_modLead_wlSpeed_1, sd4play_modLead_wlEnd_1, sd4play_modLead_wlInc_0, sd4play_beeper0_wl_1, sd4play_modLead_end, sd4play_end
  // line 864
  SD4PLAY_NOTE_BASE B, C, BC, E, IXL, OPCODE_INC_E, OPCODE_DEC_E, sd4play_modBase_start, sd4play_modBase_volEnd_1, sd4play_modBase_volInc_0, sd4play_beeper1_wl_1
  // line 867
  SD4PLAY_MOD_LEAD L, D, IXH, sd4play_modLead_start, sd4play_modLead_vol, sd4play_modLead_volSpeed_1, sd4play_modLead_volEnd_1, sd4play_modLead_volInc_0, sd4play_modLead_wl, sd4play_modLead_wlSpeed_1, sd4play_modLead_wlEnd_1, sd4play_modLead_wlInc_0, sd4play_beeper0_wl_1, sd4play_modLead_end
  // line 873
  SD4PLAY_MOD_BASE E, IXL, sd4play_modBase_start, sd4play_modBase_volEnd_1, sd4play_modBase_volInc_0
  // line 875
  ld A, (VRAM_TEXT)
  ex AF, AF
  ld (MMIO_8253_CT0), A
  ex AF, AF
  // line 876
  exx
  // line 877
  SD4PLAY_NOTE_CHORD2 B, C, BC, D, IYH, OPCODE_INC_D, OPCODE_DEC_D, sd4play_modChord_wlStart, sd4play_modChord_wl0, sd4play_modChord_wl1, sd4play_modChord_volStart, sd4play_modChord_volEnd, sd4play_modChord_volInc, sd4play_beeper2_wl_1
  // line 881
  SD4PLAY_MOD_CHORD2 C, D, IYH, sd4play_modChord_wlStart, sd4play_modChord_wl0, sd4play_modChord_wl1, sd4play_modChord_volStart, sd4play_modChord_volEnd, sd4play_modChord_volInc, sd4play_beeper2_wl_1
  // line 885
  SD4PLAY_NOTE_DRUM IYL, L, sd4play_beeper3_drumCt, sd4play_beeper3_drumNr, sd4play_beeper3_drumAdd
  // line 887
sd4play_tempo_1:
  ld E, 0 + TEMPO
  // line 888
  ld A, (VRAM_TEXT)
  ex AF, AF
  ld (MMIO_8253_CT0), A
  // line 891
sd4play_beeperLoop:
  // line 892
    // line 893
    xor A, A
    ex AF, AF
    // line 894
    exx
    // line 896
    SD4PLAY_BEEPER_WAVE_R H, L, D, sd4play_beeper0_wl_1
    // line 897
    SD4PLAY_BEEPER_WAVE_BR B, C, E, sd4play_beeper1_wl_1
    // line 898
    exx
    // line 899
    SD4PLAY_BEEPER_WAVE_BR B, C, D, sd4play_beeper2_wl_1
    // line 900
    SD4PLAY_BEEPER_DRUM H, L, HL, E, sd4play_beeper3_drumCt, sd4play_beeper3_drumNr, sd4play_beeper3_drumAdd
    // line 905
    ld H, 0 + ADDR_SD4_VTAB/256
    ld L, A
    // line 906
    ld A, (VRAM_TEXT)
    ld A, (HL)
    ld (MMIO_8253_CT0), A
    // line 907
    inc E
    jp nz, 0 + sd4play_beeperLoop
  // line 910
  exx
  // line 911
  ex AF, AF
  // line 913
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F1_MASK|KEY9_F2_MASK|KEY9_F3_MASK|KEY9_F4_MASK|KEY9_F5_MASK
  jp z, 0 + sd4play_loop
  // line 914
  jr 0 + sd4play_waitUntilKeyOff2
  // line 917
sd4play_end:
  // line 918
  xor A, A
  // line 920
sd4play_waitUntilKeyOff2:
  // line 921
  ex AF, AF
    // line 922
    sd4play__loop_top2: // do {
        // line 923
        ld A, (MMIO_8255_PORTB)
        cpl
        and A, 0 + KEY9_F1_MASK|KEY9_F2_MASK|KEY9_F3_MASK|KEY9_F4_MASK|KEY9_F5_MASK
    sd4play__loop_end2:
    jr nz, sd4play__loop_top2 // } while (nz_jr)
    sd4play__loop_exit2: // loop exit

    // line 927
    ld A, 0 + MMIO_8253_CT0_MODE3
    ld (MMIO_8253_CTRL), A
  ex AF, AF
  // line 930
  BANKH_RAM 
  // line 932
sd4play_restoreSP:
  ld SP, 0
  // line 933
  pop IX
__endasm;
}

#pragma restore
