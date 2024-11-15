/**** This file is made by ../tools/z80ana.php.  DO NOT MODIFY! ****/





#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "../src-common/asm_macros.h"

#include "addr.h"
#include "sound.h"



    STATIC_ASSERT(SD4_B1 < 256 + 1, "valueOverflow"); 
    STATIC_ASSERT(SD4_C6 >= 15,     "valueUnderflow");

#if 0
    STATIC_ASSERT(SD4_WL(SD_FREQ_C2) < 128.0f, "valueOverflow");    
    STATIC_ASSERT(SD4_WL(SD_FREQ_C5) > 15.0f,  "valueUnderflow");   
    #define MMIO_8253_CT0_PWM   MMIO_8253_CTRL_CT0(MMIO_8253_CTRL_RL_L_MASK, MMIO_8253_CTRL_MODE0_MASK)
#endif


#define TEMPO               0       
#define CHORD_VOL_SPEED_R   7       

void LEAD_ECHO_DELAY(void)  __naked
{
__asm
LEAD_ECHO_DELAY macro void
// line 34
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



#define JR          0x18
#define JR_NZ       0x20
#define JR_Z        0x28

#define LD_A_N      0x3e    
#define OR_A_N      0xf6    
#define INC_A       0x3c
#define DEC_A       0x3d
#define INC_D       0x14
#define DEC_D       0x15
#define INC_E       0x1c
#define DEC_E       0x1d
#define RES_0_L     0x85
#define SET_0_L     0xc5
#define ADD_A_HL    0x86
#define OR_A_HL     0xb6
#define XOR_A_HL    0xae








#pragma disable_warning 85
#pragma save



static const u8 SD4_DATA_[] = {
    
    
    
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



void sd4Init(void)  __naked
{
__asm
// line 106
  extern _SD4_DATA_
// line 107
  extern sd4Init_dataLoop
// line 110
  ld HL, 0 + _SD4_DATA_
// line 111
  ld DE, 0 + ADDR_SD4_VTAB
// line 112
  ld BC, 0 + SZ_SD4_VTAB + SZ_SD4_TAB
// line 113
  ldir
// line 115
  ld DE, 0 + ADDR_SD4_DRUM
// line 116
  ld BC, 0 + SZ_SD4_DRUM
// line 117
  ldir
// line 120
  BANK_VRAM_MMIO C
// line 121
  ld HL, 0 + MMIO_ETC
// line 122
  ld (HL), 0 + MMIO_ETC_GATE_MASK
// line 123
  dec L
// line 124
  ld (HL), 0 + MMIO_8253_CT0_MODE3
// line 125
  BANK_RAM C
// line 127
  ret
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
// line 172
  extern sd4playNoteLead_pop0, sd4playNoteLead_pop, sd4playNoteLead_rep_2, sd4playNoteLead_initDw, sd4playNoteLead_initDw_1, sd4playNoteLead_initEnd, sd4playNoteLead_initEndSlur
// line 179
  dec reg_slc
  jp nz, SD4PLAY_NOTE_LEAD__0
// line 180
sd4playNoteLead_pop:
// line 181
      pop reg_wlchwl
// line 184
      ld A, reg_hwl
      add A, A
      jr c, SD4PLAY_NOTE_LEAD__1
// line 185
          add A, A
          ld reg_slc, A
          jr nc, SD4PLAY_NOTE_LEAD__2
// line 188
              cp A, 0 + SD4_EXCEPT_LEN
              jr nz, SD4PLAY_NOTE_LEAD__3
// line 189
addr_rep_1:
                  ld HL, 0 + 0x0000
                  dec (HL)
                  jr nz, SD4PLAY_NOTE_LEAD__4
// line 190
                      dec L
                      dec L
                      dec L
                      ld (addr_rep_1 + 1), HL
// line 191
                      dec SP
// line 192
                      jp sd4playNoteLead_pop0
SD4PLAY_NOTE_LEAD__4:
// line 195
                  dec L
                  ld A, (HL)
                  dec L
                  ld L, (HL)
                  ld H, A
                  ld SP, HL
// line 196
                  jp sd4playNoteLead_pop0
SD4PLAY_NOTE_LEAD__3:
// line 200
// line 201
                cp A, 0 + SD4_EXCEPT_LEN + 4
                jp z, 0 + addr_end
// line 205
// line 206
                ld A, (VRAM_TEXT)
                ex AF, AF
                ld (MMIO_8253_CT0), A
                ex AF, AF
// line 207
                ld A, reg_wlc
                ld (addr_wl_1 + 1), A
// line 208
                ld reg_wlchwl, 0 + addr_mod_vol
                ld (addr_mod_start_1 + 1), reg_wlchwl
// line 209
                ld reg_wlchwl, 0 + (LEAD_VOL_MAX << 8) | inc_vol
// line 210
                ld reg_vol, 0 + LEAD_VOL_MIN_UP
// line 211
                jp sd4playNoteLead_initDw
SD4PLAY_NOTE_LEAD__2:
// line 216
          cp A, 0 + SD4_EXCEPT_LEN
          jp c, SD4PLAY_NOTE_LEAD__5
// line 217
              cp A, 0 + SD4_EXCEPT_LEN + 4 * 7
              jp nc, SD4PLAY_NOTE_LEAD__6
// line 218
                  dec SP
// line 219
                  sub A, 0 + SD4_EXCEPT_LEN - 8
// line 220
                  rept 2
                    rrca
                  endr
// line 221
                  ld HL, (addr_rep_1 + 1)
// line 222
                  inc L
                  ld (sd4playNoteLead_rep_2 + 2), HL
sd4playNoteLead_rep_2:
                  ld (0x0000), SP
// line 223
                  inc L
                  inc L
                  ld (HL), A
// line 224
                  ld (addr_rep_1 + 1), HL
// line 225
                  jp sd4playNoteLead_pop0
SD4PLAY_NOTE_LEAD__6:
SD4PLAY_NOTE_LEAD__5:
// line 230
// line 231
            dec SP
// line 232
            xor A, A
            ld (addr_wl_1 + 1), A
// line 233
            ld reg_wlchwl, 0 + addr_mod_end
            ld (addr_mod_start_1 + 1), reg_wlchwl
// line 234
            ld reg_hwl, A
// line 235
            jp sd4playNoteLead_initEndSlur
SD4PLAY_NOTE_LEAD__1:
// line 238
      add A, A
      ld reg_slc, A
      jr nc, SD4PLAY_NOTE_LEAD__7
// line 239
          cp A, 0 + SD4_EXCEPT_LEN
          jp c, SD4PLAY_NOTE_LEAD__8
// line 240
              jr nz, SD4PLAY_NOTE_LEAD__9
// line 243
                  ld A, 0 + JR
                  ld (addr_slide_0 + 0), A
// line 244
                  dec SP
// line 245
                  jp sd4playNoteLead_pop0
SD4PLAY_NOTE_LEAD__9:
// line 248
              cp A, 0 + SD4_EXCEPT_LEN + 4 * 4
              jp nc, SD4PLAY_NOTE_LEAD__10
// line 250
                  sub A, 0 + SD4_EXCEPT_LEN - (SZ_SD4_VTAB + 1)
// line 251
                  ld reg_wlc, 0 + ADDR_SD4_VTAB / 256
                  ld reg_hwl, A
                  ld A, (reg_wlchwl)
// line 252
                  ld (addr_mod_wl_speed_1 + 1), A
// line 253
                  ld A, 0 + LD_A_N
                  ld (addr_slide_0 + 0), A
// line 254
                  dec SP
// line 255
                  jp sd4playNoteLead_pop0
SD4PLAY_NOTE_LEAD__10:
SD4PLAY_NOTE_LEAD__8:
// line 259
          ld A, (VRAM_TEXT)
          ex AF, AF
          ld (MMIO_8253_CT0), A
          ex AF, AF
// line 260
addr_slide_0:
          jr nz, SD4PLAY_NOTE_LEAD__11
// line 263
              ld A, reg_wlc
              ld (addr_mod_wl_end_1 + 1), A
// line 265
              ld A, (addr_wl_1 + 1)
              cp A, reg_wlc
              ld A, 0 + INC_A
              jr c, SD4PLAY_NOTE_LEAD__12
                  inc A
SD4PLAY_NOTE_LEAD__12:
// line 266
              ld (addr_mod_wl_inc_0 + 0), A
// line 267
              ld reg_wlchwl, 0 + addr_mod_wl
              ld (addr_mod_start_1 + 1), reg_wlchwl
// line 268
              ld A, (addr_wl_1 + 1)
              rept 1
                srl A
              endr
              ld reg_hwl, A
// line 269
              jp sd4playNoteLead_initEndSlur
SD4PLAY_NOTE_LEAD__11:
// line 273
// line 274
            ld A, reg_wlc
            ld (addr_wl_1  + 1), A
// line 275
            ld reg_wlchwl, 0 + addr_mod_end
            ld (addr_mod_start_1 + 1), reg_wlchwl
// line 276
            rept 1
              srl A
            endr
            ld reg_hwl, A
// line 277
            ld reg_vol, 0 + LEAD_VOL_MAX
// line 278
            jp sd4playNoteLead_initEnd
SD4PLAY_NOTE_LEAD__7:
// line 282
      cp A, 0 + SD4_EXCEPT_LEN
      jr c, SD4PLAY_NOTE_LEAD__13
// line 283
          cp A, 0 + SD4_EXCEPT_LEN + 4 * 4
          jp nc, SD4PLAY_NOTE_LEAD__14
// line 286
              sub A, 0 + SD4_EXCEPT_LEN - SZ_SD4_VTAB
// line 287
              ld reg_wlc, 0 + ADDR_SD4_VTAB / 256
              ld reg_hwl, A
              ld A, (reg_wlchwl)
// line 288
              ld (addr_mod_vol_speed_1 + 1), A
// line 289
              dec SP
// line 290
sd4playNoteLead_pop0:
// line 291
              ld A, (VRAM_TEXT)
              ex AF, AF
              ld (MMIO_8253_CT0), A
              ex AF, AF
// line 292
              jp sd4playNoteLead_pop
SD4PLAY_NOTE_LEAD__14:
SD4PLAY_NOTE_LEAD__13:
// line 299
// line 300
        ld A, (VRAM_TEXT)
        ex AF, AF
        ld (MMIO_8253_CT0), A
        ex AF, AF
// line 301
        ld A, reg_wlc
        ld (addr_wl_1 + 1), A
// line 302
        ld reg_wlchwl, 0 + addr_mod_vol
        ld (addr_mod_start_1 + 1), reg_wlchwl
// line 303
        ld reg_wlchwl, 0 + (LEAD_VOL_MIN_DW << 8) | dec_vol
// line 304
        ld reg_vol, 0 + LEAD_VOL_MAX
// line 306
sd4playNoteLead_initDw:
// line 307
      rept 1
        srl A
      endr
      ld (sd4playNoteLead_initDw_1 + 1), A
// line 308
      ld A, reg_wlc
      ld (addr_mod_vol_end_1 + 1), A
// line 309
      ld A, reg_hwl
      ld (addr_mod_vol_inc_0 + 0), A
// line 310
sd4playNoteLead_initDw_1:
      ld reg_hwl, 0 + 0x00
// line 312
sd4playNoteLead_initEnd:
// line 313
      ld reg_wlc, 0 + 1
// line 315
sd4playNoteLead_initEndSlur:
// line 316
      ld A, (VRAM_TEXT)
      ex AF, AF
      ld (MMIO_8253_CT0), A
      ex AF, AF
SD4PLAY_NOTE_LEAD__0:
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
// line 350
addr_mod_start_1:
  jp 0 + addr_mod_end
// line 353
addr_mod_vol:
// line 354
  ld A, reg_slc
addr_mod_vol_speed_1:
  and A, 0 + 0x00
  jr nz, SD4PLAY_MOD_LEAD__0
// line 355
      ld A, reg_vol
addr_mod_vol_end_1:
      cp A, 0 + 0x00
// line 356
      jr z, SD4PLAY_MOD_LEAD__1
addr_mod_vol_inc_0:
          inc reg_vol
SD4PLAY_MOD_LEAD__1:
SD4PLAY_MOD_LEAD__0:
// line 358
  jp addr_mod_end
// line 361
addr_mod_wl:
// line 362
  ld A, reg_slc
addr_mod_wl_speed_1:
  and A, 0 + 0x00
  jr nz, 0 + addr_mod_end
// line 363
    ld A, (addr_wl_1 + 1)
addr_mod_wl_end_1:
    cp A, 0 + 0x00
// line 364
    jr z, 0 + addr_mod_end
addr_mod_wl_inc_0:
    inc A
    ld (addr_wl_1 + 1), A
// line 365
    rept 1
      srl A
    endr
    ld reg_hwl, A
// line 370
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
// line 396
  extern sd4playNoteBase_fl, sd4playNoteBase_initEnd
// line 399
  dec reg_slc
  jr nz, SD4PLAY_NOTE_BASE__0
// line 400
      pop reg_wlwl
// line 403
      ld A, reg_hwl
      add A, A
      jr c, SD4PLAY_NOTE_BASE__1
// line 404
          add A, A
          ld reg_slc, A
          jr nc, SD4PLAY_NOTE_BASE__2
// line 407
              ld A, reg_wlc
              ld (addr_wl_1 + 1), A
// line 408
              rept 1
                srl A
              endr
              ld reg_hwl, A
// line 409
              ld reg_vol, 0 + 0
// line 410
              ld A, 0 + BASE_VOL_MAX
              ld (addr_mod_vol_end_1 + 1), A
// line 411
              ld A, 0 + inc_vol
              ld (addr_mod_vol_inc_0 + 0), A
// line 412
              ld A, 0 + JR_NZ
// line 413
              jp sd4playNoteBase_initEnd
SD4PLAY_NOTE_BASE__2:
// line 417
          dec SP
// line 418
          xor A, A
          ld (addr_wl_1 + 1), A
// line 419
          ld reg_hwl, A
// line 420
          ld A, 0 + JR
        jp SD4PLAY_NOTE_BASE__3
SD4PLAY_NOTE_BASE__1:
// line 422
          add A, A
          ld reg_slc, A
// line 423
          ld A, reg_wlc
          ld (addr_wl_1 + 1), A
// line 424
          ld reg_vol, 0 + BASE_VOL_MAX
// line 426
          jr nc, SD4PLAY_NOTE_BASE__4
// line 429
              rept 1
                srl A
              endr
              ld reg_hwl, A
// line 430
              ld A, 0 + JR
            jp SD4PLAY_NOTE_BASE__5
SD4PLAY_NOTE_BASE__4:
// line 434
              rra
              ld reg_hwl, A
// line 435
              ld A, 0 + BASE_VOL_MIN_DW
              ld (addr_mod_vol_end_1 + 1), A
// line 436
              ld A, 0 + dec_vol
              ld (addr_mod_vol_inc_0 + 0), A
// line 437
              ld A, 0 + JR_NZ
SD4PLAY_NOTE_BASE__5:
SD4PLAY_NOTE_BASE__3:
// line 440
sd4playNoteBase_initEnd:
// line 441
      ld (addr_mod_start_0 + 0), A
// line 442
      ld reg_wlc, 0 + 1
// line 444
      ld A, (VRAM_TEXT)
      ex AF, AF
      ld (MMIO_8253_CT0), A
      ex AF, AF
SD4PLAY_NOTE_BASE__0:
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
// line 463
  extern sd4playModBase_end
// line 466
  ld A, reg_slc
  and A, 0 + 0x01
// line 467
addr_mod_start_0:
  jr nz, 0 + sd4playModBase_end
// line 468
  ld A, reg_vol
addr_mod_vol_end_1:
  cp A, 0 + 0x00
// line 469
  jr z, SD4PLAY_MOD_BASE__0
addr_mod_vol_inc_0:
      inc reg_vol
SD4PLAY_MOD_BASE__0:
// line 473
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
// line 504
  extern sd4playNoteChord2_initDw, sd4playNoteChord2_initEnd
// line 507
  dec reg_slc
  jp nz, SD4PLAY_NOTE_CHORD2__0
// line 508
      dec SP
      pop AF
// line 511
      add A, A
      jr c, SD4PLAY_NOTE_CHORD2__1
// line 512
          add A, A
          ld reg_slc, A
          jr nc, SD4PLAY_NOTE_CHORD2__2
// line 517
              pop reg_wl01
// line 518
              ld A, reg_wl0
              ld (addr_mod_wl0_1 + 1), A
// line 519
              ld A, reg_wl1
              ld (addr_mod_wl1_1 + 1), A
// line 520
              ld reg_vol, 0 + 0
// line 521
              ld reg_wl01, 0 + (CHORD_VOL_MAX << 8) | inc_vol
// line 522
              jp sd4playNoteChord2_initDw
SD4PLAY_NOTE_CHORD2__2:
// line 526
          xor A, A
          ld (addr_beep_wl_1 + 1), A
// line 527
          ld reg_wl1, A
// line 528
          ld A, 0 + JR
// line 529
          jp sd4playNoteChord2_initEnd
SD4PLAY_NOTE_CHORD2__1:
// line 531
      add A, A
      ld reg_slc, A
// line 532
      pop reg_wl01
// line 533
      ld A, reg_wl0
      ld (addr_mod_wl0_1 + 1), A
// line 534
      ld A, reg_wl1
      ld (addr_mod_wl1_1 + 1), A
// line 535
      ld reg_vol, 0 + CHORD_VOL_MAX
// line 536
      jp nc, SD4PLAY_NOTE_CHORD2__3
// line 539
          ld A, 0 + JR
          ld (addr_mod_vol_start_0 + 0), A
// line 540
          ld A, 0 + LD_A_N
        jp SD4PLAY_NOTE_CHORD2__4
SD4PLAY_NOTE_CHORD2__3:
// line 544
          ld reg_wl01, 0 + (CHORD_VOL_MIN_DW << 8) | dec_vol
// line 545
sd4playNoteChord2_initDw:
// line 546
          ld A, (VRAM_TEXT)
          ex AF, AF
          ld (MMIO_8253_CT0), A
          ex AF, AF
// line 547
          ld A, reg_wl0
          ld (addr_mod_vol_end_1   + 1), A
// line 548
          ld A, reg_wl1
          ld (addr_mod_vol_inc_0   + 0), A
// line 549
          ld A, 0 + JR_NZ
          ld (addr_mod_vol_start_0 + 0), A
// line 550
          ld A, 0 + LD_A_N
SD4PLAY_NOTE_CHORD2__4:
// line 552
sd4playNoteChord2_initEnd:
// line 553
      ld (addr_mod_wl_start_0 + 0), A
// line 554
      ld reg_wl0, 0 + 1
// line 556
      ld A, (VRAM_TEXT)
      ex AF, AF
      ld (MMIO_8253_CT0), A
      ex AF, AF
SD4PLAY_NOTE_CHORD2__0:
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
// line 583
  extern sd4playModChord_end
// line 586
addr_mod_wl_start_0:
  jr 0 + sd4playModChord_end
// line 589
  ld A, reg_slc
  and A, 0 + 2
addr_mod_wl0_1:
  ld A, 0 + 0x00
// line 590
  jr nz, SD4PLAY_MOD_CHORD2__0
addr_mod_wl1_1:
      ld A, 0 + 0x00
SD4PLAY_MOD_CHORD2__0:
// line 591
  ld (addr_beep_wl_1 + 1), A
  rra
  ld reg_hwl, A
// line 594
  ld A, reg_slc
  and A, 0 + 1
// line 595
addr_mod_vol_start_0:
  jr nz, 0 + sd4playModChord_end
// line 596
  ld A, reg_vol
addr_mod_vol_end_1:
  cp A, 0 + 0x00
// line 597
  jr z, SD4PLAY_MOD_CHORD2__1
addr_mod_vol_inc_0:
      inc reg_vol
SD4PLAY_MOD_CHORD2__1:
// line 600
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
// line 616
  extern sd4PlayModDrum_ct_1, sd4PlayModDrum_end
// line 618
  dec reg_sl
  jp nz, SD4PLAY_NOTE_DRUM__0
// line 619
      dec SP
      pop AF
// line 620
      ld reg_tmp, A
      and A, 0 + 0xfc
      ld reg_sl, A
// line 621
      xor A, reg_tmp
// line 622
      jr z, SD4PLAY_NOTE_DRUM__1
// line 623
          add A, 0 + (ADDR_SD4_DRUM / 256 - 1)
          ld (addr_drum_nr_1 + 1), A
// line 624
          ld A, (VRAM_TEXT)
          ex AF, AF
          ld (MMIO_8253_CT0), A
          ex AF, AF
// line 625
          ld A, 0 + RES_0_L
          ld (addr_drum_ct_1      + 1), A
// line 626
          ld A, 0 + OR_A_HL
          ld (addr_drum_add_0     + 0), A
// line 627
          ld A, 0 + 2
          ld (sd4PlayModDrum_ct_1 + 1), A
SD4PLAY_NOTE_DRUM__1:
// line 630
      jp sd4PlayModDrum_end
SD4PLAY_NOTE_DRUM__0:
// line 633
sd4PlayModDrum_ct_1:
  ld A, 0 + 0x00
  or A, A
  jr z, SD4PLAY_NOTE_DRUM__2
// line 634
      dec A
      ld (sd4PlayModDrum_ct_1 + 1), A
// line 635
      jr z, SD4PLAY_NOTE_DRUM__3
// line 636
          ld A, 0 + SET_0_L
          ld (addr_drum_ct_1  + 1), A
        jp SD4PLAY_NOTE_DRUM__4
SD4PLAY_NOTE_DRUM__3:
// line 638
          xor A, A
          ld (addr_drum_add_0 + 0), A
SD4PLAY_NOTE_DRUM__4:
SD4PLAY_NOTE_DRUM__2:
// line 642
sd4PlayModDrum_end:
endm
__endasm;
}











void SD4PLAY_BEEPER_WAVE_R(int reg_wlc, int reg_hwl, int reg_vol, int addr_wl_1)  __naked
{
__asm
SD4PLAY_BEEPER_WAVE_R macro reg_wlc, reg_hwl, reg_vol, addr_wl_1
local SD4PLAY_BEEPER_WAVE_R__0, SD4PLAY_BEEPER_WAVE_R__1, SD4PLAY_BEEPER_WAVE_R__2
// line 659
  dec reg_wlc
  jp nz, SD4PLAY_BEEPER_WAVE_R__0
// line 660
addr_wl_1:
      ld reg_wlc, 0 + 0x00
    jp SD4PLAY_BEEPER_WAVE_R__1
SD4PLAY_BEEPER_WAVE_R__0:
// line 663
      ld A, reg_wlc
      cp A, reg_hwl
// line 664
      jr nc, SD4PLAY_BEEPER_WAVE_R__2
          ex AF, AF
          add A, reg_vol
          ex AF, AF
SD4PLAY_BEEPER_WAVE_R__2:
SD4PLAY_BEEPER_WAVE_R__1:
endm
__endasm;
}










void SD4PLAY_BEEPER_WAVE_BR(int reg_wl, int reg_hwl, int reg_vol, int addr_wl_1)  __naked
{
__asm
SD4PLAY_BEEPER_WAVE_BR macro reg_wl, reg_hwl, reg_vol, addr_wl_1
local SD4PLAY_BEEPER_WAVE_BR__0
// line 680
local sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd
// line 681
  extern sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd
// line 684
  djnz B, 0 + sd4play_beeperWaveBr
// line 685
addr_wl_1:
    ld reg_wl, 0 + 0x00
    jr 0 + sd4play_beeperWaveBrEnd
// line 687
sd4play_beeperWaveBr:
// line 689
    ld A, reg_wl
    cp A, reg_hwl
// line 690
    jr nc, SD4PLAY_BEEPER_WAVE_BR__0
        ex AF, AF
        add A, reg_vol
        ex AF, AF
SD4PLAY_BEEPER_WAVE_BR__0:
// line 692
sd4play_beeperWaveBrEnd:
endm
__endasm;
}










void SD4PLAY_BEEPER_WAVE_B(int reg_wl, int reg_hwl, int addr_vol_1, int addr_wl_1)  __naked
{
__asm
SD4PLAY_BEEPER_WAVE_B macro reg_wl, reg_hwl, addr_vol_1, addr_wl_1
local SD4PLAY_BEEPER_WAVE_B__0
// line 707
local sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd
// line 708
  extern sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd
// line 711
  djnz B, 0 + sd4play_beeperWaveBr
// line 712
addr_wl_1:
    ld reg_wl, 0 + 0x00
    jr 0 + sd4play_beeperWaveBrEnd
// line 714
sd4play_beeperWaveBr:
// line 716
    ld A, reg_wl
    cp A, reg_hwl
// line 717
    jr nc, SD4PLAY_BEEPER_WAVE_B__0
        ex AF, AF
addr_vol_1:
        add A, 0 + 0x00
        ex AF, AF
SD4PLAY_BEEPER_WAVE_B__0:
// line 719
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
// line 739
  ld reg_wav_l, reg_ct
addr_drum_ct_1:
  set 0, reg_wav_l
// line 740
addr_drum_nr_1:
  ld reg_wav_h, 0 + (ADDR_SD4_DRUM - 1) / 256 + 1
// line 741
  ex AF, AF
addr_drum_add_0:
  nop
endm
__endasm;
}




u8 sd4play(u32 param)  __z88dk_fastcall __naked
{
__asm
// line 751
  extern sd4play_waitUntilKeyOff1
// line 752
  extern sd4play_waitUntilKeyOff2
// line 753
  extern sd4play_vblk0, sd4play_vblk1
// line 754
  extern sd4play_setVols, sd4play_loop, sd4play_beeperLoop
// line 755
  extern sd4play_rep_1, sd4play_leadSlide_0
// line 756
  extern sd4play_modLead_start
// line 757
  extern sd4play_modLead_vol, sd4play_modLead_volSpeed_1, sd4play_modLead_vol_1, sd4play_modLead_volEnd_1, sd4play_modLead_volInc_0
// line 758
  extern sd4play_modLead_wl, sd4play_modLead_wlSpeed_1, sd4play_modLead_wlEnd_1, sd4play_modLead_wlInc_0
// line 759
  extern sd4play_modLead_end
// line 760
  extern sd4play_modBase_start, sd4play_modBase_volEnd_1, sd4play_modBase_volInc_0
// line 761
  extern sd4play_modChord_wlStart, sd4play_modChord_wl0, sd4play_modChord_wl1
// line 762
  extern sd4play_modChord_volStart, sd4play_modChord_volEnd, sd4play_modChord_volInc
// line 763
  extern sd4play_beeper0_wl_1
// line 764
  extern sd4play_beeper1_wl_1
// line 765
  extern sd4play_beeper2_wl_1
// line 766
  extern sd4play_beeper3_drumCt, sd4play_beeper3_drumNr, sd4play_beeper3_drumAdd
// line 767
  extern sd4play_end
// line 768
  extern sd4play_restoreSP
// line 769
  extern tmp
// line 772
  push IX
// line 773
  BANK_VRAM_MMIO C
// line 776
  ld A, 0 + 0xf9
  ld (MMIO_8255_PORTA), A
// line 777
sd4play_waitUntilKeyOff1:
// line 778
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F1_MASK | KEY9_F2_MASK | KEY9_F4_MASK
  jr nz, 0 + sd4play_waitUntilKeyOff1
// line 781
  dec E
  jr z, sd4play__0
// line 782
      ld A, 0 + 0xfa
      ld (MMIO_8255_PORTA), A
sd4play__0:
// line 786
  ld A, 0 + MMIO_8253_CTRL_CT0(MMIO_8253_CTRL_RL_L_MASK, MMIO_8253_CTRL_MODE0_MASK)
// line 787
  ld (MMIO_8253_CTRL), A
// line 789
#if  0   
L00:
  ld A, (VRAM_TEXT)
// line 791
  ld A, 0 + 56/2
  ld (MMIO_8253_CT0), A
// line 792
  ex (SP), HL
  ex (SP), HL
  ex (SP), HL
  ex (SP), HL
// line 793
  jr 0 + L00
// line 794
#endif 
// line 808
  ld (sd4play_restoreSP + 1), SP
  ld SP, HL
// line 809
  xor A, A
// line 810
  ld (sd4PlayModDrum_ct_1        + 1), A
// line 811
  ld (sd4play_beeper3_drumAdd    + 0), A
// line 812
  ld (sd4play_modLead_volSpeed_1 + 1), A
// line 814
  inc A
// line 815
  ld IXH, A
  ld IXL, A
  ld IYL, A
  ld IYH, A
// line 816
  ld HL, 0 + ADDR_SD4_REP + 0xff
  ld (sd4play_rep_1 + 1), HL
// line 817
  ld A, 0 + JR
  ld (sd4play_leadSlide_0 + 0), A
// line 820
  xor A, A
  ld HL, 0 + MMIO_8255_PORTC
// line 821
sd4play_vblk1:
  or A, (HL)
  jp p, 0 + sd4play_vblk1
// line 822
  ld A, (VRAM_TEXT)
// line 823
  xor A, A
  ex AF, AF
// line 827
sd4play_loop:
// line 828
  ld A, (VRAM_TEXT)
  ex AF, AF
  ld (MMIO_8253_CT0), A
  ex AF, AF
// line 832
  SD4PLAY_NOTE_LEAD H, L, HL, D, IXH, INC_D, DEC_D, sd4play_rep_1, sd4play_leadSlide_0,         sd4play_modLead_start,         sd4play_modLead_vol, sd4play_modLead_volSpeed_1, sd4play_modLead_volEnd_1, sd4play_modLead_volInc_0,         sd4play_modLead_wl,  sd4play_modLead_wlSpeed_1,  sd4play_modLead_wlEnd_1,  sd4play_modLead_wlInc_0,         sd4play_beeper0_wl_1,         sd4play_modLead_end,         sd4play_end
// line 839
  SD4PLAY_NOTE_BASE B, C, BC, E, IXL, INC_E, DEC_E,         sd4play_modBase_start, sd4play_modBase_volEnd_1, sd4play_modBase_volInc_0,         sd4play_beeper1_wl_1
// line 842
  SD4PLAY_MOD_LEAD     L,     D, IXH,         sd4play_modLead_start,         sd4play_modLead_vol, sd4play_modLead_volSpeed_1, sd4play_modLead_volEnd_1, sd4play_modLead_volInc_0,         sd4play_modLead_wl,  sd4play_modLead_wlSpeed_1,  sd4play_modLead_wlEnd_1,  sd4play_modLead_wlInc_0,         sd4play_beeper0_wl_1,         sd4play_modLead_end
// line 848
  SD4PLAY_MOD_BASE            E, IXL,         sd4play_modBase_start, sd4play_modBase_volEnd_1, sd4play_modBase_volInc_0
// line 850
  ld A, (VRAM_TEXT)
  ex AF, AF
  ld (MMIO_8253_CT0), A
  ex AF, AF
// line 851
  exx
// line 852
  SD4PLAY_NOTE_CHORD2 B,  C,  BC,  D,  IYH, INC_D, DEC_D,             sd4play_modChord_wlStart,  sd4play_modChord_wl0, sd4play_modChord_wl1,             sd4play_modChord_volStart, sd4play_modChord_volEnd, sd4play_modChord_volInc,             sd4play_beeper2_wl_1
// line 856
  SD4PLAY_MOD_CHORD2     C,     D, IYH,             sd4play_modChord_wlStart,  sd4play_modChord_wl0, sd4play_modChord_wl1,             sd4play_modChord_volStart, sd4play_modChord_volEnd, sd4play_modChord_volInc,             sd4play_beeper2_wl_1
// line 860
  SD4PLAY_NOTE_DRUM              IYL,    L,             sd4play_beeper3_drumCt, sd4play_beeper3_drumNr, sd4play_beeper3_drumAdd
// line 862
  ld E, 0 + TEMPO
// line 863
  ld A, (VRAM_TEXT)
  ex AF, AF
  ld (MMIO_8253_CT0), A
// line 866
sd4play_beeperLoop:
// line 867
// line 868
    xor A, A
    ex AF, AF
// line 869
    exx
// line 871
    SD4PLAY_BEEPER_WAVE_R      H,  L,  D, sd4play_beeper0_wl_1
// line 872
    SD4PLAY_BEEPER_WAVE_BR     B,  C,  E, sd4play_beeper1_wl_1
// line 873
    exx
// line 874
    SD4PLAY_BEEPER_WAVE_BR B,  C,  D, sd4play_beeper2_wl_1
// line 875
    SD4PLAY_BEEPER_DRUM H, L, HL, E,                 sd4play_beeper3_drumCt, sd4play_beeper3_drumNr, sd4play_beeper3_drumAdd
// line 880
    ld H, 0 + ADDR_SD4_VTAB / 256
    ld L, A
// line 881
    ld A, (VRAM_TEXT)
    ld A, (HL)
    ld (MMIO_8253_CT0), A
// line 882
    inc E
    jp nz, 0 + sd4play_beeperLoop
// line 885
  exx
// line 886
  ex AF, AF
// line 888
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F1_MASK | KEY9_F2_MASK | KEY9_F4_MASK
  jp z, 0 + sd4play_loop
// line 889
  ld L, A
// line 890
  jr 0 + sd4play_waitUntilKeyOff2
// line 893
sd4play_end:
// line 894
  ld L, 0 + 0x00
// line 896
sd4play_waitUntilKeyOff2:
// line 897
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F1_MASK | KEY9_F2_MASK | KEY9_F4_MASK
  jr nz, 0 + sd4play_waitUntilKeyOff2
// line 900
  ld A, 0 + MMIO_8253_CT0_MODE3
  ld (MMIO_8253_CTRL), A
// line 902
  BANK_RAM C
// line 904
sd4play_restoreSP:
  ld SP, 0 + 0x0000
// line 905
  pop IX
// line 907
  ret
__endasm;
}

#pragma restore
