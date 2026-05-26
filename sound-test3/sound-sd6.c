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
#define BASE_VOL_SPEED_R    7       
#define CHORD2_VOL_SPEED_R  7       


void LEAD_ECHO_DELAY(void)  __naked
{
__asm
LEAD_ECHO_DELAY macro void
  // line 37
  dec A
endm
__endasm;
}



#ifndef SD6_EXCEPT_LEN
#define SD6_EXCEPT_LEN      4
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
  ld B, 0 + SZ_SD6_TAB&0xff
  // line 81
sd6Init_dataLoop:
    // line 82
    ld A, (HL)
    and A, 31
    ld (DE), A
    // line 83
    xor A, (HL)
    ld (HL), A
    // line 84
    inc L
    inc E
  djnz B,0 + sd6Init_dataLoop
  // line 88
  BANKH_VRAM_MMIO 
  // line 89
  ld HL, 0 + MMIO_ETC
  // line 90
  ld (HL), 0 + MMIO_ETC_GATE_MASK
  // line 91
  dec L
  // line 92
  ld (HL), 0 + MMIO_8253_CT0_MODE3
  // line 93
  BANKH_RAM 
__endasm;
}


















void SD6PLAY_NOTE_LEAD(
    int B_wlc, int IXH_slc,
    int H_tmp, int L_tmp, int HL_tmp,
    int addr_rep_1,
    int addr_vol_1, int addr_wl_1, int addr_slide_0, int addr_mod_vol_speed_1,
    int addr_beeper1_sync_0,
    int addr_end
)  __naked
{
__asm
SD6PLAY_NOTE_LEAD macro B_wlc, IXH_slc, H_tmp, L_tmp, HL_tmp, addr_rep_1, addr_vol_1, addr_wl_1, addr_slide_0, addr_mod_vol_speed_1, addr_beeper1_sync_0, addr_end
local SD6PLAY_NOTE_LEAD__else0, SD6PLAY_NOTE_LEAD__endif0, SD6PLAY_NOTE_LEAD__else1, SD6PLAY_NOTE_LEAD__endif1, SD6PLAY_NOTE_LEAD__else2, SD6PLAY_NOTE_LEAD__endif2, SD6PLAY_NOTE_LEAD__else3, SD6PLAY_NOTE_LEAD__endif3, SD6PLAY_NOTE_LEAD__else4, SD6PLAY_NOTE_LEAD__endif4, SD6PLAY_NOTE_LEAD__else5, SD6PLAY_NOTE_LEAD__endif5, SD6PLAY_NOTE_LEAD__else6, SD6PLAY_NOTE_LEAD__endif6, SD6PLAY_NOTE_LEAD__else7, SD6PLAY_NOTE_LEAD__endif7, SD6PLAY_NOTE_LEAD__else8, SD6PLAY_NOTE_LEAD__endif8, SD6PLAY_NOTE_LEAD__else9, SD6PLAY_NOTE_LEAD__endif9, SD6PLAY_NOTE_LEAD__else10, SD6PLAY_NOTE_LEAD__endif10, SD6PLAY_NOTE_LEAD__else11, SD6PLAY_NOTE_LEAD__endif11, SD6PLAY_NOTE_LEAD__else12, SD6PLAY_NOTE_LEAD__endif12, SD6PLAY_NOTE_LEAD__else13, SD6PLAY_NOTE_LEAD__endif13, SD6PLAY_NOTE_LEAD__else14, SD6PLAY_NOTE_LEAD__endif14, SD6PLAY_NOTE_LEAD__else15, SD6PLAY_NOTE_LEAD__endif15, SD6PLAY_NOTE_LEAD__else16, SD6PLAY_NOTE_LEAD__endif16
  // line 122
  extern sd6playNoteLead_pop, sd6playNoteLead_rep_2, sd6playNoteLead_initEnd, sd6playNoteLead_initEndSlur, sd6playNoteLead_modBranch_1, sd6playNoteLead_modWl, sd6playNoteLead_modWlEnd_1, sd6playNoteLead_modWlSpeed_1, sd6playNoteLead_modWlAdd_0, sd6playNoteLead_modVol, sd6playNoteLead_modVolVal_1, sd6playNoteLead_modVolEnd_1, sd6playNoteLead_modVolInc_0, sd6playNoteLead_modEnd
  // line 132
  dec IXH_slc
  jp nz, SD6PLAY_NOTE_LEAD__endif0
      // line 133
sd6playNoteLead_pop:
      // line 134
      pop HL_tmp
      // line 137
      ld A, L_tmp
      add A, A
      jp c, SD6PLAY_NOTE_LEAD__endif1
          // line 138
          add A, A
          ld IXH_slc, A
          jr c, SD6PLAY_NOTE_LEAD__endif2
              // line 140
              cp A, 0 + SD6_EXCEPT_LEN
              jp c, SD6PLAY_NOTE_LEAD__endif3
                  // line 141
                  cp A, 0 + SD6_EXCEPT_LEN+4*7
                  jp nc, SD6PLAY_NOTE_LEAD__endif4
                      // line 142
                      dec SP
                      // line 143
                      sub A, 0 + SD6_EXCEPT_LEN-8
                      // line 144
                      rrca
                      rrca
                      // line 145
                      ld HL, (addr_rep_1+1)
                      // line 146
                      inc L
                      ld (sd6playNoteLead_rep_2+2), HL
sd6playNoteLead_rep_2:
                      ld (0x0000), SP
                      // line 147
                      inc L
                      inc L
                      ld (HL), A
                      // line 148
                      ld (addr_rep_1+1), HL
                      // line 149
                      jp 0 + sd6playNoteLead_pop
                  SD6PLAY_NOTE_LEAD__endif4:

              SD6PLAY_NOTE_LEAD__endif3:

              // line 154
              dec SP
              // line 155
              xor A, A
              ld (addr_wl_1+1), A
              // line 156
              inc A
              ld (addr_vol_1+1), A
              // line 157
              ld HL, 0 + sd6playNoteLead_modEnd
              // line 158
              jp 0 + sd6playNoteLead_initEndSlur
          SD6PLAY_NOTE_LEAD__endif2:

          // line 162
          cp A, 0 + SD6_EXCEPT_LEN
          jr nz, SD6PLAY_NOTE_LEAD__endif5
              // line 163
addr_rep_1:
              ld HL, 0
              dec (HL)
              jr nz, SD6PLAY_NOTE_LEAD__endif6
                  // line 164
                  dec L
                  dec L
                  dec L
                  ld (addr_rep_1+1), HL
                  // line 165
                  dec SP
                  // line 166
                  jp 0 + sd6playNoteLead_pop
              SD6PLAY_NOTE_LEAD__endif6:

              // line 169
              dec L
              ld A, (HL)
              dec L
              ld L, (HL)
              ld H, A
              ld SP, HL
              // line 170
              jp 0 + sd6playNoteLead_pop
          SD6PLAY_NOTE_LEAD__endif5:

          // line 174
            // line 175
            cp A, 0 + SD6_EXCEPT_LEN+4
            jp z, 0 + addr_end
          // line 179
            // line 180
            ld A, H_tmp
            ld (addr_wl_1+1), A
            // line 181
            ld H_tmp, 0 + ADDR_SD6_TAB/256
            ld L_tmp, A
            ld A, (HL_tmp)
            // line 182
            ld (sd6playNoteLead_modVolEnd_1+1), A
            // line 183
            ld A, 2
            ld (sd6playNoteLead_modVolVal_1+1), A
            // line 184
            ld (addr_vol_1+1), A
            // line 185
            ld A, 0 + OPCODE_INC_A
            ld (sd6playNoteLead_modVolInc_0), A
            // line 186
            ld HL, 0 + sd6playNoteLead_modVol
            // line 187
            jp 0 + sd6playNoteLead_initEnd
      SD6PLAY_NOTE_LEAD__endif1:

      // line 190
      add A, A
      ld IXH_slc, A
      jr c, SD6PLAY_NOTE_LEAD__endif7
          // line 192
          cp A, 0 + SD6_EXCEPT_LEN
          jp c, SD6PLAY_NOTE_LEAD__endif8
              // line 193
              cp A, 0 + SD6_EXCEPT_LEN+4*4
              jp nc, SD6PLAY_NOTE_LEAD__endif9
                  // line 194
                  sub A, 0 + SD6_EXCEPT_LEN-1
                  // line 195
                  ld H_tmp, 0 + ADDR_SD6_TAB/256
                  ld L_tmp, A
                  ld A, (HL_tmp)
                  // line 196
                  ld (addr_mod_vol_speed_1+1), A
                  // line 197
                  dec SP
                  // line 198
                  jp 0 + sd6playNoteLead_pop
              SD6PLAY_NOTE_LEAD__endif9:

          SD6PLAY_NOTE_LEAD__endif8:

          // line 203
            // line 204
            ld A, H_tmp
            ld (addr_wl_1+1), A
            // line 205
            ld H_tmp, 0 + ADDR_SD6_TAB/256
            ld L_tmp, A
            ld A, (HL_tmp)
            // line 206
            ld (sd6playNoteLead_modVolVal_1+1), A
            // line 207
            ld (addr_vol_1+1), A
            // line 208
            ld A, 2
            ld (sd6playNoteLead_modVolEnd_1+1), A
            // line 209
            ld A, 0 + OPCODE_DEC_A
            ld (sd6playNoteLead_modVolInc_0), A
            // line 210
            ld HL, 0 + sd6playNoteLead_modVol
            // line 211
            jp 0 + sd6playNoteLead_initEnd
      SD6PLAY_NOTE_LEAD__endif7:

      // line 215
      cp A, 0 + SD6_EXCEPT_LEN
      jp c, SD6PLAY_NOTE_LEAD__endif10
          // line 217
          jr nz, SD6PLAY_NOTE_LEAD__endif11
              // line 218
              ld A, 0 + OPCODE_JR
              ld (addr_slide_0), A
              // line 219
              dec SP
              // line 220
              jp 0 + sd6playNoteLead_pop
          SD6PLAY_NOTE_LEAD__endif11:

          // line 224
          cp A, 0 + SD6_EXCEPT_LEN+4*4
          jp nc, SD6PLAY_NOTE_LEAD__endif12
              // line 225
              sub A, 0 + SD6_EXCEPT_LEN-2
              // line 226
              ld H_tmp, 0 + ADDR_SD6_TAB/256
              ld L_tmp, A
              ld A, (HL_tmp)
              // line 227
              ld (sd6playNoteLead_modWlSpeed_1+1), A
              // line 228
              ld A, 0 + OPCODE_LD_A_N
              ld (addr_slide_0), A
              // line 229
              dec SP
              // line 230
              jp 0 + sd6playNoteLead_pop
          SD6PLAY_NOTE_LEAD__endif12:

      SD6PLAY_NOTE_LEAD__endif10:

      // line 235
addr_slide_0:
      jr nz, SD6PLAY_NOTE_LEAD__endif13
          // line 236
          ld A, H_tmp
          ld (sd6playNoteLead_modWlEnd_1+1), A
          // line 238
          ld A, (addr_wl_1+1)
          cp A, H_tmp
          ld A, 0 + OPCODE_INC_A
          jr c, SD6PLAY_NOTE_LEAD__endif14
              inc A
          SD6PLAY_NOTE_LEAD__endif14:

          // line 239
          ld (sd6playNoteLead_modWlAdd_0), A
          // line 240
          ld HL, 0 + sd6playNoteLead_modWl
          // line 241
          jp 0 + sd6playNoteLead_initEndSlur
      SD6PLAY_NOTE_LEAD__endif13:

      // line 245
        // line 246
        ld A, H_tmp
        ld (addr_wl_1+1), A
        // line 247
        ld H_tmp, 0 + ADDR_SD6_TAB/256
        ld L_tmp, A
        ld A, (HL_tmp)
        // line 248
        ld (addr_vol_1+1), A
        // line 249
        ld HL, 0 + sd6playNoteLead_modEnd
      // line 253
sd6playNoteLead_initEnd:
      // line 254
      ld B_wlc, 1
      // line 255
#if  BEEPER1_SYNC
      ld A, 0 + OPCODE_LD_A_N
      ld (addr_beeper1_sync_0), A
      // line 257
#endif 
      // line 259
sd6playNoteLead_initEndSlur:
      // line 260
      ld (sd6playNoteLead_modBranch_1+1), HL
  SD6PLAY_NOTE_LEAD__endif0:

  // line 264
sd6playNoteLead_modBranch_1:
  jp 0 + sd6playNoteLead_modEnd
  // line 267
sd6playNoteLead_modWl:
  // line 268
  ld A, IXH_slc
sd6playNoteLead_modWlSpeed_1:
  and A, 0
  jr nz, 0 + sd6playNoteLead_modEnd
    // line 269
    ld A, (addr_wl_1+1)
sd6playNoteLead_modWlEnd_1:
    cp A, 0
    // line 270
    jr z, 0 + sd6playNoteLead_modEnd
sd6playNoteLead_modWlAdd_0:
    inc A
    ld (addr_wl_1+1), A
    // line 271
    ld H_tmp, 0 + ADDR_SD6_TAB/256
    ld L_tmp, A
    ld A, (HL_tmp)
    // line 272
    ld (addr_vol_1+1), A
    // line 273
    jp 0 + sd6playNoteLead_modEnd
  // line 277
sd6playNoteLead_modVol:
  // line 278
  ld A, IXH_slc
addr_mod_vol_speed_1:
  and A, 0
  jp nz, SD6PLAY_NOTE_LEAD__endif15
      // line 279
sd6playNoteLead_modVolVal_1:
      ld A, 0
sd6playNoteLead_modVolEnd_1:
      cp A, 0
      // line 280
      jr z, SD6PLAY_NOTE_LEAD__endif16
sd6playNoteLead_modVolInc_0:
          inc A
          ld (sd6playNoteLead_modVolVal_1+1), A
      SD6PLAY_NOTE_LEAD__endif16:

      // line 281
      ld (addr_vol_1+1), A
  SD6PLAY_NOTE_LEAD__endif15:

  // line 285
sd6playNoteLead_modEnd:
endm
__endasm;
}













void SD6PLAY_NOTE_BASE(
    int B_wlc, int IXH_slc,
    int H_tmp, int L_tmp, int HL_tmp,
    int addr_vol_1, int addr_wl_1
)  __naked
{
__asm
SD6PLAY_NOTE_BASE macro B_wlc, IXH_slc, H_tmp, L_tmp, HL_tmp, addr_vol_1, addr_wl_1
local SD6PLAY_NOTE_BASE__else0, SD6PLAY_NOTE_BASE__endif0, SD6PLAY_NOTE_BASE__else1, SD6PLAY_NOTE_BASE__endif1, SD6PLAY_NOTE_BASE__else2, SD6PLAY_NOTE_BASE__endif2, SD6PLAY_NOTE_BASE__else3, SD6PLAY_NOTE_BASE__endif3, SD6PLAY_NOTE_BASE__else4, SD6PLAY_NOTE_BASE__endif4, SD6PLAY_NOTE_BASE__else5, SD6PLAY_NOTE_BASE__endif5
  // line 308
  extern sd6playNoteBase_fl, sd6playNoteBase_initEnd, sd6playNoteBase_modStart, sd6playNoteBase_modVol, sd6playNoteBase_modVolEnd, sd6playNoteBase_modVolInc, sd6playNoteBase_modVolConv, sd6playNoteBase_modEnd
  // line 318
  dec IXH_slc
  jp nz, SD6PLAY_NOTE_BASE__endif0
      // line 319
      pop HL_tmp
      // line 322
      ld A, L_tmp
      add A, A
      jp c, SD6PLAY_NOTE_BASE__else1
          // line 323
          add A, A
          ld IXH_slc, A
          jr c, SD6PLAY_NOTE_BASE__endif2
              // line 325
              dec SP
              // line 326
              xor A, A
              ld (addr_wl_1+1), A
              // line 327
              inc A
              ld (addr_vol_1+1), A
              // line 328
              ld A, 0 + OPCODE_JR
              // line 329
              jp 0 + sd6playNoteBase_initEnd
          SD6PLAY_NOTE_BASE__endif2:

          // line 331
            // line 333
            ld A, H_tmp
            ld (addr_wl_1+1), A
            // line 334
            ld H_tmp, 0 + ADDR_SD6_TAB/256
            ld L_tmp, A
            ld A, (HL_tmp)
            // line 335
            ld (sd6playNoteBase_modVolEnd+1), A
            // line 336
            ld A, 2
            ld (sd6playNoteBase_modVol+1), A
            // line 337
            ld (addr_vol_1+1), A
            // line 338
            ld A, 0 + OPCODE_INC_A
            ld (sd6playNoteBase_modVolInc), A
            // line 339
            ld A, 0 + OPCODE_LD_A_N
      jp SD6PLAY_NOTE_BASE__endif1
      SD6PLAY_NOTE_BASE__else1:
          // line 342
          add A, A
          ld IXH_slc, A
          // line 343
          ld A, H_tmp
          ld (addr_wl_1+1), A
          // line 344
          ld H_tmp, 0 + ADDR_SD6_TAB/256
          ld L_tmp, A
          ld A, (HL_tmp)
          // line 345
          jr c, SD6PLAY_NOTE_BASE__else3
              // line 347
              ld (sd6playNoteBase_modVol+1), A
              // line 348
              ld (addr_vol_1+1), A
              // line 349
              ld A, 2
              ld (sd6playNoteBase_modVolEnd+1), A
              // line 350
              ld A, 0 + OPCODE_DEC_A
              ld (sd6playNoteBase_modVolInc), A
              // line 351
              ld A, 0 + OPCODE_LD_A_N
          jp SD6PLAY_NOTE_BASE__endif3
          SD6PLAY_NOTE_BASE__else3:
              // line 354
              ld (addr_vol_1+1), A
              // line 355
              ld A, 0 + OPCODE_JR
          SD6PLAY_NOTE_BASE__endif3:

      SD6PLAY_NOTE_BASE__endif1:

      // line 358
sd6playNoteBase_initEnd:
      // line 359
      ld (sd6playNoteBase_modStart), A
      // line 360
      ld B_wlc, 1
  SD6PLAY_NOTE_BASE__endif0:

  // line 364
sd6playNoteBase_modStart:
  // line 365
  jr 0 + sd6playNoteBase_modEnd
  // line 368
  ld A, IXH_slc
  and A, 0 + BASE_VOL_SPEED_R
  jp nz, SD6PLAY_NOTE_BASE__endif4
      // line 369
sd6playNoteBase_modVol:
      ld A, 0
      // line 370
sd6playNoteBase_modVolEnd:
      cp A, 0
      // line 371
      jr z, SD6PLAY_NOTE_BASE__endif5
          // line 372
sd6playNoteBase_modVolInc:
          inc A
          // line 373
          ld (sd6playNoteBase_modVol+1), A
      SD6PLAY_NOTE_BASE__endif5:

      // line 375
      ld (addr_vol_1+1), A
  SD6PLAY_NOTE_BASE__endif4:

  // line 379
sd6playNoteBase_modEnd:
endm
__endasm;
}

















void SD6PLAY_NOTE_CHORD2(
    int B_wlc0, int C_wlc1, int BC_wlc01, int IXH_slc,
    int H_tmp, int L_tmp, int HL_tmp,
    int D_vol01, int addr_wl0_1, int addr_wl1_1
)  __naked
{
__asm
SD6PLAY_NOTE_CHORD2 macro B_wlc0, C_wlc1, BC_wlc01, IXH_slc, H_tmp, L_tmp, HL_tmp, D_vol01, addr_wl0_1, addr_wl1_1
local SD6PLAY_NOTE_CHORD2__else0, SD6PLAY_NOTE_CHORD2__endif0, SD6PLAY_NOTE_CHORD2__else1, SD6PLAY_NOTE_CHORD2__endif1, SD6PLAY_NOTE_CHORD2__else2, SD6PLAY_NOTE_CHORD2__endif2, SD6PLAY_NOTE_CHORD2__else3, SD6PLAY_NOTE_CHORD2__endif3, SD6PLAY_NOTE_CHORD2__else4, SD6PLAY_NOTE_CHORD2__endif4, SD6PLAY_NOTE_CHORD2__else5, SD6PLAY_NOTE_CHORD2__endif5
  // line 406
  extern sd6playNoteChord2_initEnd, sd6playNoteChord2_modStart_0, sd6playNoteChord2_modVol_1, sd6playNoteChord2_modVolEnd_1, sd6playNoteChord2_modVolInc_0, sd6playNoteChord2_modEnd
  // line 416
  dec IXH_slc
  jp nz, SD6PLAY_NOTE_CHORD2__endif0
      // line 417
      dec SP
      pop AF
      // line 419
      add A, A
      jp c, SD6PLAY_NOTE_CHORD2__else1
          // line 420
          add A, A
          ld IXH_slc, A
          jr c, SD6PLAY_NOTE_CHORD2__endif2
              // line 422
              xor A, A
              ld (addr_wl0_1+1), A
              // line 423
              ld (addr_wl1_1+1), A
              // line 425
              ld D_vol01, A
              // line 427
              ld A, 0 + OPCODE_JR
              // line 428
              jp 0 + sd6playNoteChord2_initEnd
          SD6PLAY_NOTE_CHORD2__endif2:

          // line 430
            // line 431
            pop BC_wlc01
            // line 434
            ld A, C_wlc1
            ld (addr_wl1_1+1), A
            // line 435
            ld A, B_wlc0
            ld (addr_wl0_1+1), A
            // line 437
            ld H_tmp, 0 + ADDR_SD6_TAB/256
            ld L_tmp, A
            ld A, (HL_tmp)
            // line 438
            ld (sd6playNoteChord2_modVolEnd_1+1), A
            // line 440
            ld A, 2
            // line 441
            ld (sd6playNoteChord2_modVol_1+1), A
            // line 442
            ld D_vol01, A
            // line 444
            ld A, 0 + OPCODE_INC_A
            // line 445
            ld (sd6playNoteChord2_modVolInc_0), A
            // line 447
            ld A, 0 + OPCODE_LD_A_N
      jp SD6PLAY_NOTE_CHORD2__endif1
      SD6PLAY_NOTE_CHORD2__else1:
          // line 450
          add A, A
          ld IXH_slc, A
          // line 451
          pop BC_wlc01
          // line 453
          ld A, B_wlc0
          ld (addr_wl0_1+1), A
          // line 454
          ld H_tmp, 0 + ADDR_SD6_TAB/256
          ld L_tmp, A
          ld A, (HL_tmp)
          // line 455
          ld D_vol01, A
          // line 457
          jr c, SD6PLAY_NOTE_CHORD2__else3
              // line 459
              ld (sd6playNoteChord2_modVol_1+1), A
              // line 461
              ld A, C_wlc1
              ld (addr_wl1_1+1), A
              // line 463
              ld A, 2
              // line 464
              ld (sd6playNoteChord2_modVolEnd_1+1), A
              // line 466
              ld A, 0 + OPCODE_DEC_A
              // line 467
              ld (sd6playNoteChord2_modVolInc_0), A
              // line 469
              ld A, 0 + OPCODE_LD_A_N
          jp SD6PLAY_NOTE_CHORD2__endif3
          SD6PLAY_NOTE_CHORD2__else3:
              // line 472
              ld A, C_wlc1
              ld (addr_wl1_1+1), A
              // line 474
              ld A, 0 + OPCODE_JR
          SD6PLAY_NOTE_CHORD2__endif3:

      SD6PLAY_NOTE_CHORD2__endif1:

      // line 477
sd6playNoteChord2_initEnd:
      // line 478
      ld (sd6playNoteChord2_modStart_0), A
      // line 479
      ld B_wlc0, 1
      // line 480
      ld C_wlc1, B_wlc0
  SD6PLAY_NOTE_CHORD2__endif0:

  // line 484
sd6playNoteChord2_modStart_0:
  jr 0 + sd6playNoteChord2_modEnd
  // line 486
  ld A, IXH_slc
  and A, 0 + CHORD2_VOL_SPEED_R
  jp nz, SD6PLAY_NOTE_CHORD2__endif4
      // line 488
sd6playNoteChord2_modVol_1:
      ld A, 0
      // line 489
sd6playNoteChord2_modVolEnd_1:
      cp A, 0
      // line 490
      jr z, SD6PLAY_NOTE_CHORD2__endif5
          // line 491
sd6playNoteChord2_modVolInc_0:
          inc A
          // line 492
          ld (sd6playNoteChord2_modVol_1+1), A
      SD6PLAY_NOTE_CHORD2__endif5:

      // line 494
      ld D_vol01, A
  SD6PLAY_NOTE_CHORD2__endif4:

  // line 498
sd6playNoteChord2_modEnd:
endm
__endasm;
}










void SD6PLAY_NOTE_DRUM(int IYL_sl, int L_tmp, int addr_drum_nr_1)  __naked
{
__asm
SD6PLAY_NOTE_DRUM macro IYL_sl, L_tmp, addr_drum_nr_1
local SD6PLAY_NOTE_DRUM__else0, SD6PLAY_NOTE_DRUM__endif0
  // line 515
  ld A, 70
  // line 516
  dec IYL_sl
  jp nz, SD6PLAY_NOTE_DRUM__endif0
      // line 517
      dec SP
      pop AF
      // line 518
      ld L_tmp, A
      and A, 252
      ld IYL_sl, A
      // line 519
      xor A, L_tmp
      // line 520
      rlca
      rlca
      rlca
      add A, 102
  SD6PLAY_NOTE_DRUM__endif0:

  // line 522
  ld (addr_drum_nr_1+1), A
endm
__endasm;
}











void SD6PLAY_BEEPER_WAVE(int C_wl, int E_pulse, int addr_vol, int addr_wl_1)  __naked
{
__asm
SD6PLAY_BEEPER_WAVE macro C_wl, E_pulse, addr_vol, addr_wl_1
local SD6PLAY_BEEPER_WAVE__else0, SD6PLAY_BEEPER_WAVE__endif0, SD6PLAY_BEEPER_WAVE__else1, SD6PLAY_BEEPER_WAVE__endif1
  // line 541
  dec C_wl
  jp nz, SD6PLAY_BEEPER_WAVE__else0
      // line 542
addr_wl_1:
      ld C_wl, 0
  jp SD6PLAY_BEEPER_WAVE__endif0
  SD6PLAY_BEEPER_WAVE__else0:
      // line 545
      ld A, C_wl
      // line 546
addr_vol_1:
      cp A, 2
      jr nc, SD6PLAY_BEEPER_WAVE__endif1
          ld E_pulse, 0 + PULSE_H
      SD6PLAY_BEEPER_WAVE__endif1:

  SD6PLAY_BEEPER_WAVE__endif0:

endm
__endasm;
}










void SD6PLAY_BEEPER_WAVE_R(int C_wl, int E_pulse, int H_vol, int addr_wl_1)  __naked
{
__asm
SD6PLAY_BEEPER_WAVE_R macro C_wl, E_pulse, H_vol, addr_wl_1
local SD6PLAY_BEEPER_WAVE_R__else0, SD6PLAY_BEEPER_WAVE_R__endif0, SD6PLAY_BEEPER_WAVE_R__else1, SD6PLAY_BEEPER_WAVE_R__endif1
  // line 566
  dec C_wl
  jp nz, SD6PLAY_BEEPER_WAVE_R__else0
      // line 567
addr_wl_1:
      ld C_wl, 0
  jp SD6PLAY_BEEPER_WAVE_R__endif0
  SD6PLAY_BEEPER_WAVE_R__else0:
      // line 570
      ld A, C_wl
      cp A, H_vol
      jr nc, SD6PLAY_BEEPER_WAVE_R__endif1
          ld E_pulse, 0 + PULSE_H
      SD6PLAY_BEEPER_WAVE_R__endif1:

  SD6PLAY_BEEPER_WAVE_R__endif0:

endm
__endasm;
}










void SD6PLAY_BEEPER_WAVE_BR(int B_wl, int E_pulse, int H_vol, int addr_wl_1)  __naked
{
__asm
SD6PLAY_BEEPER_WAVE_BR macro B_wl, E_pulse, H_vol, addr_wl_1
local SD6PLAY_BEEPER_WAVE_BR__else0, SD6PLAY_BEEPER_WAVE_BR__endif0
  // line 588
  local sd6play_beeperWaveBr
  local sd6play_beeperWaveBrEnd
  // line 589
  extern sd6play_beeperWaveBr, sd6play_beeperWaveBrEnd
  // line 592
  djnz B,0 + sd6play_beeperWaveBr
    // line 593
addr_wl_1:
    ld B_wl, 0
    jr 0 + sd6play_beeperWaveBrEnd
  // line 595
sd6play_beeperWaveBr:
    // line 597
    ld A, B_wl
    cp A, H_vol
    jr nc, SD6PLAY_BEEPER_WAVE_BR__endif0
        ld E_pulse, 0 + PULSE_H
    SD6PLAY_BEEPER_WAVE_BR__endif0:

  // line 599
sd6play_beeperWaveBrEnd:
endm
__endasm;
}










void SD6PLAY_BEEPER_DRUM(int HL_wav, int E_pulse, int addr_drum_nr_1)  __naked
{
__asm
SD6PLAY_BEEPER_DRUM macro HL_wav, E_pulse, addr_drum_nr_1
local SD6PLAY_BEEPER_DRUM__else0, SD6PLAY_BEEPER_DRUM__endif0
  // line 617
addr_drum_nr_1:
  bit 0, (HL_wav)
  // line 618
  jr z, SD6PLAY_BEEPER_DRUM__endif0
      ld E_pulse, 0 + PULSE_H
  SD6PLAY_BEEPER_DRUM__endif0:

endm
__endasm;
}




u8 sd6play(const u32 param) 
{
__asm
  // line 629
  extern sd6play_waitUntilKeyOff2
  // line 630
  extern sd6play_tempo_1
  // line 631
  extern sd6play_rep_1
  // line 632
  extern sd6play_setVols, sd6play_loop, sd6play_beeperLoop
  // line 633
  extern sd6play_beeper0_wl_1, sd6play_slide0, sd6play_modVolSpeed0_1
  // line 634
  extern sd6play_beeper1_wl_1
  // line 635
  extern sd6play_beeper2_wl
  // line 636
  extern sd6play_beeper3_wl
  // line 637
  extern sd6play_beeper4_wl
  // line 638
  extern sd6play_beeper5_drumNr
  // line 639
  extern sd6play_beeper1_sync, sd6play_beeper1_syncEnd
  // line 640
  extern sd6play_end
  // line 641
  extern sd6play_restoreSP_1
  // line 642
  extern tmp
  // line 645
  push IX
  // line 646
  BANKH_VRAM_MMIO 
  // line 649
  ld A, 249
  ld (MMIO_8255_PORTA), A
  // line 650
  sd6play__loop_top0: // do {
      // line 651
      ld A, (MMIO_8255_PORTB)
      cpl
      and A, 0 + KEY9_F1_MASK|KEY9_F2_MASK|KEY9_F3_MASK|KEY9_F4_MASK
  sd6play__loop_end0:
  jr nz, sd6play__loop_top0 // } while (nz_jr)
  sd6play__loop_exit0: // loop exit

  // line 655
  bit 0, D
  jr nz, sd6play__endif1
      // line 656
      ld A, 250
      ld (MMIO_8255_PORTA), A
  sd6play__endif1:

  // line 660
  ld A, E
  or A, 0 + ADDR_SD6_DRUM&0xff
  // line 661
  ld (sd6play_tempo_1+1), A
  // line 675
  ld (sd6play_restoreSP_1+1), SP
  ld SP, HL
  // line 676
  xor A, A
  // line 677
  ld C, 0 + BEEPER1_WL_CT_VAL
  ld (sd6play_beeper1_wl_1+1), A
  // line 678
  inc A
  // line 679
  ld IXH, A
  ld IXL, A
  ld IYL, A
  ld IYH, A
  // line 680
  ld A, 7
  ld (sd6play_modVolSpeed0_1+1), A
  // line 681
  ld A, 0 + OPCODE_JR
  ld (sd6play_slide0), A
  // line 682
  ld HL, 0 + ADDR_SD6_REP+0xff
  ld (sd6play_rep_1+1), HL
  // line 683
#if  BEEPER1_SYNC
  ld (sd6play_beeper1_sync), A
  // line 685
#endif 
  // line 689
sd6play_loop:
  // line 692
  SD6PLAY_NOTE_LEAD B, IXH, H, L, HL, sd6play_rep_1, sd6play_setVols + 1, sd6play_beeper0_wl_1, sd6play_slide0, sd6play_modVolSpeed0_1, sd6play_beeper1_sync, sd6play_end
  // line 695
  SD6PLAY_NOTE_BASE D, IXL, H, L, HL, sd6play_setVols, sd6play_beeper2_wl
  // line 697
sd6play_setVols:
  ld HL, 0
  // line 698
  exx
  // line 699
  SD6PLAY_NOTE_CHORD2 B, C, BC, IYH, H, L, HL, D, sd6play_beeper3_wl, sd6play_beeper4_wl
  // line 700
  SD6PLAY_NOTE_DRUM IYL, L, sd6play_beeper5_drumNr
  // line 701
sd6play_tempo_1:
  // line 702
  ld HL, 0 + ADDR_SD6_DRUM|0
  // line 705
sd6play_beeperLoop:
  // line 706
    // line 707
    exx
    // line 710
    ld E, 0 + PULSE_L
    // line 711
    SD6PLAY_BEEPER_WAVE_BR B, E, H, sd6play_beeper0_wl_1
    // line 712
    SD6PLAY_BEEPER_WAVE_R C, E, H, sd6play_beeper1_wl_1
    // line 713
    SD6PLAY_BEEPER_WAVE_R D, E, L, sd6play_beeper2_wl
    // line 714
    ld A, E
    exx
    ld E, A
    // line 715
    SD6PLAY_BEEPER_WAVE_BR B, E, D, sd6play_beeper3_wl
    // line 716
    SD6PLAY_BEEPER_WAVE_R C, E, D, sd6play_beeper4_wl
    // line 717
    SD6PLAY_BEEPER_DRUM HL, E, sd6play_beeper5_drumNr
    // line 719
    ld A, E
    ld (MMIO_8253_CTRL), A
    // line 720
    inc L
    jp nz, 0 + sd6play_beeperLoop
  // line 723
  exx
  // line 726
  ld A, IXH
  and A, 3
  LEAD_ECHO_DELAY 
  jr nz, sd6play__endif2
      // line 728
#if  BEEPER1_SYNC                                        
      // line 729
sd6play_beeper1_sync:
      jr 0 + sd6play_beeper1_syncEnd
        // line 730
        ld C, 0 + BEEPER1_WL_CT_VAL
        // line 731
        ld A, 0 + OPCODE_JR
        ld (sd6play_beeper1_sync), A
      // line 733
sd6play_beeper1_syncEnd:
      // line 734
#endif 
      ld A, (sd6play_beeper0_wl_1+1)
      ld (sd6play_beeper1_wl_1+1), A
  sd6play__endif2:

  // line 738
  ld A, (MMIO_8255_PORTB)
  cpl
  and A, 0 + KEY9_F1_MASK|KEY9_F2_MASK|KEY9_F3_MASK|KEY9_F4_MASK
  jp z, 0 + sd6play_loop
  // line 739
  jr 0 + sd6play_waitUntilKeyOff2
  // line 742
sd6play_end:
  // line 743
  xor A, A
  // line 745
sd6play_waitUntilKeyOff2:
  // line 746
  ex AF, AF
    // line 747
    sd6play__loop_top3: // do {
        // line 748
        ld A, (MMIO_8255_PORTB)
        cpl
        and A, 0 + KEY9_F1_MASK|KEY9_F2_MASK|KEY9_F3_MASK|KEY9_F4_MASK
    sd6play__loop_end3:
    jr nz, sd6play__loop_top3 // } while (nz_jr)
    sd6play__loop_exit3: // loop exit

    // line 752
    ld A, 0 + MMIO_8253_CT0_MODE3
    ld (MMIO_8253_CTRL), A
  ex AF, AF
  // line 755
  BANKH_RAM 
  // line 757
sd6play_restoreSP_1:
  ld SP, 0
  // line 758
  pop IX
__endasm;
}

#pragma restore
