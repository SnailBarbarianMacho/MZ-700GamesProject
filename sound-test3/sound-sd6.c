/**** This file is made by ../tools/aal80.php.  DO NOT MODIFY! ****/





#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "../src-common/asm-macros.h"

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
#define SD6_EXCEPT_LEN      0
#endif


#define PULSE_L MMIO_8253_CT0_MODE0
#define PULSE_H MMIO_8253_CT0_MODE3


#pragma save
#pragma disable_warning 85          
#pragma disable_warning 59          



static u8 const SD6_DATA_[] = {
#include "music/sd6-data.h"
};





void sd6Init(void) 
{
__asm
  // line 70
  extern _SD6_DATA_
  // line 71
  extern sd6Init_dataLoop
  // line 74
  ld HL, 0 + _SD6_DATA_
  // line 75
  ld DE, 0 + ADDR_SD6_DRUM
  // line 76
  ld BC, 0 + SZ_SD6_DRUM
  // line 77
  ldir
  // line 79
  ld HL, 0 + ADDR_SD6_DRUM
  // line 80
  ld B, 0 + SZ_SD6_TAB & 0xff
  // line 81
sd6Init_dataLoop:
    // line 82
    ld A, (HL)
    and A, 0 + 0x1f
    ld (DE), A
    // line 83
    xor A, (HL)
    ld (HL), A
    // line 84
    inc L
    inc E
  djnz B, 0 + sd6Init_dataLoop
  // line 88
  BANKH_VRAM_MMIO C
  // line 89
  ld HL, 0 + MMIO_ETC
  // line 90
  ld (HL), 0 + MMIO_ETC_GATE_MASK
  // line 91
  dec L
  // line 92
  ld (HL), 0 + MMIO_8253_CT0_MODE3
  // line 93
  BANKH_RAM C
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
  // line 123
  extern sd6playNoteLead_pop, sd6playNoteLead_rep_2, sd6playNoteLead_initEnd, sd6playNoteLead_initEndSlur, sd6playNoteLead_modBranch, sd6playNoteLead_modWl, sd6playNoteLead_modWlEnd_1, sd6playNoteLead_modWlSpeed_1, sd6playNoteLead_modWlAdd_0, sd6playNoteLead_modVol, sd6playNoteLead_modVolVal_1, sd6playNoteLead_modVolEnd_1, sd6playNoteLead_modVolInc_0, sd6playNoteLead_modEnd
  // line 133
  dec reg_slc
  jp nz, SD6PLAY_NOTE_LEAD__0 // if (z) {
      // line 134
sd6playNoteLead_pop:
      // line 135
      pop reg_tmp_hl
      // line 138
      ld A, reg_tmp_l
      add A, A
      jp c, SD6PLAY_NOTE_LEAD__1 // if (nc) {
          // line 139
          add A, A
          ld reg_slc, A
          jr c, SD6PLAY_NOTE_LEAD__2 // if (nc_jr) {
              // line 141
              cp A, 0 + SD6_EXCEPT_LEN
              jp c, SD6PLAY_NOTE_LEAD__3 // if (nc) {
                  // line 142
                  cp A, 0 + SD6_EXCEPT_LEN + 4 * 7
                  jp nc, SD6PLAY_NOTE_LEAD__4 // if (c) {
                      // line 143
                      dec SP
                      // line 144
                      sub A, 0 + SD6_EXCEPT_LEN - 8
                      // line 145
                      rrca
                      rrca
                      // line 146
                      ld HL, (addr_rep_1 + 1)
                      // line 147
                      inc L
                      ld (sd6playNoteLead_rep_2 + 2), HL
sd6playNoteLead_rep_2:
                      ld (0x0000), SP
                      // line 148
                      inc L
                      inc L
                      ld (HL), A
                      // line 149
                      ld (addr_rep_1 + 1), HL
                      // line 150
                      jp sd6playNoteLead_pop
                  SD6PLAY_NOTE_LEAD__4: // } endif 
              SD6PLAY_NOTE_LEAD__3: // } endif 
              // line 155
              dec SP
              // line 156
              xor A, A
              ld (addr_wl_1  + 1), A
              // line 157
              inc A
              ld (addr_vol_1 + 1), A
              // line 158
              ld HL, 0 + sd6playNoteLead_modEnd
              // line 159
              jp sd6playNoteLead_initEndSlur
          SD6PLAY_NOTE_LEAD__2: // } endif 
          // line 163
          cp A, 0 + SD6_EXCEPT_LEN
          jr nz, SD6PLAY_NOTE_LEAD__5 // if (z_jr) {
              // line 164
addr_rep_1:
              ld HL, 0 + 0x0000
              dec (HL)
              jr nz, SD6PLAY_NOTE_LEAD__6 // if (z_jr) {
                  // line 165
                  dec L
                  dec L
                  dec L
                  ld (addr_rep_1 + 1), HL
                  // line 166
                  dec SP
                  // line 167
                  jp sd6playNoteLead_pop
              SD6PLAY_NOTE_LEAD__6: // } endif 
              // line 170
              dec L
              ld A, (HL)
              dec L
              ld L, (HL)
              ld H, A
              ld SP, HL
              // line 171
              jp sd6playNoteLead_pop
          SD6PLAY_NOTE_LEAD__5: // } endif 
          // line 175
            // line 176
            cp A, 0 + SD6_EXCEPT_LEN + 4
            jp z, 0 + addr_end
          // line 180
            // line 181
            ld A, reg_tmp_h
            ld (addr_wl_1 + 1), A
            // line 182
            ld reg_tmp_h, 0 + ADDR_SD6_TAB / 256
            ld reg_tmp_l, A
            ld A, (reg_tmp_hl)
            // line 183
            ld (           sd6playNoteLead_modVolEnd_1 + 1), A
            // line 184
            ld A, 0 + 2
            ld (sd6playNoteLead_modVolVal_1 + 1), A
            // line 185
            ld (addr_vol_1 + 1), A
            // line 186
            ld A, 0 + OPCODE_INC_A
            ld (sd6playNoteLead_modVolInc_0), A
            // line 187
            ld HL, 0 + sd6playNoteLead_modVol
            // line 188
            jp sd6playNoteLead_initEnd
      SD6PLAY_NOTE_LEAD__1: // } endif 
      // line 191
      add A, A
      ld reg_slc, A
      jr c, SD6PLAY_NOTE_LEAD__7 // if (nc_jr) {
          // line 193
          cp A, 0 + SD6_EXCEPT_LEN
          jp c, SD6PLAY_NOTE_LEAD__8 // if (nc) {
              // line 194
              cp A, 0 + SD6_EXCEPT_LEN + 4 * 4
              jp nc, SD6PLAY_NOTE_LEAD__9 // if (c) {
                  // line 195
                  sub A, 0 + SD6_EXCEPT_LEN - 1
                  // line 196
                  ld reg_tmp_h, 0 + ADDR_SD6_TAB / 256
                  ld reg_tmp_l, A
                  ld A, (reg_tmp_hl)
                  // line 197
                  ld (addr_mod_vol_speed_1 + 1), A
                  // line 198
                  dec SP
                  // line 199
                  jp sd6playNoteLead_pop
              SD6PLAY_NOTE_LEAD__9: // } endif 
          SD6PLAY_NOTE_LEAD__8: // } endif 
          // line 204
            // line 205
            ld A, reg_tmp_h
            ld (addr_wl_1 + 1), A
            // line 206
            ld reg_tmp_h, 0 + ADDR_SD6_TAB / 256
            ld reg_tmp_l, A
            ld A, (reg_tmp_hl)
            // line 207
            ld (           sd6playNoteLead_modVolVal_1 + 1), A
            // line 208
            ld (addr_vol_1                             + 1), A
            // line 209
            ld A, 0 + 2
            ld (sd6playNoteLead_modVolEnd_1 + 1), A
            // line 210
            ld A, 0 + OPCODE_DEC_A
            ld (sd6playNoteLead_modVolInc_0    ), A
            // line 211
            ld HL, 0 + sd6playNoteLead_modVol
            // line 212
            jp sd6playNoteLead_initEnd
      SD6PLAY_NOTE_LEAD__7: // } endif 
      // line 216
      cp A, 0 + SD6_EXCEPT_LEN
      jp c, SD6PLAY_NOTE_LEAD__10 // if (nc) {
          // line 218
          jr nz, SD6PLAY_NOTE_LEAD__11 // if (z_jr) {
              // line 219
              ld A, 0 + OPCODE_JR
              ld (addr_slide_0), A
              // line 220
              dec SP
              // line 221
              jp sd6playNoteLead_pop
          SD6PLAY_NOTE_LEAD__11: // } endif 
          // line 225
          cp A, 0 + SD6_EXCEPT_LEN + 4 * 4
          jp nc, SD6PLAY_NOTE_LEAD__12 // if (c) {
              // line 226
              sub A, 0 + SD6_EXCEPT_LEN - 2
              // line 227
              ld reg_tmp_h, 0 + ADDR_SD6_TAB / 256
              ld reg_tmp_l, A
              ld A, (reg_tmp_hl)
              // line 228
              ld (sd6playNoteLead_modWlSpeed_1 + 1), A
              // line 229
              ld A, 0 + OPCODE_LD_A_N
              ld (addr_slide_0), A
              // line 230
              dec SP
              // line 231
              jp sd6playNoteLead_pop
          SD6PLAY_NOTE_LEAD__12: // } endif 
      SD6PLAY_NOTE_LEAD__10: // } endif 
      // line 236
addr_slide_0:
      jr nz, SD6PLAY_NOTE_LEAD__13 // if (z_jr) {
          // line 237
          ld A, reg_tmp_h
          ld (sd6playNoteLead_modWlEnd_1 + 1), A
          // line 239
          ld A, (addr_wl_1 + 1)
          cp A, reg_tmp_h
          ld A, 0 + OPCODE_INC_A
          jr c, SD6PLAY_NOTE_LEAD__14 // if (nc_jr) {
              inc A
          SD6PLAY_NOTE_LEAD__14: // } endif 
          // line 240
          ld (sd6playNoteLead_modWlAdd_0), A
          // line 241
          ld HL, 0 + sd6playNoteLead_modWl
          // line 242
          jp sd6playNoteLead_initEndSlur
      SD6PLAY_NOTE_LEAD__13: // } endif 
      // line 246
        // line 247
        ld A, reg_tmp_h
        ld (addr_wl_1 + 1), A
        // line 248
        ld reg_tmp_h, 0 + ADDR_SD6_TAB / 256
        ld reg_tmp_l, A
        ld A, (reg_tmp_hl)
        // line 249
        ld (addr_vol_1               + 1), A
        // line 250
        ld HL, 0 + sd6playNoteLead_modEnd
      // line 254
sd6playNoteLead_initEnd:
      // line 255
      ld reg_wlc, 0 + 1
      // line 256
#if  BEEPER1_SYNC
      ld A, 0 + OPCODE_LD_A_N
      ld (addr_beeper1_sync_0), A
      // line 258
#endif 
      // line 260
sd6playNoteLead_initEndSlur:
      // line 261
      ld (sd6playNoteLead_modBranch + 1), HL
  SD6PLAY_NOTE_LEAD__0: // } endif 
  // line 265
sd6playNoteLead_modBranch:
  jp 0 + sd6playNoteLead_modEnd
  // line 268
sd6playNoteLead_modWl:
  // line 269
  ld A, reg_slc
sd6playNoteLead_modWlSpeed_1:
  and A, 0 + 0x00
  jr nz, 0 + sd6playNoteLead_modEnd
    // line 270
    ld A, (addr_wl_1 + 1)
sd6playNoteLead_modWlEnd_1:
    cp A, 0 + 0x00
    // line 271
    jr z, 0 + sd6playNoteLead_modEnd
sd6playNoteLead_modWlAdd_0:
    inc A
    ld (addr_wl_1 + 1), A
    // line 272
    ld reg_tmp_h, 0 + ADDR_SD6_TAB / 256
    ld reg_tmp_l, A
    ld A, (reg_tmp_hl)
    // line 273
    ld (addr_vol_1 + 1), A
    // line 274
    jp sd6playNoteLead_modEnd
  // line 278
sd6playNoteLead_modVol:
  // line 279
  ld A, reg_slc
addr_mod_vol_speed_1:
  and A, 0 + 0x00
  jp nz, SD6PLAY_NOTE_LEAD__15 // if (z) {
      // line 280
sd6playNoteLead_modVolVal_1:
      ld A, 0 + 0x00
sd6playNoteLead_modVolEnd_1:
      cp A, 0 + 0x00
      // line 281
      jr z, SD6PLAY_NOTE_LEAD__16 // if (nz_jr) {
sd6playNoteLead_modVolInc_0:
          inc A
          ld (sd6playNoteLead_modVolVal_1 + 1), A
      SD6PLAY_NOTE_LEAD__16: // } endif 
      // line 282
      ld (addr_vol_1 + 1), A
  SD6PLAY_NOTE_LEAD__15: // } endif 
  // line 286
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
  // line 309
  extern sd6playNoteBase_fl, sd6playNoteBase_initEnd, sd6playNoteBase_modStart, sd6playNoteBase_modVol, sd6playNoteBase_modVolEnd, sd6playNoteBase_modVolInc, sd6playNoteBase_modVolConv, sd6playNoteBase_modEnd
  // line 319
  dec reg_slc
  jp nz, SD6PLAY_NOTE_BASE__0 // if (z) {
      // line 320
      pop reg_tmp_hl
      // line 323
      ld A, reg_tmp_l
      add A, A
      jp c, SD6PLAY_NOTE_BASE__3 // if (nc) {
          // line 324
          add A, A
          ld reg_slc, A
          jr c, SD6PLAY_NOTE_BASE__2 // if (nc_jr) {
              // line 326
              dec SP
              // line 327
              xor A, A
              ld (addr_wl_1  + 1), A
              // line 328
              inc A
              ld (      addr_vol_1 + 1), A
              // line 329
              ld A, 0 + OPCODE_JR
              // line 330
              jp sd6playNoteBase_initEnd
          SD6PLAY_NOTE_BASE__2: // } endif 
          // line 332
            // line 334
            ld A, reg_tmp_h
            ld (addr_wl_1         + 1), A
            // line 335
            ld reg_tmp_h, 0 + ADDR_SD6_TAB / 256
            ld reg_tmp_l, A
            ld A, (reg_tmp_hl)
            // line 336
            ld (       sd6playNoteBase_modVolEnd + 1), A
            // line 337
            ld A, 0 + 2
            ld (sd6playNoteBase_modVol    + 1), A
            // line 338
            ld (       addr_vol_1                + 1), A
            // line 339
            ld A, 0 + OPCODE_INC_A
            ld (sd6playNoteBase_modVolInc), A
            // line 340
            ld A, 0 + OPCODE_LD_A_N
      jp SD6PLAY_NOTE_BASE__1
      SD6PLAY_NOTE_BASE__3: // } else {
          // line 343
          add A, A
          ld reg_slc, A
          // line 344
          ld A, reg_tmp_h
          ld (addr_wl_1 + 1), A
          // line 345
          ld reg_tmp_h, 0 + ADDR_SD6_TAB / 256
          ld reg_tmp_l, A
          ld A, (reg_tmp_hl)
          // line 346
          jr c, SD6PLAY_NOTE_BASE__5 // if (nc_jr) {
              // line 348
              ld (       sd6playNoteBase_modVol    + 1), A
              // line 349
              ld (       addr_vol_1                + 1), A
              // line 350
              ld A, 0 + 2
              ld (sd6playNoteBase_modVolEnd + 1), A
              // line 351
              ld A, 0 + OPCODE_DEC_A
              ld (sd6playNoteBase_modVolInc), A
              // line 352
              ld A, 0 + OPCODE_LD_A_N
          jp SD6PLAY_NOTE_BASE__4
          SD6PLAY_NOTE_BASE__5: // } else {
              // line 355
              ld (addr_vol_1 + 1), A
              // line 356
              ld A, 0 + OPCODE_JR
SD6PLAY_NOTE_BASE__4: // } endif
SD6PLAY_NOTE_BASE__1: // } endif
      // line 359
sd6playNoteBase_initEnd:
      // line 360
      ld (sd6playNoteBase_modStart), A
      // line 361
      ld reg_wlc, 0 + 1
  SD6PLAY_NOTE_BASE__0: // } endif 
  // line 365
sd6playNoteBase_modStart:
  // line 366
  jr 0 + sd6playNoteBase_modEnd
  // line 369
  ld A, reg_slc
  and A, 0 + BASE_VOL_SPEED_R
  jp nz, SD6PLAY_NOTE_BASE__6 // if (z) {
      // line 370
sd6playNoteBase_modVol:
      ld A, 0 + 0x00
      // line 371
sd6playNoteBase_modVolEnd:
      cp A, 0 + 0x00
      // line 372
      jr z, SD6PLAY_NOTE_BASE__7 // if (nz_jr) {
          // line 373
sd6playNoteBase_modVolInc:
          inc A
          // line 374
          ld (sd6playNoteBase_modVol + 1), A
      SD6PLAY_NOTE_BASE__7: // } endif 
      // line 376
      ld (addr_vol_1 + 1), A
  SD6PLAY_NOTE_BASE__6: // } endif 
  // line 380
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
  // line 407
  extern sd6playNoteChord2_initEnd, sd6playNoteChord2_modStart_0, sd6playNoteChord2_modVol_1, sd6playNoteChord2_modVolEnd_1, sd6playNoteChord2_modVolInc_0, sd6playNoteChord2_modEnd
  // line 417
  dec reg_slc
  jp nz, SD6PLAY_NOTE_CHORD2__0 // if (z) {
      // line 418
      dec SP
      pop AF
      // line 420
      add A, A
      jp c, SD6PLAY_NOTE_CHORD2__3 // if (nc) {
          // line 421
          add A, A
          ld reg_slc, A
          jr c, SD6PLAY_NOTE_CHORD2__2 // if (nc_jr) {
              // line 423
              xor A, A
              ld (addr_wl0_1 + 1), A
              // line 424
              ld (           addr_wl1_1 + 1), A
              // line 426
              ld reg_vol01, A
              // line 428
              ld A, 0 + OPCODE_JR
              // line 429
              jp sd6playNoteChord2_initEnd
          SD6PLAY_NOTE_CHORD2__2: // } endif 
          // line 431
            // line 432
            pop reg_wlc10
            // line 435
            ld A, reg_wlc1
            ld (addr_wl1_1 + 1), A
            // line 436
            ld A, reg_wlc0
            ld (addr_wl0_1 + 1), A
            // line 438
            ld reg_tmp_h, 0 + ADDR_SD6_TAB / 256
            ld reg_tmp_l, A
            ld A, (reg_tmp_hl)
            // line 439
            ld (sd6playNoteChord2_modVolEnd_1 + 1), A
            // line 441
            ld A, 0 + 2
            // line 442
            ld (sd6playNoteChord2_modVol_1 + 1), A
            // line 443
            ld reg_vol01, A
            // line 445
            ld A, 0 + OPCODE_INC_A
            // line 446
            ld (sd6playNoteChord2_modVolInc_0), A
            // line 448
            ld A, 0 + OPCODE_LD_A_N
      jp SD6PLAY_NOTE_CHORD2__1
      SD6PLAY_NOTE_CHORD2__3: // } else {
          // line 451
          add A, A
          ld reg_slc, A
          // line 452
          pop reg_wlc10
          // line 454
          ld A, reg_wlc0
          ld (addr_wl0_1 + 1), A
          // line 455
          ld reg_tmp_h, 0 + ADDR_SD6_TAB / 256
          ld reg_tmp_l, A
          ld A, (reg_tmp_hl)
          // line 456
          ld reg_vol01, A
          // line 458
          jr c, SD6PLAY_NOTE_CHORD2__5 // if (nc_jr) {
              // line 460
              ld (sd6playNoteChord2_modVol_1 + 1), A
              // line 462
              ld A, reg_wlc1
              ld (addr_wl1_1 + 1), A
              // line 464
              ld A, 0 + 2
              // line 465
              ld (sd6playNoteChord2_modVolEnd_1 + 1), A
              // line 467
              ld A, 0 + OPCODE_DEC_A
              // line 468
              ld (sd6playNoteChord2_modVolInc_0), A
              // line 470
              ld A, 0 + OPCODE_LD_A_N
          jp SD6PLAY_NOTE_CHORD2__4
          SD6PLAY_NOTE_CHORD2__5: // } else {
              // line 473
              ld A, reg_wlc1
              ld (addr_wl1_1 + 1), A
              // line 475
              ld A, 0 + OPCODE_JR
SD6PLAY_NOTE_CHORD2__4: // } endif
SD6PLAY_NOTE_CHORD2__1: // } endif
      // line 478
sd6playNoteChord2_initEnd:
      // line 479
      ld (sd6playNoteChord2_modStart_0), A
      // line 480
      ld reg_wlc0, 0 + 1
      // line 481
      ld reg_wlc1, reg_wlc0
  SD6PLAY_NOTE_CHORD2__0: // } endif 
  // line 485
sd6playNoteChord2_modStart_0:
  jr 0 + sd6playNoteChord2_modEnd
  // line 487
  ld A, reg_slc
  and A, 0 + CHORD2_VOL_SPEED_R
  jp nz, SD6PLAY_NOTE_CHORD2__6 // if (z) {
      // line 489
sd6playNoteChord2_modVol_1:
      ld A, 0 + 0x00
      // line 490
sd6playNoteChord2_modVolEnd_1:
      cp A, 0 + 0x00
      // line 491
      jr z, SD6PLAY_NOTE_CHORD2__7 // if (nz_jr) {
          // line 492
sd6playNoteChord2_modVolInc_0:
          inc A
          // line 493
          ld (sd6playNoteChord2_modVol_1 + 1), A
      SD6PLAY_NOTE_CHORD2__7: // } endif 
      // line 495
      ld reg_vol01, A
  SD6PLAY_NOTE_CHORD2__6: // } endif 
  // line 499
sd6playNoteChord2_modEnd:
endm
__endasm;
}










void SD6PLAY_NOTE_DRUM(int reg_sl, int reg_tmp, int addr_drum_nr_1)  __naked
{
__asm
SD6PLAY_NOTE_DRUM macro reg_sl, reg_tmp, addr_drum_nr_1
local SD6PLAY_NOTE_DRUM__0
  // line 516
  ld A, 0 + 0x46
  // line 517
  dec reg_sl
  jp nz, SD6PLAY_NOTE_DRUM__0 // if (z) {
      // line 518
      dec SP
      pop AF
      // line 519
      ld reg_tmp, A
      and A, 0 + 0xfc
      ld reg_sl, A
      // line 520
      xor A, reg_tmp
      // line 521
      rlca
      rlca
      rlca
      add A, 0 + 0x66
  SD6PLAY_NOTE_DRUM__0: // } endif 
  // line 523
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
  jp nz, SD6PLAY_BEEPER_WAVE__1 // if (z) {
      // line 543
addr_wl_1:
      ld reg_wl, 0 + 0x00
  jp SD6PLAY_BEEPER_WAVE__0
  SD6PLAY_BEEPER_WAVE__1: // } else {
      // line 546
      ld A, reg_wl
      // line 547
addr_vol_1:
      cp A, 0 + 0x02
      jr nc, SD6PLAY_BEEPER_WAVE__2 // if (c_jr) {
          ld reg_pulse, 0 + PULSE_H
      SD6PLAY_BEEPER_WAVE__2: // } endif 
SD6PLAY_BEEPER_WAVE__0: // } endif
endm
__endasm;
}










void SD6PLAY_BEEPER_WAVE_R(int reg_wl, int reg_pulse, int reg_vol, int addr_wl_1)  __naked
{
__asm
SD6PLAY_BEEPER_WAVE_R macro reg_wl, reg_pulse, reg_vol, addr_wl_1
local SD6PLAY_BEEPER_WAVE_R__0, SD6PLAY_BEEPER_WAVE_R__1, SD6PLAY_BEEPER_WAVE_R__2
  // line 567
  dec reg_wl
  jp nz, SD6PLAY_BEEPER_WAVE_R__1 // if (z) {
      // line 568
addr_wl_1:
      ld reg_wl, 0 + 0x00
  jp SD6PLAY_BEEPER_WAVE_R__0
  SD6PLAY_BEEPER_WAVE_R__1: // } else {
      // line 571
      ld A, reg_wl
      cp A, reg_vol
      jr nc, SD6PLAY_BEEPER_WAVE_R__2 // if (c_jr) {
          ld reg_pulse, 0 + PULSE_H
      SD6PLAY_BEEPER_WAVE_R__2: // } endif 
SD6PLAY_BEEPER_WAVE_R__0: // } endif
endm
__endasm;
}










void SD6PLAY_BEEPER_WAVE_BR(int reg_wl, int reg_pulse, int reg_vol, int addr_wl_1)  __naked
{
__asm
SD6PLAY_BEEPER_WAVE_BR macro reg_wl, reg_pulse, reg_vol, addr_wl_1
local SD6PLAY_BEEPER_WAVE_BR__0
  // line 589
local sd6play_beeperWaveBr, sd6play_beeperWaveBrEnd
  // line 590
  extern sd6play_beeperWaveBr, sd6play_beeperWaveBrEnd
  // line 593
  djnz B, 0 + sd6play_beeperWaveBr
    // line 594
addr_wl_1:
    ld reg_wl, 0 + 0x00
    jr 0 + sd6play_beeperWaveBrEnd
  // line 596
sd6play_beeperWaveBr:
    // line 598
    ld A, reg_wl
    cp A, reg_vol
    jr nc, SD6PLAY_BEEPER_WAVE_BR__0 // if (c_jr) {
        ld reg_pulse, 0 + PULSE_H
    SD6PLAY_BEEPER_WAVE_BR__0: // } endif 
  // line 600
sd6play_beeperWaveBrEnd:
endm
__endasm;
}










void SD6PLAY_BEEPER_DRUM(int reg_wav, int reg_pulse, int addr_drum_nr_1)  __naked
{
__asm
SD6PLAY_BEEPER_DRUM macro reg_wav, reg_pulse, addr_drum_nr_1
local SD6PLAY_BEEPER_DRUM__0
  // line 618
addr_drum_nr_1:
  bit 0, (reg_wav)
  // line 619
  jr z, SD6PLAY_BEEPER_DRUM__0 // if (nz_jr) {
      ld reg_pulse, 0 + PULSE_H
  SD6PLAY_BEEPER_DRUM__0: // } endif 
endm
__endasm;
}




u8 sd6play(u32 param)  __z88dk_fastcall
{
__asm
  // line 630
  extern sd6play_waitUntilKeyOff1
  // line 631
  extern sd6play_waitUntilKeyOff2
  // line 632
  extern sd6Play_rep_1
  // line 633
  extern sd6play_setVols, sd6play_loop, sd6play_beeperLoop
  // line 634
  extern sd6play_beeper0_wl, sd6play_slide0, sd6play_modVolSpeed0
  // line 635
  extern sd6play_beeper1_wl
  // line 636
  extern sd6play_beeper2_wl
  // line 637
  extern sd6play_beeper3_wl
  // line 638
  extern sd6play_beeper4_wl
  // line 639
  extern sd6play_beeper5_drumNr
  // line 640
  extern sd6play_beeper1_sync, sd6play_beeper1_syncEnd
  // line 641
  extern sd6play_end
  // line 642
  extern sd6play_restoreSP
  // line 643
  extern tmp
  // line 646
  push IX
  // line 647
  BANKH_VRAM_MMIO C
  // line 650
  ld A, 0 + 0xf9
  ld (MMIO_8255_PORTA), A
  // line 651
sd6play_waitUntilKeyOff1:
  // line 652
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F2_MASK | KEY9_F4_MASK
  jr nz, 0 + sd6play_waitUntilKeyOff1
  // line 655
  dec E
  jr z, sd6play__0 // if (nz_jr) {
      // line 656
      ld A, 0 + 0xfa
      ld (MMIO_8255_PORTA), A
  sd6play__0: // } endif 
  // line 671
  ld (sd6play_restoreSP + 1), SP
  ld SP, HL
  // line 672
  xor A, A
  // line 673
  ld C, 0 + BEEPER1_WL_CT_VAL
  ld (sd6play_beeper1_wl + 1), A
  // line 674
  inc A
  // line 675
  ld IXH, A
  ld IXL, A
  ld IYL, A
  ld IYH, A
  // line 676
  ld A, 0 + 7
  ld (sd6play_modVolSpeed0 + 1), A
  // line 677
  ld A, 0 + OPCODE_JR
  ld (sd6play_slide0          ), A
  // line 678
  ld HL, 0 + ADDR_SD6_REP + 0xff
  ld (sd6Play_rep_1 + 1), HL
  // line 679
#if  BEEPER1_SYNC
  ld (sd6play_beeper1_sync            ), A
  // line 681
#endif 
  // line 685
sd6play_loop:
  // line 688
  SD6PLAY_NOTE_LEAD B, IXH, H, L, HL, sd6Play_rep_1, sd6play_setVols + 1, sd6play_beeper0_wl, sd6play_slide0, sd6play_modVolSpeed0, sd6play_beeper1_sync, sd6play_end
  // line 691
  SD6PLAY_NOTE_BASE D, IXL, H, L, HL, sd6play_setVols, sd6play_beeper2_wl
  // line 693
sd6play_setVols:
  ld HL, 0 + 0x0000
  // line 694
  exx
  // line 695
  SD6PLAY_NOTE_CHORD2 B, C, BC, IYH, H, L, HL, D, sd6play_beeper3_wl, sd6play_beeper4_wl
  // line 696
  SD6PLAY_NOTE_DRUM IYL, L, sd6play_beeper5_drumNr
  // line 697
  ld HL, 0 + ADDR_SD6_DRUM | TEMPO
  // line 700
sd6play_beeperLoop:
  // line 701
    // line 702
    exx
    // line 705
    ld E, 0 + PULSE_L
    // line 706
    SD6PLAY_BEEPER_WAVE_BR B, E, H, sd6play_beeper0_wl
    // line 707
    SD6PLAY_BEEPER_WAVE_R C, E, H, sd6play_beeper1_wl
    // line 708
    SD6PLAY_BEEPER_WAVE_R D, E, L, sd6play_beeper2_wl
    // line 709
    ld A, E
    exx
    ld E, A
    // line 710
    SD6PLAY_BEEPER_WAVE_BR B, E, D, sd6play_beeper3_wl
    // line 711
    SD6PLAY_BEEPER_WAVE_R C, E, D, sd6play_beeper4_wl
    // line 712
    SD6PLAY_BEEPER_DRUM HL, E, sd6play_beeper5_drumNr
    // line 714
    ld A, E
    ld (MMIO_8253_CTRL), A
    // line 715
    inc L
    jp nz, 0 + sd6play_beeperLoop
  // line 718
  exx
  // line 721
  ld A, IXH
  and A, 0 + 3
  LEAD_ECHO_DELAY 
  jr nz, sd6play__1 // if (z_jr) {
      // line 723
#if  BEEPER1_SYNC                                        
      // line 724
sd6play_beeper1_sync:
      jr 0 + sd6play_beeper1_syncEnd
        // line 725
        ld C, 0 + BEEPER1_WL_CT_VAL
        // line 726
        ld A, 0 + OPCODE_JR
        ld (sd6play_beeper1_sync), A
      // line 728
sd6play_beeper1_syncEnd:
      // line 729
#endif 
      ld A, (sd6play_beeper0_wl + 1)
      ld (sd6play_beeper1_wl + 1), A
  sd6play__1: // } endif 
  // line 733
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F2_MASK | KEY9_F4_MASK
  jp z, 0 + sd6play_loop
  // line 734
  ld L, A
  // line 735
  jr 0 + sd6play_waitUntilKeyOff2
  // line 738
sd6play_end:
  // line 739
  ld L, 0 + 0x00
  // line 741
sd6play_waitUntilKeyOff2:
  // line 742
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F2_MASK | KEY9_F4_MASK
  jr nz, 0 + sd6play_waitUntilKeyOff2
  // line 745
  ld A, 0 + MMIO_8253_CT0_MODE3
  ld (MMIO_8253_CTRL), A
  // line 747
  BANKH_RAM C
  // line 749
sd6play_restoreSP:
  ld SP, 0 + 0x0000
  // line 750
  pop IX
__endasm;
}

#pragma restore
