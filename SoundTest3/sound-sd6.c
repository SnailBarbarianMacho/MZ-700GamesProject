/**** This file is made by ../tools/z80ana.php.  DO NOT MODIFY! ****/





#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "../src-common/asm_macros.h"

#include "addr.h"
#include "sound.h"



    STATIC_ASSERT(SD6_B1 < 256 + 1, "valueOverflow"); 
    STATIC_ASSERT(SD6_C6 >= 15,     "valueUnderflow");

#if 0
    STATIC_ASSERT(SD6_WL(SD_FREQ_C2) < 128.0f, "valueOverflow");    
    STATIC_ASSERT(SD6_WL(SD_FREQ_C5) > 15.0f,  "valueUnderflow");   
    #define MMIO_8253_CT0_PWM   MMIO_8253_CTRL_CT0(MMIO_8253_CTRL_RL_L_MASK, MMIO_8253_CTRL_MODE0_MASK)
#endif


#define BEEPER1_WL_CT_VAL   8       
#define BEEPER1_SYNC        true    
#define TEMPO               0       
#define BASE_VOL_SPEED_R    7       
#define CHORD2_VOL_SPEED_R  7       


void LEAD_ECHO_DELAY(void)  __naked
{
__asm
LEAD_ECHO_DELAY macro void
// line 38
  dec A
endm
__endasm;
}



#ifndef SD6_EXCEPT_LEN
#define SD6_EXCEPT_LEN      1
#endif
#ifndef SD6_EXCEPT_LEN_SZ
#define SD6_EXCEPT_LEN_SZ   1
#endif




#define JR          0x18
#define JR_NZ       0x20
#define JR_Z        0x28

#define LD_A_N      0x3e    
#define INC_A       0x3c
#define DEC_A       0x3d




#define PULSE_L MMIO_8253_CT0_MODE0
#define PULSE_H MMIO_8253_CT0_MODE3


#pragma disable_warning 85
#pragma save



static const u8 SD6_DATA_[] = {
#include "music/sd6_data.h"
};





void sd6Init(void)  __naked
{
__asm
// line 83
  extern _SD6_DATA_
// line 84
  extern sd6Init_dataLoop
// line 87
  ld HL, 0 + _SD6_DATA_
// line 88
  ld DE, 0 + ADDR_SD6_DRUM
// line 89
  ld BC, 0 + SZ_SD6_DRUM
// line 90
  ldir
// line 92
  ld HL, 0 + ADDR_SD6_DRUM
// line 93
  ld B, 0 + SZ_SD6_VTAB & 0xff
// line 94
sd6Init_dataLoop:
// line 95
    ld A, (HL)
    and A, 0 + 0x1f
    ld (DE), A
// line 96
    xor A, (HL)
    rept 4
      rlca
    endr
    ld (HL), A
// line 97
    inc L
    inc E
// line 98
    djnz B, 0 + sd6Init_dataLoop
// line 102
  BANK_VRAM_MMIO C
// line 103
  ld HL, 0 + MMIO_ETC
// line 104
  ld (HL), 0 + MMIO_ETC_GATE_MASK
// line 105
  dec L
// line 106
  ld (HL), 0 + MMIO_8253_CT0_MODE3
// line 107
  BANK_RAM C
// line 109
  ret
__endasm;
}



















void SD6PLAY_NOTE_LEAD(
    int reg_wlc, int reg_slc,
    int reg_tmp_h, int reg_tmp_l, int reg_tmp_hl,
    int addr_rep_1,
    int addr_vol_1, int addr_wl_1, int addr_slide_0, int addr_mod_vol_speed_1,
    int addr_beeper1_sync_0,
    int addr_end
)  __naked
{
__asm
SD6PLAY_NOTE_LEAD macro reg_wlc, reg_slc, reg_tmp_h, reg_tmp_l, reg_tmp_hl, addr_rep_1, addr_vol_1, addr_wl_1, addr_slide_0, addr_mod_vol_speed_1, addr_beeper1_sync_0, addr_end
local SD6PLAY_NOTE_LEAD__0, SD6PLAY_NOTE_LEAD__1, SD6PLAY_NOTE_LEAD__2, SD6PLAY_NOTE_LEAD__3, SD6PLAY_NOTE_LEAD__4, SD6PLAY_NOTE_LEAD__5, SD6PLAY_NOTE_LEAD__6, SD6PLAY_NOTE_LEAD__7, SD6PLAY_NOTE_LEAD__8, SD6PLAY_NOTE_LEAD__9, SD6PLAY_NOTE_LEAD__10, SD6PLAY_NOTE_LEAD__11, SD6PLAY_NOTE_LEAD__12, SD6PLAY_NOTE_LEAD__13, SD6PLAY_NOTE_LEAD__14, SD6PLAY_NOTE_LEAD__15, SD6PLAY_NOTE_LEAD__16
// line 139
  extern sd6playNoteLead_pop, sd6playNoteLead_rep_2, sd6playNoteLead_initEnd, sd6playNoteLead_initEndSlur, sd6playNoteLead_modBranch, sd6playNoteLead_modWl, sd6playNoteLead_modWlEnd_1, sd6playNoteLead_modWlSpeed_1, sd6playNoteLead_modWlAdd_0, sd6playNoteLead_modVol, sd6playNoteLead_modVolVal_1, sd6playNoteLead_modVolEnd_1, sd6playNoteLead_modVolInc_0, sd6playNoteLead_modEnd
// line 149
  dec reg_slc
  jp nz, SD6PLAY_NOTE_LEAD__0
// line 150
sd6playNoteLead_pop:
// line 151
      pop reg_tmp_hl
// line 154
      ld A, reg_tmp_l
      add A, A
      jp c, SD6PLAY_NOTE_LEAD__1
// line 155
          add A, A
          ld reg_slc, A
          jr c, SD6PLAY_NOTE_LEAD__2
// line 157
              cp A, 0 + SD6_EXCEPT_LEN
              jp c, SD6PLAY_NOTE_LEAD__3
// line 158
                  cp A, 0 + SD6_EXCEPT_LEN + 4 * 7
                  jp nc, SD6PLAY_NOTE_LEAD__4
// line 159
                      dec SP
// line 160
                      sub A, 0 + SD6_EXCEPT_LEN - 8
// line 161
                      rept 2
                        rrca
                      endr
// line 162
                      ld HL, (addr_rep_1 + 1)
// line 163
                      inc L
                      ld (sd6playNoteLead_rep_2 + 2), HL
sd6playNoteLead_rep_2:
                      ld (0x0000), SP
// line 164
                      inc L
                      inc L
                      ld (HL), A
// line 165
                      ld (addr_rep_1 + 1), HL
// line 166
                      jp sd6playNoteLead_pop
SD6PLAY_NOTE_LEAD__4:
SD6PLAY_NOTE_LEAD__3:
// line 171
              dec SP
// line 172
              xor A, A
              ld (addr_wl_1  + 1), A
// line 173
              inc A
              ld (addr_vol_1 + 1), A
// line 174
              ld HL, 0 + sd6playNoteLead_modEnd
// line 175
              jp sd6playNoteLead_initEndSlur
SD6PLAY_NOTE_LEAD__2:
// line 179
          cp A, 0 + SD6_EXCEPT_LEN
          jr nz, SD6PLAY_NOTE_LEAD__5
// line 180
addr_rep_1:
              ld HL, 0 + 0x0000
              dec (HL)
              jr nz, SD6PLAY_NOTE_LEAD__6
// line 181
                  dec L
                  dec L
                  dec L
                  ld (addr_rep_1 + 1), HL
// line 182
                  dec SP
// line 183
                  jp sd6playNoteLead_pop
SD6PLAY_NOTE_LEAD__6:
// line 186
              dec L
              ld A, (HL)
              dec L
              ld L, (HL)
              ld H, A
              ld SP, HL
// line 187
              jp sd6playNoteLead_pop
SD6PLAY_NOTE_LEAD__5:
// line 191
// line 192
            cp A, 0 + SD6_EXCEPT_LEN + 4
            jp z, 0 + addr_end
// line 196
// line 197
            ld A, reg_tmp_h
            ld (addr_wl_1 + 1), A
// line 198
            ld reg_tmp_h, 0 + ADDR_SD6_VTAB / 256
            ld reg_tmp_l, A
            ld A, (reg_tmp_hl)
// line 199
            ld (           sd6playNoteLead_modVolEnd_1 + 1), A
// line 200
            ld A, 0 + 2
            ld (sd6playNoteLead_modVolVal_1 + 1), A
// line 201
            ld (addr_vol_1 + 1), A
// line 202
            ld A, 0 + INC_A
            ld (sd6playNoteLead_modVolInc_0    ), A
// line 203
            ld HL, 0 + sd6playNoteLead_modVol
// line 204
            jp sd6playNoteLead_initEnd
SD6PLAY_NOTE_LEAD__1:
// line 207
      add A, A
      ld reg_slc, A
      jr c, SD6PLAY_NOTE_LEAD__7
// line 209
          cp A, 0 + SD6_EXCEPT_LEN
          jp c, SD6PLAY_NOTE_LEAD__8
// line 210
              cp A, 0 + SD6_EXCEPT_LEN + 4 * 4
              jp nc, SD6PLAY_NOTE_LEAD__9
// line 211
                  sub A, 0 + SD6_EXCEPT_LEN - 1
// line 212
                  ld reg_tmp_h, 0 + ADDR_SD6_VTAB / 256
                  ld reg_tmp_l, A
                  ld A, (reg_tmp_hl)
// line 213
                  ld (addr_mod_vol_speed_1 + 1), A
// line 214
                  dec SP
// line 215
                  jp sd6playNoteLead_pop
SD6PLAY_NOTE_LEAD__9:
SD6PLAY_NOTE_LEAD__8:
// line 220
// line 221
            ld A, reg_tmp_h
            ld (addr_wl_1 + 1), A
// line 222
            ld reg_tmp_h, 0 + ADDR_SD6_VTAB / 256
            ld reg_tmp_l, A
            ld A, (reg_tmp_hl)
// line 223
            ld (           sd6playNoteLead_modVolVal_1 + 1), A
// line 224
            ld (addr_vol_1                             + 1), A
// line 225
            ld A, 0 + 2
            ld (sd6playNoteLead_modVolEnd_1 + 1), A
// line 226
            ld A, 0 + DEC_A
            ld (sd6playNoteLead_modVolInc_0    ), A
// line 227
            ld HL, 0 + sd6playNoteLead_modVol
// line 228
            jp sd6playNoteLead_initEnd
SD6PLAY_NOTE_LEAD__7:
// line 232
      cp A, 0 + SD6_EXCEPT_LEN
      jp c, SD6PLAY_NOTE_LEAD__10
// line 234
          jr nz, SD6PLAY_NOTE_LEAD__11
// line 235
              ld A, 0 + JR
              ld (addr_slide_0), A
// line 236
              dec SP
// line 237
              jp sd6playNoteLead_pop
SD6PLAY_NOTE_LEAD__11:
// line 241
          cp A, 0 + SD6_EXCEPT_LEN + 4 * 4
          jp nc, SD6PLAY_NOTE_LEAD__12
// line 242
              sub A, 0 + SD6_EXCEPT_LEN - 2
// line 243
              ld reg_tmp_h, 0 + ADDR_SD6_VTAB / 256
              ld reg_tmp_l, A
              ld A, (reg_tmp_hl)
// line 244
              ld (sd6playNoteLead_modWlSpeed_1 + 1), A
// line 245
              ld A, 0 + LD_A_N
              ld (addr_slide_0), A
// line 246
              dec SP
// line 247
              jp sd6playNoteLead_pop
SD6PLAY_NOTE_LEAD__12:
SD6PLAY_NOTE_LEAD__10:
// line 252
addr_slide_0:
      jr nz, SD6PLAY_NOTE_LEAD__13
// line 253
          ld A, reg_tmp_h
          ld (sd6playNoteLead_modWlEnd_1 + 1), A
// line 255
          ld A, (addr_wl_1 + 1)
          cp A, reg_tmp_h
          ld A, 0 + INC_A
          jr c, SD6PLAY_NOTE_LEAD__14
              inc A
SD6PLAY_NOTE_LEAD__14:
// line 256
          ld (sd6playNoteLead_modWlAdd_0), A
// line 257
          ld HL, 0 + sd6playNoteLead_modWl
// line 258
          jp sd6playNoteLead_initEndSlur
SD6PLAY_NOTE_LEAD__13:
// line 262
// line 263
        ld A, reg_tmp_h
        ld (addr_wl_1 + 1), A
// line 264
        ld reg_tmp_h, 0 + ADDR_SD6_VTAB / 256
        ld reg_tmp_l, A
        ld A, (reg_tmp_hl)
// line 265
        ld (addr_vol_1               + 1), A
// line 266
        ld HL, 0 + sd6playNoteLead_modEnd
// line 270
sd6playNoteLead_initEnd:
// line 271
      ld reg_wlc, 0 + 1
// line 272
#if  BEEPER1_SYNC
      ld A, 0 + LD_A_N
      ld (addr_beeper1_sync_0), A
// line 274
#endif 
// line 276
sd6playNoteLead_initEndSlur:
// line 277
      ld (sd6playNoteLead_modBranch + 1), HL
SD6PLAY_NOTE_LEAD__0:
// line 281
sd6playNoteLead_modBranch:
  jp 0 + sd6playNoteLead_modEnd
// line 284
sd6playNoteLead_modWl:
// line 285
  ld A, reg_slc
sd6playNoteLead_modWlSpeed_1:
  and A, 0 + 0x00
  jr nz, 0 + sd6playNoteLead_modEnd
// line 286
    ld A, (addr_wl_1 + 1)
sd6playNoteLead_modWlEnd_1:
    cp A, 0 + 0x00
// line 287
    jr z, 0 + sd6playNoteLead_modEnd
sd6playNoteLead_modWlAdd_0:
    inc A
    ld (addr_wl_1 + 1), A
// line 288
    ld reg_tmp_h, 0 + ADDR_SD6_VTAB / 256
    ld reg_tmp_l, A
    ld A, (reg_tmp_hl)
// line 289
    ld (addr_vol_1 + 1), A
// line 290
    jp sd6playNoteLead_modEnd
// line 294
sd6playNoteLead_modVol:
// line 295
  ld A, reg_slc
addr_mod_vol_speed_1:
  and A, 0 + 0x00
  jp nz, SD6PLAY_NOTE_LEAD__15
// line 296
sd6playNoteLead_modVolVal_1:
      ld A, 0 + 0x00
sd6playNoteLead_modVolEnd_1:
      cp A, 0 + 0x00
// line 297
      jr z, SD6PLAY_NOTE_LEAD__16
sd6playNoteLead_modVolInc_0:
          inc A
          ld (sd6playNoteLead_modVolVal_1 + 1), A
SD6PLAY_NOTE_LEAD__16:
// line 298
      ld (addr_vol_1 + 1), A
SD6PLAY_NOTE_LEAD__15:
// line 302
sd6playNoteLead_modEnd:
endm
__endasm;
}













void SD6PLAY_NOTE_BASE(
    int reg_wlc, int reg_slc,
    int reg_tmp_h, int reg_tmp_l, int reg_tmp_hl,
    int addr_vol_1, int addr_wl_1
)  __naked
{
__asm
SD6PLAY_NOTE_BASE macro reg_wlc, reg_slc, reg_tmp_h, reg_tmp_l, reg_tmp_hl, addr_vol_1, addr_wl_1
local SD6PLAY_NOTE_BASE__0, SD6PLAY_NOTE_BASE__1, SD6PLAY_NOTE_BASE__2, SD6PLAY_NOTE_BASE__3, SD6PLAY_NOTE_BASE__4, SD6PLAY_NOTE_BASE__5, SD6PLAY_NOTE_BASE__6, SD6PLAY_NOTE_BASE__7
// line 323
  extern sd6playNoteBase_fl, sd6playNoteBase_initEnd, sd6playNoteBase_modStart, sd6playNoteBase_modVol, sd6playNoteBase_modVolEnd, sd6playNoteBase_modVolInc, sd6playNoteBase_modVolConv, sd6playNoteBase_modEnd
// line 333
  dec reg_slc
  jp nz, SD6PLAY_NOTE_BASE__0
// line 334
      pop reg_tmp_hl
// line 337
      ld A, reg_tmp_l
      add A, A
      jp c, SD6PLAY_NOTE_BASE__1
// line 338
          add A, A
          ld reg_slc, A
          jr c, SD6PLAY_NOTE_BASE__2
// line 340
              dec SP
// line 341
              xor A, A
              ld (addr_wl_1  + 1), A
// line 342
              inc A
              ld (      addr_vol_1 + 1), A
// line 343
              ld A, 0 + JR
// line 344
              jp sd6playNoteBase_initEnd
SD6PLAY_NOTE_BASE__2:
// line 346
// line 348
            ld A, reg_tmp_h
            ld (addr_wl_1             + 1), A
// line 349
            ld reg_tmp_h, 0 + ADDR_SD6_VTAB / 256
            ld reg_tmp_l, A
            ld A, (reg_tmp_hl)
// line 350
            ld (           sd6playNoteBase_modVolEnd + 1), A
// line 351
            ld A, 0 + 2
            ld (sd6playNoteBase_modVol    + 1), A
// line 352
            ld (           addr_vol_1                + 1), A
// line 353
            ld A, 0 + INC_A
            ld (sd6playNoteBase_modVolInc    ), A
// line 354
            ld A, 0 + LD_A_N
        jp SD6PLAY_NOTE_BASE__3
SD6PLAY_NOTE_BASE__1:
// line 357
          add A, A
          ld reg_slc, A
// line 358
          ld A, reg_tmp_h
          ld (addr_wl_1 + 1), A
// line 359
          ld reg_tmp_h, 0 + ADDR_SD6_VTAB / 256
          ld reg_tmp_l, A
          ld A, (reg_tmp_hl)
// line 360
          jr c, SD6PLAY_NOTE_BASE__4
// line 362
              ld (           sd6playNoteBase_modVol    + 1), A
// line 363
              ld (           addr_vol_1                + 1), A
// line 364
              ld A, 0 + 2
              ld (sd6playNoteBase_modVolEnd + 1), A
// line 365
              ld A, 0 + DEC_A
              ld (sd6playNoteBase_modVolInc    ), A
// line 366
              ld A, 0 + LD_A_N
            jp SD6PLAY_NOTE_BASE__5
SD6PLAY_NOTE_BASE__4:
// line 369
              ld (addr_vol_1 + 1), A
// line 370
              ld A, 0 + JR
SD6PLAY_NOTE_BASE__5:
SD6PLAY_NOTE_BASE__3:
// line 373
sd6playNoteBase_initEnd:
// line 374
      ld (sd6playNoteBase_modStart), A
// line 375
      ld reg_wlc, 0 + 1
SD6PLAY_NOTE_BASE__0:
// line 379
sd6playNoteBase_modStart:
// line 380
  jr 0 + sd6playNoteBase_modEnd
// line 383
  ld A, reg_slc
  and A, 0 + BASE_VOL_SPEED_R
  jp nz, SD6PLAY_NOTE_BASE__6
// line 384
sd6playNoteBase_modVol:
      ld A, 0 + 0x00
sd6playNoteBase_modVolEnd:
      cp A, 0 + 0x00
// line 385
      jr z, SD6PLAY_NOTE_BASE__7
sd6playNoteBase_modVolInc:
          inc A
          ld (sd6playNoteBase_modVol + 1), A
SD6PLAY_NOTE_BASE__7:
// line 386
      ld (addr_vol_1 + 1), A
SD6PLAY_NOTE_BASE__6:
// line 390
sd6playNoteBase_modEnd:
endm
__endasm;
}

















void SD6PLAY_NOTE_CHORD2(
    int reg_wlc0, int reg_wlc1, int reg_wlc10, int reg_slc,
    int reg_tmp_h, int reg_tmp_l, int reg_tmp_hl,
    int reg_vol01, int addr_wl0_1, int addr_wl1_1
)  __naked
{
__asm
SD6PLAY_NOTE_CHORD2 macro reg_wlc0, reg_wlc1, reg_wlc10, reg_slc, reg_tmp_h, reg_tmp_l, reg_tmp_hl, reg_vol01, addr_wl0_1, addr_wl1_1
local SD6PLAY_NOTE_CHORD2__0, SD6PLAY_NOTE_CHORD2__1, SD6PLAY_NOTE_CHORD2__2, SD6PLAY_NOTE_CHORD2__3, SD6PLAY_NOTE_CHORD2__4, SD6PLAY_NOTE_CHORD2__5, SD6PLAY_NOTE_CHORD2__6, SD6PLAY_NOTE_CHORD2__7
// line 415
  extern sd6playNoteChord2_initEnd, sd6playNoteChord2_modStart_0, sd6playNoteChord2_modVol_1, sd6playNoteChord2_modVolEnd_1, sd6playNoteChord2_modVolInc_0, sd6playNoteChord2_modEnd
// line 425
  dec reg_slc
  jp nz, SD6PLAY_NOTE_CHORD2__0
// line 426
      dec SP
      pop AF
// line 428
      add A, A
      jp c, SD6PLAY_NOTE_CHORD2__1
// line 429
          add A, A
          ld reg_slc, A
          jr c, SD6PLAY_NOTE_CHORD2__2
// line 431
              xor A, A
              ld (addr_wl0_1 + 1), A
// line 432
              ld (           addr_wl1_1 + 1), A
// line 434
              ld reg_vol01, A
// line 436
              ld A, 0 + JR
// line 437
              jp sd6playNoteChord2_initEnd
SD6PLAY_NOTE_CHORD2__2:
// line 439
// line 440
            pop reg_wlc10
// line 443
            ld A, reg_wlc1
            ld (addr_wl1_1 + 1), A
// line 444
            ld A, reg_wlc0
            ld (addr_wl0_1 + 1), A
// line 446
            ld reg_tmp_h, 0 + ADDR_SD6_VTAB / 256
            ld reg_tmp_l, A
            ld A, (reg_tmp_hl)
// line 447
            ld (sd6playNoteChord2_modVolEnd_1 + 1), A
// line 449
            ld A, 0 + 2
// line 450
            ld (sd6playNoteChord2_modVol_1 + 1), A
// line 451
            ld reg_vol01, A
// line 453
            ld A, 0 + INC_A
// line 454
            ld (sd6playNoteChord2_modVolInc_0), A
// line 456
            ld A, 0 + LD_A_N
        jp SD6PLAY_NOTE_CHORD2__3
SD6PLAY_NOTE_CHORD2__1:
// line 459
          add A, A
          ld reg_slc, A
// line 460
          pop reg_wlc10
// line 462
          ld A, reg_wlc0
          ld (addr_wl0_1 + 1), A
// line 463
          ld reg_tmp_h, 0 + ADDR_SD6_VTAB / 256
          ld reg_tmp_l, A
          ld A, (reg_tmp_hl)
// line 464
          ld reg_vol01, A
// line 466
          jr c, SD6PLAY_NOTE_CHORD2__4
// line 468
              ld (sd6playNoteChord2_modVol_1 + 1), A
// line 470
              ld A, reg_wlc1
              ld (addr_wl1_1 + 1), A
// line 472
              ld A, 0 + 2
// line 473
              ld (sd6playNoteChord2_modVolEnd_1 + 1), A
// line 475
              ld A, 0 + DEC_A
// line 476
              ld (sd6playNoteChord2_modVolInc_0), A
// line 478
              ld A, 0 + LD_A_N
            jp SD6PLAY_NOTE_CHORD2__5
SD6PLAY_NOTE_CHORD2__4:
// line 481
              ld A, reg_wlc1
              ld (addr_wl1_1 + 1), A
// line 483
              ld A, 0 + JR
SD6PLAY_NOTE_CHORD2__5:
SD6PLAY_NOTE_CHORD2__3:
// line 486
sd6playNoteChord2_initEnd:
// line 487
      ld (sd6playNoteChord2_modStart_0), A
// line 488
      ld reg_wlc0, 0 + 1
// line 489
      ld reg_wlc1, reg_wlc0
SD6PLAY_NOTE_CHORD2__0:
// line 493
sd6playNoteChord2_modStart_0:
  jr 0 + sd6playNoteChord2_modEnd
// line 495
  ld A, reg_slc
  and A, 0 + CHORD2_VOL_SPEED_R
  jp nz, SD6PLAY_NOTE_CHORD2__6
// line 497
sd6playNoteChord2_modVol_1:
      ld A, 0 + 0x00
sd6playNoteChord2_modVolEnd_1:
      cp A, 0 + 0x00
// line 498
      jr z, SD6PLAY_NOTE_CHORD2__7
sd6playNoteChord2_modVolInc_0:
          inc A
          ld (sd6playNoteChord2_modVol_1 + 1), A
SD6PLAY_NOTE_CHORD2__7:
// line 499
      ld reg_vol01, A
SD6PLAY_NOTE_CHORD2__6:
// line 503
sd6playNoteChord2_modEnd:
endm
__endasm;
}










void SD6PLAY_NOTE_DRUM(int reg_sl, int reg_tmp, int addr_drum_nr_1)  __naked
{
__asm
SD6PLAY_NOTE_DRUM macro reg_sl, reg_tmp, addr_drum_nr_1
local SD6PLAY_NOTE_DRUM__0
// line 518
  ld A, 0 + 0x46
// line 519
  dec reg_sl
  jp nz, SD6PLAY_NOTE_DRUM__0
// line 520
      dec SP
      pop AF
// line 521
      ld reg_tmp, A
      and A, 0 + 0xfc
      ld reg_sl, A
// line 522
      xor A, reg_tmp
// line 523
      rept 3
        rlca
      endr
      or A, 0 + 0x46
SD6PLAY_NOTE_DRUM__0:
// line 525
  ld (addr_drum_nr_1 + 1), A
endm
__endasm;
}











void SD6PLAY_BEEPER_WAVE(int reg_wl, int reg_pulse, int addr_vol, int addr_wl_1)  __naked
{
__asm
SD6PLAY_BEEPER_WAVE macro reg_wl, reg_pulse, addr_vol, addr_wl_1
local SD6PLAY_BEEPER_WAVE__0, SD6PLAY_BEEPER_WAVE__1, SD6PLAY_BEEPER_WAVE__2
// line 542
  dec reg_wl
  jp nz, SD6PLAY_BEEPER_WAVE__0
// line 543
addr_wl_1:
      ld reg_wl, 0 + 0x00
    jp SD6PLAY_BEEPER_WAVE__1
SD6PLAY_BEEPER_WAVE__0:
// line 546
      ld A, reg_wl
addr_vol_1:
      cp A, 0 + 0x02
      jr nc, SD6PLAY_BEEPER_WAVE__2
          ld reg_pulse, 0 + PULSE_H
SD6PLAY_BEEPER_WAVE__2:
SD6PLAY_BEEPER_WAVE__1:
endm
__endasm;
}










void SD6PLAY_BEEPER_WAVE_R(int reg_wl, int reg_pulse, int reg_vol, int addr_wl_1)  __naked
{
__asm
SD6PLAY_BEEPER_WAVE_R macro reg_wl, reg_pulse, reg_vol, addr_wl_1
local SD6PLAY_BEEPER_WAVE_R__0, SD6PLAY_BEEPER_WAVE_R__1, SD6PLAY_BEEPER_WAVE_R__2
// line 564
  dec reg_wl
  jp nz, SD6PLAY_BEEPER_WAVE_R__0
// line 565
addr_wl_1:
      ld reg_wl, 0 + 0x00
    jp SD6PLAY_BEEPER_WAVE_R__1
SD6PLAY_BEEPER_WAVE_R__0:
// line 568
      ld A, reg_wl
      cp A, reg_vol
      jr nc, SD6PLAY_BEEPER_WAVE_R__2
          ld reg_pulse, 0 + PULSE_H
SD6PLAY_BEEPER_WAVE_R__2:
SD6PLAY_BEEPER_WAVE_R__1:
endm
__endasm;
}










void SD6PLAY_BEEPER_WAVE_BR(int reg_wl, int reg_pulse, int reg_vol, int addr_wl_1)  __naked
{
__asm
SD6PLAY_BEEPER_WAVE_BR macro reg_wl, reg_pulse, reg_vol, addr_wl_1
local SD6PLAY_BEEPER_WAVE_BR__0
// line 584
local sd6play_beeperWaveBr, sd6play_beeperWaveBrEnd
// line 585
  extern sd6play_beeperWaveBr, sd6play_beeperWaveBrEnd
// line 588
  djnz B, 0 + sd6play_beeperWaveBr
// line 589
addr_wl_1:
    ld reg_wl, 0 + 0x00
    jr 0 + sd6play_beeperWaveBrEnd
// line 591
sd6play_beeperWaveBr:
// line 593
    ld A, reg_wl
    cp A, reg_vol
    jr nc, SD6PLAY_BEEPER_WAVE_BR__0
        ld reg_pulse, 0 + PULSE_H
SD6PLAY_BEEPER_WAVE_BR__0:
// line 595
sd6play_beeperWaveBrEnd:
endm
__endasm;
}










void SD6PLAY_BEEPER_DRUM(int reg_wav, int reg_pulse, int addr_drum_nr)  __naked
{
__asm
SD6PLAY_BEEPER_DRUM macro reg_wav, reg_pulse, addr_drum_nr
local SD6PLAY_BEEPER_DRUM__0
// line 611
addr_drum_nr:
  bit 0, (reg_wav)
  jr z, SD6PLAY_BEEPER_DRUM__0
      ld reg_pulse, 0 + PULSE_H
SD6PLAY_BEEPER_DRUM__0:
endm
__endasm;
}




u8 sd6play(u32 param)  __z88dk_fastcall __naked
{
__asm
// line 620
  extern sd6play_waitUntilKeyOff1
// line 621
  extern sd6play_waitUntilKeyOff2
// line 622
  extern sd6Play_rep_1
// line 623
  extern sd6play_setVols, sd6play_loop, sd6play_beeperLoop
// line 624
  extern sd6play_beeper0_wl, sd6play_slide0, sd6play_modVolSpeed0
// line 625
  extern sd6play_beeper1_wl
// line 626
  extern sd6play_beeper2_wl
// line 627
  extern sd6play_beeper3_wl
// line 628
  extern sd6play_beeper4_wl
// line 629
  extern sd6play_beeper5_drumNr
// line 630
  extern sd6play_beeper1_sync, sd6play_beeper1_syncEnd
// line 631
  extern sd6play_end
// line 632
  extern sd6play_restoreSP
// line 633
  extern tmp
// line 636
  push IX
// line 637
  BANK_VRAM_MMIO C
// line 640
  ld A, 0 + 0xf9
  ld (MMIO_8255_PORTA), A
// line 641
sd6play_waitUntilKeyOff1:
// line 642
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F1_MASK | KEY9_F2_MASK | KEY9_F4_MASK
  jr nz, 0 + sd6play_waitUntilKeyOff1
// line 645
  dec E
  jr z, sd6play__0
// line 646
      ld A, 0 + 0xfa
      ld (MMIO_8255_PORTA), A
sd6play__0:
// line 661
  ld (sd6play_restoreSP + 1), SP
  ld SP, HL
// line 662
  xor A, A
// line 663
  ld C, 0 + BEEPER1_WL_CT_VAL
  ld (sd6play_beeper1_wl + 1), A
// line 664
  inc A
// line 665
  ld IXH, A
  ld IXL, A
  ld IYL, A
  ld IYH, A
// line 666
  ld A, 0 + 7
  ld (sd6play_modVolSpeed0 + 1), A
// line 667
  ld A, 0 + JR
  ld (sd6play_slide0          ), A
// line 668
  ld HL, 0 + ADDR_SD6_REP + 0xff
  ld (sd6Play_rep_1 + 1), HL
// line 669
#if  BEEPER1_SYNC
  ld (sd6play_beeper1_sync            ), A
// line 671
#endif 
// line 675
sd6play_loop:
// line 678
  SD6PLAY_NOTE_LEAD B,  IXH, H, L, HL,         sd6Play_rep_1,         sd6play_setVols + 1, sd6play_beeper0_wl, sd6play_slide0, sd6play_modVolSpeed0, sd6play_beeper1_sync, sd6play_end
// line 681
  SD6PLAY_NOTE_BASE D,  IXL, H, L, HL,         sd6play_setVols,     sd6play_beeper2_wl
// line 683
sd6play_setVols:
  ld HL, 0 + 0x0000
// line 684
  exx
// line 685
  SD6PLAY_NOTE_CHORD2 B,   C,   BC,   IYH, H, L, HL, D,    sd6play_beeper3_wl, sd6play_beeper4_wl
// line 686
  SD6PLAY_NOTE_DRUM                   IYL,    L,     sd6play_beeper5_drumNr
// line 687
  ld HL, 0 + (ADDR_SD6_DRUM) | TEMPO
// line 690
sd6play_beeperLoop:
// line 691
// line 692
    exx
// line 695
    ld E, 0 + PULSE_L
// line 696
    SD6PLAY_BEEPER_WAVE_BR     B,  E,    H,  sd6play_beeper0_wl
// line 697
    SD6PLAY_BEEPER_WAVE_R      C,  E,    H,  sd6play_beeper1_wl
// line 698
    SD6PLAY_BEEPER_WAVE_R      D,  E,    L,  sd6play_beeper2_wl
// line 699
    ld A, E
    exx
    ld E, A
// line 700
    SD6PLAY_BEEPER_WAVE_BR B,  E,    D,  sd6play_beeper3_wl
// line 701
    SD6PLAY_BEEPER_WAVE_R  C,  E,    D,  sd6play_beeper4_wl
// line 702
    SD6PLAY_BEEPER_DRUM HL,    E,    sd6play_beeper5_drumNr
// line 704
    ld A, E
    ld (MMIO_8253_CTRL), A
// line 705
    inc L
    jp nz, 0 + sd6play_beeperLoop
// line 708
  exx
// line 711
  ld A, IXH
  and A, 0 + 3
  LEAD_ECHO_DELAY 
  jr nz, sd6play__1
// line 713
#if  BEEPER1_SYNC                                        
// line 714
sd6play_beeper1_sync:
      jr 0 + sd6play_beeper1_syncEnd
// line 715
        ld C, 0 + BEEPER1_WL_CT_VAL
// line 716
        ld A, 0 + JR
        ld (sd6play_beeper1_sync), A
// line 718
sd6play_beeper1_syncEnd:
// line 719
#endif 
      ld A, (sd6play_beeper0_wl + 1)
      ld (sd6play_beeper1_wl + 1), A
sd6play__1:
// line 723
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F1_MASK | KEY9_F2_MASK | KEY9_F4_MASK
  jp z, 0 + sd6play_loop
// line 724
  ld L, A
// line 725
  jr 0 + sd6play_waitUntilKeyOff2
// line 728
sd6play_end:
// line 729
  ld L, 0 + 0x00
// line 731
sd6play_waitUntilKeyOff2:
// line 732
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F1_MASK | KEY9_F2_MASK | KEY9_F4_MASK
  jr nz, 0 + sd6play_waitUntilKeyOff2
// line 735
  ld A, 0 + MMIO_8253_CT0_MODE3
  ld (MMIO_8253_CTRL), A
// line 737
  BANK_RAM C
// line 739
sd6play_restoreSP:
  ld SP, 0 + 0x0000
// line 740
  pop IX
// line 742
  ret
__endasm;
}

#pragma restore
