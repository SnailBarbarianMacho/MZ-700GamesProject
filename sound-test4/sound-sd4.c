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

void LEAD_ECHO_DELAY(void)  __naked
{
__asm
LEAD_ECHO_DELAY macro void
  // line 40
  dec A
endm
__endasm;
}



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
  // line 117
  extern _SD4_DATA_
  // line 118
  extern sd4Init_dataLoop
  // line 121
  ld HL, 0 + _SD4_DATA_
  // line 122
  ld DE, 0 + ADDR_SD4_VTAB
  // line 123
  ld BC, 0 + SZ_SD4_VTAB + SZ_SD4_TAB
  // line 124
  ldir
  // line 126
  ld DE, 0 + ADDR_SD4_DRUM
  // line 127
  ld BC, 0 + SZ_SD4_DRUM
  // line 128
  ldir
  // line 131
  BANKH_VRAM_MMIO C
  // line 132
  ld HL, 0 + MMIO_ETC
  // line 133
  ld (HL), 0 + MMIO_ETC_GATE_MASK
  // line 134
  dec L
  // line 135
  ld (HL), 0 + MMIO_8253_CT0_MODE3
  // line 136
  BANKH_RAM C
__endasm;
}






























void SD4PLAY_NOTE_LEAD(
    int reg_wlc, int reg_hwl, int reg_wlchwl, int reg_vol, int reg_slc,
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
SD4PLAY_NOTE_LEAD macro reg_wlc, reg_hwl, reg_wlchwl, reg_vol, reg_slc, inc_vol, dec_vol, addr_rep_1, addr_slide_0, addr_mod_start_1, addr_mod_vol, addr_mod_vol_speed_1, addr_mod_vol_end_1, addr_mod_vol_inc_0, addr_mod_wl, addr_mod_wl_speed_1, addr_mod_wl_end_1, addr_mod_wl_inc_0, addr_wl_1, addr_mod_end, addr_end
local SD4PLAY_NOTE_LEAD__0, SD4PLAY_NOTE_LEAD__1, SD4PLAY_NOTE_LEAD__2, SD4PLAY_NOTE_LEAD__3, SD4PLAY_NOTE_LEAD__4, SD4PLAY_NOTE_LEAD__5, SD4PLAY_NOTE_LEAD__6, SD4PLAY_NOTE_LEAD__7, SD4PLAY_NOTE_LEAD__8, SD4PLAY_NOTE_LEAD__9, SD4PLAY_NOTE_LEAD__10, SD4PLAY_NOTE_LEAD__11, SD4PLAY_NOTE_LEAD__12, SD4PLAY_NOTE_LEAD__13, SD4PLAY_NOTE_LEAD__14
  // line 181
  extern sd4playNoteLead_pop0, sd4playNoteLead_pop, sd4playNoteLead_rep_2, sd4playNoteLead_initDw, sd4playNoteLead_initDw_1, sd4playNoteLead_initEnd, sd4playNoteLead_initEndSlur
  // line 188
  dec reg_slc
  jp nz, SD4PLAY_NOTE_LEAD__0 // if (z) {
      // line 189
sd4playNoteLead_pop:
      // line 190
      pop reg_wlchwl
      // line 193
      ld A, reg_hwl
      add A, A
      jr c, SD4PLAY_NOTE_LEAD__1 // if (nc_jr) {
          // line 194
          add A, A
          ld reg_slc, A
          jr nc, SD4PLAY_NOTE_LEAD__2 // if (c_jr) {
              // line 197
              cp A, 0 + SD4_EXCEPT_LEN
              jr nz, SD4PLAY_NOTE_LEAD__3 // if (z_jr) {
                  // line 198
addr_rep_1:
                  ld HL, 0 + 0x0000
                  dec (HL)
                  jr nz, SD4PLAY_NOTE_LEAD__4 // if (z_jr) {
                      // line 199
                      dec L
                      dec L
                      dec L
                      ld (addr_rep_1 + 1), HL
                      // line 200
                      dec SP
                      // line 201
                      jp sd4playNoteLead_pop0
                  SD4PLAY_NOTE_LEAD__4: // } endif 
                  // line 204
                  dec L
                  ld A, (HL)
                  dec L
                  ld L, (HL)
                  ld H, A
                  ld SP, HL
                  // line 205
                  jp sd4playNoteLead_pop0
              SD4PLAY_NOTE_LEAD__3: // } endif 
              // line 209
                // line 210
                cp A, 0 + SD4_EXCEPT_LEN + 4
                jp z, 0 + addr_end
              // line 214
                // line 215
                ld A, (VRAM_TEXT)
                ex AF, AF
                ld (MMIO_8253_CT0), A
                ex AF, AF
                // line 216
                ld A, reg_wlc
                ld (addr_wl_1 + 1), A
                // line 217
                ld reg_wlchwl, 0 + addr_mod_vol
                ld (addr_mod_start_1 + 1), reg_wlchwl
                // line 218
                ld reg_wlchwl, 0 + (LEAD_VOL_MAX << 8) | inc_vol
                // line 219
                ld reg_vol, 0 + LEAD_VOL_MIN_UP
                // line 220
                jp sd4playNoteLead_initDw
          SD4PLAY_NOTE_LEAD__2: // } endif 
          // line 225
          cp A, 0 + SD4_EXCEPT_LEN
          jp c, SD4PLAY_NOTE_LEAD__5 // if (nc) {
              // line 226
              cp A, 0 + SD4_EXCEPT_LEN + 4 * 7
              jp nc, SD4PLAY_NOTE_LEAD__6 // if (c) {
                  // line 227
                  dec SP
                  // line 228
                  sub A, 0 + SD4_EXCEPT_LEN - 8
                  // line 229
                  rrca
                  rrca
                  // line 230
                  ld HL, (addr_rep_1 + 1)
                  // line 231
                  inc L
                  ld (sd4playNoteLead_rep_2 + 2), HL
sd4playNoteLead_rep_2:
                  ld (0x0000), SP
                  // line 232
                  inc L
                  inc L
                  ld (HL), A
                  // line 233
                  ld (addr_rep_1 + 1), HL
                  // line 234
                  jp sd4playNoteLead_pop0
              SD4PLAY_NOTE_LEAD__6: // } endif 
          SD4PLAY_NOTE_LEAD__5: // } endif 
          // line 239
            // line 240
            dec SP
            // line 241
            xor A, A
            ld (addr_wl_1 + 1), A
            // line 242
            ld reg_wlchwl, 0 + addr_mod_end
            ld (addr_mod_start_1 + 1), reg_wlchwl
            // line 243
            ld reg_hwl, A
            // line 244
            jp sd4playNoteLead_initEndSlur
      SD4PLAY_NOTE_LEAD__1: // } endif 
      // line 247
      add A, A
      ld reg_slc, A
      jr nc, SD4PLAY_NOTE_LEAD__7 // if (c_jr) {
          // line 248
          cp A, 0 + SD4_EXCEPT_LEN
          jp c, SD4PLAY_NOTE_LEAD__8 // if (nc) {
              // line 249
              jr nz, SD4PLAY_NOTE_LEAD__9 // if (z_jr) {
                  // line 252
                  ld A, 0 + OPCODE_JR
                  ld (addr_slide_0 + 0), A
                  // line 253
                  dec SP
                  // line 254
                  jp sd4playNoteLead_pop0
              SD4PLAY_NOTE_LEAD__9: // } endif 
              // line 257
              cp A, 0 + SD4_EXCEPT_LEN + 4 * 4
              jp nc, SD4PLAY_NOTE_LEAD__10 // if (c) {
                  // line 259
                  sub A, 0 + SD4_EXCEPT_LEN - (SZ_SD4_VTAB + 1)
                  // line 260
                  ld reg_wlc, 0 + ADDR_SD4_VTAB / 256
                  ld reg_hwl, A
                  ld A, (reg_wlchwl)
                  // line 261
                  ld (addr_mod_wl_speed_1 + 1), A
                  // line 262
                  ld A, 0 + OPCODE_LD_A_N
                  ld (addr_slide_0 + 0), A
                  // line 263
                  dec SP
                  // line 264
                  jp sd4playNoteLead_pop0
              SD4PLAY_NOTE_LEAD__10: // } endif 
          SD4PLAY_NOTE_LEAD__8: // } endif 
          // line 268
          ld A, (VRAM_TEXT)
          ex AF, AF
          ld (MMIO_8253_CT0), A
          ex AF, AF
          // line 269
addr_slide_0:
          jr nz, SD4PLAY_NOTE_LEAD__11 // if (z_jr) {
              // line 272
              ld A, reg_wlc
              ld (addr_mod_wl_end_1 + 1), A
              // line 274
              ld A, (addr_wl_1 + 1)
              cp A, reg_wlc
              ld A, 0 + OPCODE_INC_A
              jr c, SD4PLAY_NOTE_LEAD__12 // if (nc_jr) {
                  inc A
              SD4PLAY_NOTE_LEAD__12: // } endif 
              // line 275
              ld (addr_mod_wl_inc_0 + 0), A
              // line 276
              ld reg_wlchwl, 0 + addr_mod_wl
              ld (addr_mod_start_1 + 1), reg_wlchwl
              // line 277
              ld A, (addr_wl_1 + 1)
              srl A
              ld reg_hwl, A
              // line 278
              jp sd4playNoteLead_initEndSlur
          SD4PLAY_NOTE_LEAD__11: // } endif 
          // line 282
            // line 283
            ld A, reg_wlc
            ld (addr_wl_1  + 1), A
            // line 284
            ld reg_wlchwl, 0 + addr_mod_end
            ld (addr_mod_start_1 + 1), reg_wlchwl
            // line 285
            srl A
            ld reg_hwl, A
            // line 286
            ld reg_vol, 0 + LEAD_VOL_MAX
            // line 287
            jp sd4playNoteLead_initEnd
      SD4PLAY_NOTE_LEAD__7: // } endif 
      // line 291
      cp A, 0 + SD4_EXCEPT_LEN
      jr c, SD4PLAY_NOTE_LEAD__13 // if (nc_jr) {
          // line 292
          cp A, 0 + SD4_EXCEPT_LEN + 4 * 4
          jp nc, SD4PLAY_NOTE_LEAD__14 // if (c) {
              // line 295
              sub A, 0 + SD4_EXCEPT_LEN - SZ_SD4_VTAB
              // line 296
              ld reg_wlc, 0 + ADDR_SD4_VTAB / 256
              ld reg_hwl, A
              ld A, (reg_wlchwl)
              // line 297
              ld (addr_mod_vol_speed_1 + 1), A
              // line 298
              dec SP
              // line 299
sd4playNoteLead_pop0:
              // line 300
              ld A, (VRAM_TEXT)
              ex AF, AF
              ld (MMIO_8253_CT0), A
              ex AF, AF
              // line 301
              jp sd4playNoteLead_pop
          SD4PLAY_NOTE_LEAD__14: // } endif 
      SD4PLAY_NOTE_LEAD__13: // } endif 
      // line 308
        // line 309
        ld A, (VRAM_TEXT)
        ex AF, AF
        ld (MMIO_8253_CT0), A
        ex AF, AF
        // line 310
        ld A, reg_wlc
        ld (addr_wl_1 + 1), A
        // line 311
        ld reg_wlchwl, 0 + addr_mod_vol
        ld (addr_mod_start_1 + 1), reg_wlchwl
        // line 312
        ld reg_wlchwl, 0 + (LEAD_VOL_MIN_DW << 8) | dec_vol
        // line 313
        ld reg_vol, 0 + LEAD_VOL_MAX
      // line 315
sd4playNoteLead_initDw:
      // line 316
      srl A
      ld (sd4playNoteLead_initDw_1 + 1), A
      // line 317
      ld A, reg_wlc
      ld (addr_mod_vol_end_1 + 1), A
      // line 318
      ld A, reg_hwl
      ld (addr_mod_vol_inc_0 + 0), A
      // line 319
sd4playNoteLead_initDw_1:
      ld reg_hwl, 0 + 0x00
      // line 321
sd4playNoteLead_initEnd:
      // line 322
      ld reg_wlc, 0 + 1
      // line 324
sd4playNoteLead_initEndSlur:
      // line 325
      ld A, (VRAM_TEXT)
      ex AF, AF
      ld (MMIO_8253_CT0), A
      ex AF, AF
  SD4PLAY_NOTE_LEAD__0: // } endif 
endm
__endasm;
}




















void SD4PLAY_MOD_LEAD(
    int reg_hwl, int reg_vol, int reg_slc,
    int addr_mod_start_1,
    int addr_mod_vol, int addr_mod_vol_speed_1, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_mod_wl,  int addr_mod_wl_speed_1,  int addr_mod_wl_end_1,  int addr_mod_wl_inc_0,
    int addr_wl_1,
    int addr_mod_end
)  __naked
{
__asm
SD4PLAY_MOD_LEAD macro reg_hwl, reg_vol, reg_slc, addr_mod_start_1, addr_mod_vol, addr_mod_vol_speed_1, addr_mod_vol_end_1, addr_mod_vol_inc_0, addr_mod_wl, addr_mod_wl_speed_1, addr_mod_wl_end_1, addr_mod_wl_inc_0, addr_wl_1, addr_mod_end
local SD4PLAY_MOD_LEAD__0, SD4PLAY_MOD_LEAD__1
  // line 360
addr_mod_start_1:
  jp 0 + addr_mod_end
  // line 363
addr_mod_vol:
  // line 364
  ld A, reg_slc
addr_mod_vol_speed_1:
  and A, 0 + 0x00
  jr nz, SD4PLAY_MOD_LEAD__0 // if (z_jr) {
      // line 365
      ld A, reg_vol
addr_mod_vol_end_1:
      cp A, 0 + 0x00
      // line 366
      jr z, SD4PLAY_MOD_LEAD__1 // if (nz_jr) {
addr_mod_vol_inc_0:
          inc reg_vol
      SD4PLAY_MOD_LEAD__1: // } endif 
  SD4PLAY_MOD_LEAD__0: // } endif 
  // line 368
  jp addr_mod_end
  // line 371
addr_mod_wl:
  // line 372
  ld A, reg_slc
addr_mod_wl_speed_1:
  and A, 0 + 0x00
  jr nz, 0 + addr_mod_end
    // line 373
    ld A, (addr_wl_1 + 1)
addr_mod_wl_end_1:
    cp A, 0 + 0x00
    // line 374
    jr z, 0 + addr_mod_end
addr_mod_wl_inc_0:
    inc A
    ld (addr_wl_1 + 1), A
    // line 375
    srl A
    ld reg_hwl, A
  // line 380
addr_mod_end:
endm
__endasm;
}

















void SD4PLAY_NOTE_BASE(
    int reg_wlc, int reg_hwl, int reg_wlwl, int reg_vol, int reg_slc,
    int inc_vol, int dec_vol,
    int addr_mod_start_0, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_wl_1
)  __naked
{
__asm
SD4PLAY_NOTE_BASE macro reg_wlc, reg_hwl, reg_wlwl, reg_vol, reg_slc, inc_vol, dec_vol, addr_mod_start_0, addr_mod_vol_end_1, addr_mod_vol_inc_0, addr_wl_1
local SD4PLAY_NOTE_BASE__0, SD4PLAY_NOTE_BASE__1, SD4PLAY_NOTE_BASE__2, SD4PLAY_NOTE_BASE__3, SD4PLAY_NOTE_BASE__4, SD4PLAY_NOTE_BASE__5
  // line 408
  extern sd4playNoteBase_fl, sd4playNoteBase_initEnd
  // line 411
  dec reg_slc
  jr nz, SD4PLAY_NOTE_BASE__0 // if (z_jr) {
      // line 412
      pop reg_wlwl
      // line 415
      ld A, reg_hwl
      add A, A
      jr c, SD4PLAY_NOTE_BASE__3 // if (nc_jr) {
          // line 416
          add A, A
          ld reg_slc, A
          jr nc, SD4PLAY_NOTE_BASE__2 // if (c_jr) {
              // line 419
              ld A, reg_wlc
              ld (addr_wl_1 + 1), A
              // line 420
              srl A
              ld reg_hwl, A
              // line 421
              ld reg_vol, 0 + 0
              // line 422
              ld A, 0 + BASE_VOL_MAX
              ld (addr_mod_vol_end_1 + 1), A
              // line 423
              ld A, 0 + inc_vol
              ld (addr_mod_vol_inc_0 + 0), A
              // line 424
              ld A, 0 + OPCODE_JR_NZ
              // line 425
              jp sd4playNoteBase_initEnd
          SD4PLAY_NOTE_BASE__2: // } endif 
          // line 429
          dec SP
          // line 430
          xor A, A
          ld (addr_wl_1 + 1), A
          // line 431
          ld reg_hwl, A
          // line 432
          ld A, 0 + OPCODE_JR
      jp SD4PLAY_NOTE_BASE__1
      SD4PLAY_NOTE_BASE__3: // } else {
          // line 434
          add A, A
          ld reg_slc, A
          // line 435
          ld A, reg_wlc
          ld (addr_wl_1 + 1), A
          // line 436
          ld reg_vol, 0 + BASE_VOL_MAX
          // line 438
          jr nc, SD4PLAY_NOTE_BASE__5 // if (c_jr) {
              // line 441
              srl A
              ld reg_hwl, A
              // line 442
              ld A, 0 + OPCODE_JR
          jp SD4PLAY_NOTE_BASE__4
          SD4PLAY_NOTE_BASE__5: // } else {
              // line 446
              rra
              ld reg_hwl, A
              // line 447
              ld A, 0 + BASE_VOL_MIN_DW
              ld (addr_mod_vol_end_1 + 1), A
              // line 448
              ld A, 0 + dec_vol
              ld (addr_mod_vol_inc_0 + 0), A
              // line 449
              ld A, 0 + OPCODE_JR_NZ
SD4PLAY_NOTE_BASE__4: // } endif
SD4PLAY_NOTE_BASE__1: // } endif
      // line 452
sd4playNoteBase_initEnd:
      // line 453
      ld (addr_mod_start_0 + 0), A
      // line 454
      ld reg_wlc, 0 + 1
      // line 456
      ld A, (VRAM_TEXT)
      ex AF, AF
      ld (MMIO_8253_CT0), A
      ex AF, AF
  SD4PLAY_NOTE_BASE__0: // } endif 
endm
__endasm;
}











void SD4PLAY_MOD_BASE(
    int reg_vol, int reg_slc,
    int addr_mod_start_0, int addr_mod_vol_end_1, int addr_mod_vol_inc_0
)  __naked
{
__asm
SD4PLAY_MOD_BASE macro reg_vol, reg_slc, addr_mod_start_0, addr_mod_vol_end_1, addr_mod_vol_inc_0
local SD4PLAY_MOD_BASE__0
  // line 477
  extern sd4playModBase_end
  // line 480
  ld A, reg_slc
  and A, 0 + 0x01
  // line 481
addr_mod_start_0:
  jr nz, 0 + sd4playModBase_end
  // line 482
  ld A, reg_vol
addr_mod_vol_end_1:
  cp A, 0 + 0x00
  // line 483
  jr z, SD4PLAY_MOD_BASE__0 // if (nz_jr) {
addr_mod_vol_inc_0:
      inc reg_vol
  SD4PLAY_MOD_BASE__0: // } endif 
  // line 487
sd4playModBase_end:
endm
__endasm;
}





















void SD4PLAY_NOTE_CHORD2(
    int reg_wl0, int reg_wl1, int reg_wl01, int reg_vol, int reg_slc,
    int inc_vol, int dec_vol,
    int addr_mod_wl_start_0,  int addr_mod_wl0_1,     int addr_mod_wl1_1,
    int addr_mod_vol_start_0, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_beep_wl_1
)  __naked
{
__asm
SD4PLAY_NOTE_CHORD2 macro reg_wl0, reg_wl1, reg_wl01, reg_vol, reg_slc, inc_vol, dec_vol, addr_mod_wl_start_0, addr_mod_wl0_1, addr_mod_wl1_1, addr_mod_vol_start_0, addr_mod_vol_end_1, addr_mod_vol_inc_0, addr_beep_wl_1
local SD4PLAY_NOTE_CHORD2__0, SD4PLAY_NOTE_CHORD2__1, SD4PLAY_NOTE_CHORD2__2, SD4PLAY_NOTE_CHORD2__3, SD4PLAY_NOTE_CHORD2__4
  // line 520
  extern sd4playNoteChord2_initDw, sd4playNoteChord2_initEnd
  // line 523
  dec reg_slc
  jp nz, SD4PLAY_NOTE_CHORD2__0 // if (z) {
      // line 524
      dec SP
      pop AF
      // line 527
      add A, A
      jr c, SD4PLAY_NOTE_CHORD2__1 // if (nc_jr) {
          // line 528
          add A, A
          ld reg_slc, A
          jr nc, SD4PLAY_NOTE_CHORD2__2 // if (c_jr) {
              // line 533
              pop reg_wl01
              // line 534
              ld A, reg_wl0
              ld (addr_mod_wl0_1 + 1), A
              // line 535
              ld A, reg_wl1
              ld (addr_mod_wl1_1 + 1), A
              // line 536
              ld reg_vol, 0 + 0
              // line 537
              ld reg_wl01, 0 + (CHORD_VOL_MAX << 8) | inc_vol
              // line 538
              jp sd4playNoteChord2_initDw
          SD4PLAY_NOTE_CHORD2__2: // } endif 
          // line 542
          xor A, A
          ld (addr_beep_wl_1 + 1), A
          // line 543
          ld reg_wl1, A
          // line 544
          ld A, 0 + OPCODE_JR
          // line 545
          jp sd4playNoteChord2_initEnd
      SD4PLAY_NOTE_CHORD2__1: // } endif 
      // line 547
      add A, A
      ld reg_slc, A
      // line 548
      pop reg_wl01
      // line 549
      ld A, reg_wl0
      ld (addr_mod_wl0_1 + 1), A
      // line 550
      ld A, reg_wl1
      ld (addr_mod_wl1_1 + 1), A
      // line 551
      ld reg_vol, 0 + CHORD_VOL_MAX
      // line 552
      jp nc, SD4PLAY_NOTE_CHORD2__4 // if (c) {
          // line 555
          ld A, 0 + OPCODE_JR
          ld (addr_mod_vol_start_0 + 0), A
          // line 556
          ld A, 0 + OPCODE_LD_A_N
      jp SD4PLAY_NOTE_CHORD2__3
      SD4PLAY_NOTE_CHORD2__4: // } else {
          // line 560
          ld reg_wl01, 0 + (CHORD_VOL_MIN_DW << 8) | dec_vol
          // line 561
sd4playNoteChord2_initDw:
          // line 562
          ld A, (VRAM_TEXT)
          ex AF, AF
          ld (MMIO_8253_CT0), A
          ex AF, AF
          // line 563
          ld A, reg_wl0
          ld (addr_mod_vol_end_1   + 1), A
          // line 564
          ld A, reg_wl1
          ld (addr_mod_vol_inc_0   + 0), A
          // line 565
          ld A, 0 + OPCODE_JR_NZ
          ld (addr_mod_vol_start_0 + 0), A
          // line 566
          ld A, 0 + OPCODE_LD_A_N
SD4PLAY_NOTE_CHORD2__3: // } endif
      // line 568
sd4playNoteChord2_initEnd:
      // line 569
      ld (addr_mod_wl_start_0 + 0), A
      // line 570
      ld reg_wl0, 0 + 1
      // line 572
      ld A, (VRAM_TEXT)
      ex AF, AF
      ld (MMIO_8253_CT0), A
      ex AF, AF
  SD4PLAY_NOTE_CHORD2__0: // } endif 
endm
__endasm;
}

















void SD4PLAY_MOD_CHORD2(
    int reg_hwl, int reg_vol, int reg_slc,
    int addr_mod_wl_start_0,  int addr_mod_wl0_1,     int addr_mod_wl1_1,
    int addr_mod_vol_start_0, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_beep_wl_1
)  __naked
{
__asm
SD4PLAY_MOD_CHORD2 macro reg_hwl, reg_vol, reg_slc, addr_mod_wl_start_0, addr_mod_wl0_1, addr_mod_wl1_1, addr_mod_vol_start_0, addr_mod_vol_end_1, addr_mod_vol_inc_0, addr_beep_wl_1
local SD4PLAY_MOD_CHORD2__0, SD4PLAY_MOD_CHORD2__1
  // line 601
  extern sd4playModChord_end
  // line 604
addr_mod_wl_start_0:
  jr 0 + sd4playModChord_end
  // line 607
  ld A, reg_slc
  and A, 0 + 2
addr_mod_wl0_1:
  ld A, 0 + 0x00
  // line 608
  jr nz, SD4PLAY_MOD_CHORD2__0 // if (z_jr) {
addr_mod_wl1_1:
      ld A, 0 + 0x00
  SD4PLAY_MOD_CHORD2__0: // } endif 
  // line 609
  ld (addr_beep_wl_1 + 1), A
  rra
  ld reg_hwl, A
  // line 612
  ld A, reg_slc
  and A, 0 + 1
  // line 613
addr_mod_vol_start_0:
  jr nz, 0 + sd4playModChord_end
  // line 614
  ld A, reg_vol
addr_mod_vol_end_1:
  cp A, 0 + 0x00
  // line 615
  jr z, SD4PLAY_MOD_CHORD2__1 // if (nz_jr) {
addr_mod_vol_inc_0:
      inc reg_vol
  SD4PLAY_MOD_CHORD2__1: // } endif 
  // line 618
sd4playModChord_end:
endm
__endasm;
}











void SD4PLAY_NOTE_DRUM(int reg_sl, int reg_tmp,
    int addr_drum_ct_1, int addr_drum_nr_1, int addr_drum_add_0)  __naked
{
__asm
SD4PLAY_NOTE_DRUM macro reg_sl, reg_tmp, addr_drum_ct_1, addr_drum_nr_1, addr_drum_add_0
local SD4PLAY_NOTE_DRUM__0, SD4PLAY_NOTE_DRUM__1, SD4PLAY_NOTE_DRUM__2, SD4PLAY_NOTE_DRUM__3, SD4PLAY_NOTE_DRUM__4
  // line 636
  extern sd4PlayModDrum_ct_1, sd4PlayModDrum_end
  // line 638
  dec reg_sl
  jp nz, SD4PLAY_NOTE_DRUM__0 // if (z) {
      // line 639
      dec SP
      pop AF
      // line 640
      ld reg_tmp, A
      and A, 0 + 0xfc
      ld reg_sl, A
      // line 641
      xor A, reg_tmp
      // line 642
      jr z, SD4PLAY_NOTE_DRUM__1 // if (nz_jr) {
          // line 643
          add A, 0 + (ADDR_SD4_DRUM / 256 - 1)
          ld (addr_drum_nr_1 + 1), A
          // line 644
          ld A, (VRAM_TEXT)
          ex AF, AF
          ld (MMIO_8253_CT0), A
          ex AF, AF
          // line 645
          ld A, 0 + OPCODE_RES_0_L
          ld (addr_drum_ct_1  + 1), A
          // line 646
          ld A, 0 + OPCODE_OR_HL
          ld (addr_drum_add_0 + 0), A
          // line 647
          ld A, 0 + 2
          ld (sd4PlayModDrum_ct_1 + 1), A
      SD4PLAY_NOTE_DRUM__1: // } endif 
      // line 650
      jp sd4PlayModDrum_end
  SD4PLAY_NOTE_DRUM__0: // } endif 
  // line 653
sd4PlayModDrum_ct_1:
  ld A, 0 + 0x00
  or A, A
  jr z, SD4PLAY_NOTE_DRUM__2 // if (nz_jr) {
      // line 654
      dec A
      ld (sd4PlayModDrum_ct_1 + 1), A
      // line 655
      jr z, SD4PLAY_NOTE_DRUM__4 // if (nz_jr) {
          // line 656
          ld A, 0 + OPCODE_SET_0_L
          ld (addr_drum_ct_1  + 1), A
      jp SD4PLAY_NOTE_DRUM__3
      SD4PLAY_NOTE_DRUM__4: // } else {
          // line 658
          xor A, A
          ld (addr_drum_add_0 + 0), A
SD4PLAY_NOTE_DRUM__3: // } endif
  SD4PLAY_NOTE_DRUM__2: // } endif 
  // line 662
sd4PlayModDrum_end:
endm
__endasm;
}











void SD4PLAY_BEEPER_WAVE_R(int reg_wlc, int reg_hwl, int reg_vol, int addr_wl_1)  __naked
{
__asm
SD4PLAY_BEEPER_WAVE_R macro reg_wlc, reg_hwl, reg_vol, addr_wl_1
local SD4PLAY_BEEPER_WAVE_R__0, SD4PLAY_BEEPER_WAVE_R__1, SD4PLAY_BEEPER_WAVE_R__2
  // line 681
  dec reg_wlc
  jp nz, SD4PLAY_BEEPER_WAVE_R__1 // if (z) {
      // line 682
addr_wl_1:
      ld reg_wlc, 0 + 0x00
  jp SD4PLAY_BEEPER_WAVE_R__0
  SD4PLAY_BEEPER_WAVE_R__1: // } else {
      // line 685
      ld A, reg_wlc
      cp A, reg_hwl
      // line 686
      jr nc, SD4PLAY_BEEPER_WAVE_R__2 // if (c_jr) {
          ex AF, AF
          add A, reg_vol
          ex AF, AF
      SD4PLAY_BEEPER_WAVE_R__2: // } endif 
SD4PLAY_BEEPER_WAVE_R__0: // } endif
endm
__endasm;
}










void SD4PLAY_BEEPER_WAVE_BR(int reg_wl, int reg_hwl, int reg_vol, int addr_wl_1)  __naked
{
__asm
SD4PLAY_BEEPER_WAVE_BR macro reg_wl, reg_hwl, reg_vol, addr_wl_1
local SD4PLAY_BEEPER_WAVE_BR__0
  // line 704
local sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd
  // line 705
  extern sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd
  // line 708
  djnz B, 0 + sd4play_beeperWaveBr
    // line 709
addr_wl_1:
    ld reg_wl, 0 + 0x00
    jr 0 + sd4play_beeperWaveBrEnd
  // line 711
sd4play_beeperWaveBr:
    // line 713
    ld A, reg_wl
    cp A, reg_hwl
    // line 714
    jr nc, SD4PLAY_BEEPER_WAVE_BR__0 // if (c_jr) {
        ex AF, AF
        add A, reg_vol
        ex AF, AF
    SD4PLAY_BEEPER_WAVE_BR__0: // } endif 
  // line 716
sd4play_beeperWaveBrEnd:
endm
__endasm;
}










void SD4PLAY_BEEPER_WAVE_B(int reg_wl, int reg_hwl, int addr_vol_1, int addr_wl_1)  __naked
{
__asm
SD4PLAY_BEEPER_WAVE_B macro reg_wl, reg_hwl, addr_vol_1, addr_wl_1
local SD4PLAY_BEEPER_WAVE_B__0
  // line 733
local sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd
  // line 734
  extern sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd
  // line 737
  djnz B, 0 + sd4play_beeperWaveBr
    // line 738
addr_wl_1:
    ld reg_wl, 0 + 0x00
    jr 0 + sd4play_beeperWaveBrEnd
  // line 740
sd4play_beeperWaveBr:
    // line 742
    ld A, reg_wl
    cp A, reg_hwl
    // line 743
    jr nc, SD4PLAY_BEEPER_WAVE_B__0 // if (c_jr) {
        ex AF, AF
addr_vol_1:
        add A, 0 + 0x00
        ex AF, AF
    SD4PLAY_BEEPER_WAVE_B__0: // } endif 
  // line 745
sd4play_beeperWaveBrEnd:
endm
__endasm;
}













void SD4PLAY_BEEPER_DRUM(
    int reg_wav_h, int reg_wav_l, int reg_wav_hl, int reg_ct,
    int addr_drum_ct_1, int addr_drum_nr_1, int addr_drum_add_0)  __naked
{
__asm
SD4PLAY_BEEPER_DRUM macro reg_wav_h, reg_wav_l, reg_wav_hl, reg_ct, addr_drum_ct_1, addr_drum_nr_1, addr_drum_add_0
  // line 767
  ld reg_wav_l, reg_ct
addr_drum_ct_1:
  set 0, reg_wav_l
  // line 768
addr_drum_nr_1:
  ld reg_wav_h, 0 + (ADDR_SD4_DRUM - 1) / 256 + 1
  // line 769
  ex AF, AF
addr_drum_add_0:
  nop
endm
__endasm;
}




u8 sd4play(u32 param)  __z88dk_fastcall
{
__asm
  // line 781
  extern sd4play_waitUntilKeyOff1
  // line 782
  extern sd4play_waitUntilKeyOff2
  // line 783
  extern sd4play_vblk0, sd4play_vblk1
  // line 784
  extern sd4play_setVols, sd4play_loop, sd4play_beeperLoop
  // line 785
  extern sd4play_rep_1, sd4play_leadSlide_0
  // line 786
  extern sd4play_modLead_start
  // line 787
  extern sd4play_modLead_vol, sd4play_modLead_volSpeed_1, sd4play_modLead_vol_1, sd4play_modLead_volEnd_1, sd4play_modLead_volInc_0
  // line 788
  extern sd4play_modLead_wl, sd4play_modLead_wlSpeed_1, sd4play_modLead_wlEnd_1, sd4play_modLead_wlInc_0
  // line 789
  extern sd4play_modLead_end
  // line 790
  extern sd4play_modBase_start, sd4play_modBase_volEnd_1, sd4play_modBase_volInc_0
  // line 791
  extern sd4play_modChord_wlStart, sd4play_modChord_wl0, sd4play_modChord_wl1
  // line 792
  extern sd4play_modChord_volStart, sd4play_modChord_volEnd, sd4play_modChord_volInc
  // line 793
  extern sd4play_beeper0_wl_1
  // line 794
  extern sd4play_beeper1_wl_1
  // line 795
  extern sd4play_beeper2_wl_1
  // line 796
  extern sd4play_beeper3_drumCt, sd4play_beeper3_drumNr, sd4play_beeper3_drumAdd
  // line 797
  extern sd4play_end
  // line 798
  extern sd4play_restoreSP
  // line 799
  extern tmp
  // line 802
  push IX
  // line 803
  BANKH_VRAM_MMIO C
  // line 806
  ld A, 0 + 0xf9
  ld (MMIO_8255_PORTA), A
  // line 807
sd4play_waitUntilKeyOff1:
  // line 808
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F2_MASK | KEY9_F4_MASK
  jr nz, 0 + sd4play_waitUntilKeyOff1
  // line 811
  dec E
  jr z, sd4play__0 // if (nz_jr) {
      // line 812
      ld A, 0 + 0xfa
      ld (MMIO_8255_PORTA), A
  sd4play__0: // } endif 
  // line 816
  ld A, 0 + MMIO_8253_CTRL_CT0(MMIO_8253_CTRL_RL_L_MASK, MMIO_8253_CTRL_MODE0_MASK)
  // line 817
  ld (MMIO_8253_CTRL), A
  // line 819
#if  0   
L00:
  ld A, (VRAM_TEXT)
  // line 821
  ld A, 0 + 56/2
  ld (MMIO_8253_CT0), A
  // line 822
  ex (SP), HL
  ex (SP), HL
  ex (SP), HL
  ex (SP), HL
  // line 823
  jr 0 + L00
  // line 824
#endif 
  // line 838
  ld (sd4play_restoreSP + 1), SP
  ld SP, HL
  // line 839
  xor A, A
  // line 840
  ld (sd4PlayModDrum_ct_1        + 1), A
  // line 841
  ld (sd4play_beeper3_drumAdd    + 0), A
  // line 842
  ld (sd4play_modLead_volSpeed_1 + 1), A
  // line 844
  inc A
  // line 845
  ld IXH, A
  ld IXL, A
  ld IYL, A
  ld IYH, A
  // line 846
  ld HL, 0 + ADDR_SD4_REP + 0xff
  ld (sd4play_rep_1 + 1), HL
  // line 847
  ld A, 0 + OPCODE_JR
  ld (sd4play_leadSlide_0 + 0), A
  // line 850
  xor A, A
  ld HL, 0 + MMIO_8255_PORTC
  // line 851
sd4play_vblk1:
  or A, (HL)
  jp p, 0 + sd4play_vblk1
  // line 852
  ld A, (VRAM_TEXT)
  // line 853
  xor A, A
  ex AF, AF
  // line 857
sd4play_loop:
  // line 858
  ld A, (VRAM_TEXT)
  ex AF, AF
  ld (MMIO_8253_CT0), A
  ex AF, AF
  // line 862
  SD4PLAY_NOTE_LEAD H, L, HL, D, IXH, OPCODE_INC_D, OPCODE_DEC_D, sd4play_rep_1, sd4play_leadSlide_0, sd4play_modLead_start, sd4play_modLead_vol, sd4play_modLead_volSpeed_1, sd4play_modLead_volEnd_1, sd4play_modLead_volInc_0, sd4play_modLead_wl, sd4play_modLead_wlSpeed_1, sd4play_modLead_wlEnd_1, sd4play_modLead_wlInc_0, sd4play_beeper0_wl_1, sd4play_modLead_end, sd4play_end
  // line 869
  SD4PLAY_NOTE_BASE B, C, BC, E, IXL, OPCODE_INC_E, OPCODE_DEC_E, sd4play_modBase_start, sd4play_modBase_volEnd_1, sd4play_modBase_volInc_0, sd4play_beeper1_wl_1
  // line 872
  SD4PLAY_MOD_LEAD L, D, IXH, sd4play_modLead_start, sd4play_modLead_vol, sd4play_modLead_volSpeed_1, sd4play_modLead_volEnd_1, sd4play_modLead_volInc_0, sd4play_modLead_wl, sd4play_modLead_wlSpeed_1, sd4play_modLead_wlEnd_1, sd4play_modLead_wlInc_0, sd4play_beeper0_wl_1, sd4play_modLead_end
  // line 878
  SD4PLAY_MOD_BASE E, IXL, sd4play_modBase_start, sd4play_modBase_volEnd_1, sd4play_modBase_volInc_0
  // line 880
  ld A, (VRAM_TEXT)
  ex AF, AF
  ld (MMIO_8253_CT0), A
  ex AF, AF
  // line 881
  exx
  // line 882
  SD4PLAY_NOTE_CHORD2 B, C, BC, D, IYH, OPCODE_INC_D, OPCODE_DEC_D, sd4play_modChord_wlStart, sd4play_modChord_wl0, sd4play_modChord_wl1, sd4play_modChord_volStart, sd4play_modChord_volEnd, sd4play_modChord_volInc, sd4play_beeper2_wl_1
  // line 886
  SD4PLAY_MOD_CHORD2 C, D, IYH, sd4play_modChord_wlStart, sd4play_modChord_wl0, sd4play_modChord_wl1, sd4play_modChord_volStart, sd4play_modChord_volEnd, sd4play_modChord_volInc, sd4play_beeper2_wl_1
  // line 890
  SD4PLAY_NOTE_DRUM IYL, L, sd4play_beeper3_drumCt, sd4play_beeper3_drumNr, sd4play_beeper3_drumAdd
  // line 892
  ld E, 0 + TEMPO
  // line 893
  ld A, (VRAM_TEXT)
  ex AF, AF
  ld (MMIO_8253_CT0), A
  // line 896
sd4play_beeperLoop:
  // line 897
    // line 898
    xor A, A
    ex AF, AF
    // line 899
    exx
    // line 901
    SD4PLAY_BEEPER_WAVE_R H, L, D, sd4play_beeper0_wl_1
    // line 902
    SD4PLAY_BEEPER_WAVE_BR B, C, E, sd4play_beeper1_wl_1
    // line 903
    exx
    // line 904
    SD4PLAY_BEEPER_WAVE_BR B, C, D, sd4play_beeper2_wl_1
    // line 905
    SD4PLAY_BEEPER_DRUM H, L, HL, E, sd4play_beeper3_drumCt, sd4play_beeper3_drumNr, sd4play_beeper3_drumAdd
    // line 910
    ld H, 0 + ADDR_SD4_VTAB / 256
    ld L, A
    // line 911
    ld A, (VRAM_TEXT)
    ld A, (HL)
    ld (MMIO_8253_CT0), A
    // line 912
    inc E
    jp nz, 0 + sd4play_beeperLoop
  // line 915
  exx
  // line 916
  ex AF, AF
  // line 918
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F2_MASK | KEY9_F4_MASK
  jp z, 0 + sd4play_loop
  // line 919
  ld L, A
  // line 920
  jr 0 + sd4play_waitUntilKeyOff2
  // line 923
sd4play_end:
  // line 924
  ld L, 0 + 0x00
  // line 926
sd4play_waitUntilKeyOff2:
  // line 927
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F2_MASK | KEY9_F4_MASK
  jr nz, 0 + sd4play_waitUntilKeyOff2
  // line 930
  ld A, 0 + MMIO_8253_CT0_MODE3
  ld (MMIO_8253_CTRL), A
  // line 932
  BANKH_RAM C
  // line 934
sd4play_restoreSP:
  ld SP, 0 + 0x0000
  // line 935
  pop IX
__endasm;
}

#pragma restore
