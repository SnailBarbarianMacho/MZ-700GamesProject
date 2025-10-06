/**** This file is made by ../tools/z80ana.php.  DO NOT MODIFY! ****/





#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"

#include "../../../../src-common/asm-macros.h"
#include "assert.h"
#include "input.h"


u8          input_;
static u8   input_old_;              
u8          input_trg_;
u8          input_joy_;
u8          input_joy_mode_;
u8          input_mz1x03_sensitivity_;


void inputInit(void) __z88dk_fastcall
{
    input_     = 0;
    input_old_ = 0;
    input_trg_ = 0;
    input_joy_mode_ = INPUT_JOY_MODE_AM7J_DETECTING;
    input_mz1x03_sensitivity_ = 0;
}


static const u8 INPUT_TAB_[] = {
    
    0xf1,      1, 0x40, INPUT_MASK_A,
    0xf2,      5, 0x20, INPUT_MASK_D, 0x08, INPUT_MASK_U, 0x04, INPUT_MASK_A, 0x02, INPUT_MASK_U, 0x01, INPUT_MASK_B,
    0xf3,      3, 0x40, INPUT_MASK_B, 0x20, INPUT_MASK_R, 0x08, INPUT_MASK_D,
    0xf4,      4, 0x80, INPUT_MASK_L, 0x40, INPUT_MASK_B, 0x10, INPUT_MASK_R, 0x01, INPUT_MASK_L,
    0xf6,      1, 0x10, INPUT_MASK_A,
    0xf7,      4, 0x20, INPUT_MASK_U, 0x10, INPUT_MASK_D, 0x08, INPUT_MASK_R, 0x04, INPUT_MASK_L,
    0xf9,      4, 0x80, INPUT_MASK_START, 0x40, INPUT_MASK_CANCEL, 0x20, INPUT_MASK_START, 0x10, INPUT_MASK_CANCEL,
    0,
};



static void inputKey_(void) __z88dk_fastcall __naked
{
__asm
    BANKH_VRAM_MMIO C           

    
    ld      A, (_input_)
    ld      (_input_old_), A

    ld      HL, 0 + _INPUT_TAB_
    ld      DE, 0 + MMIO_8255_PORTA
    ld      C,  0x00           

    
STROBE_LOOP:
    ld      A,  (HL)             
    or      A
    jr      z,  STROBE_LOOP_END
    inc     HL
    ld      (DE), A             
    inc     E                   
    ld      A,  (DE)            
    ld      E,  A

    ld      B,  (HL)            
    inc     HL

    
KEY_LOOP:
    ld      A,  (HL)            
    inc     HL
    and     A,  E
    jp      nz, KEY_LOOP_END
    ld      A,  (HL)
    or      A,  C               
    ld      C,  A               
KEY_LOOP_END:
    inc     HL
    djnz    B,  KEY_LOOP
    ld      E,  0 + MMIO_8255_PORTA & 0xff
    jp      STROBE_LOOP

STROBE_LOOP_END:
    ld      A,  C
    ld      (_input_), A
    BANKH_RAM C                 
    ret
__endasm;
}



static void inputAM7J_(void) __z88dk_fastcall __naked
{
__asm
    
    BANKH_VRAM_MMIO C           

    
    ld      HL, 0 + MMIO_ETC
    ld      DE, 0x0e00 | MMIO_ETC_JA2_MASK

    
    ld      A,  E               
    and     A,  (HL)            
    jp      nz, AM7J_H_DETECTED 
    ld      A,  E               
    and     A,  (HL)            
    jp      nz, AM7J_H_DETECTED 
    ld      A,  E               
    and     A,  (HL)            
    jp      nz, AM7J_H_DETECTED 
    ld      A,  E               
    and     A,  (HL)            
    jp      nz, AM7J_H_DETECTED 
    ld      A,  E               
    and     A,  (HL)            
    jp      nz, AM7J_H_DETECTED 
    ld      A,  E               
    and     A,  (HL)            
    
    jr      z, AM7J_NOT_DETECTED 

AM7J_H_DETECTED:
    
    and     A, (HL)             
    jr      z, AM7J_L_DETECTED  
    and     A, (HL)             
    jr      z, AM7J_L_DETECTED  
    and     A, (HL)             
    jr      z, AM7J_L_DETECTED  
    and     A, (HL)             
    jr      z, AM7J_L_DETECTED  
    and     A, (HL)             
    jr      z, AM7J_L_DETECTED  
    and     A, (HL)             
    jr      z, AM7J_L_DETECTED  
    and     A, (HL)             
    
    jr      z, AM7J_L_DETECTED  

    
AM7J_NOT_DETECTED: 
    ld      A,  0 + INPUT_MASK_NC
    jp      AM7J_END

    
AM7J_L_DETECTED:
    
    ld      A,  (HL)            
    
    rrca                        
    rrca                        
    rl      D                   

    ld      A,  0 + MMIO_ETC_JA_MASK
    

    
    and     A,  (HL)            
    
    add     A,  D               
    and     A,  0x33            
    rrca                        
    rrca                        
    ld      D,  A               
    

    
    ld      A,  (HL)            
    
    and     A,  0 + MMIO_ETC_JA_MASK
    add     A,  D               
    rlca                        
    rlca                        
    ld      D,  A               
    

    
    ld      A,  (HL)            
    
    rlca                        
    and     A,  E               
    or      A,  D               

    
#if 0    
    or      A,  0xc0            
    ld      E,  A
    and     A,  0x0c            
    ld      A,  E               
    jp      nz, JEND2           
                                
                                
    and     A,  0x30            
    dec     A                   
    add     A,  A               
    add     A,  A               
    or      A,  D               
JEND2:
    cpl     A                   
#else
    xor     A,  0x3f            
#endif
AM7J_END:
    ld      (_input_joy_), A
    BANKH_RAM C                 
    ret

#if 0
    

    
AM7J_ND_STAT_DETECTED:
    ld      A, (_input_am7j_ct_)
    inc     A
    cmp     A, 0 + AM7J_CT
    ld      (_input_am7j_ct_), A
    ld      A, 0
    jp      nz, AM7J_END        
    ld      (_input_am7j_ct_), A
    ld      (_input_am7j_detected_), A
    jp      AM7J_END            



    
    ld      D, A                
      ld    A, (_input_am7j_detected_)
      or    A, A
    ld      A, D                
    jp      nz, AM7J_END
    
AM7J_D_STAT_NOT_DETECTED:
    ld      A, (_input_am7j_ct_)
    inc     A
    cmp     A, 0 + AM7J_CT
    ld      (_input_am7j_ct_), A
    ld      A, 0x00             
    jp      nz, AM7J_END
    dec     A                   
    ld      (_input_am7j_detected_), A
    inc     A
    ld      (_input_am7j_ct_), A
AM7J_END:

    
    or      A, C
    ld      (_input_), A
    ld      E, A

    
    ld      A, (_input_old_)
    cpl     A                   
    and     A, E
    ld      (_input_trg_), A
#endif
__endasm;
}


extern int _input_joy_;
extern int _input_;
extern int _input_old_;
extern int _input_trg_;


static void inputMain2_(void) 
{
__asm
// line 278
  ld A, (_input_joy_)
  and A, 0 + (~INPUT_MASK_NC) & 0xff
  ld B, A
// line 280
  ld A, (_input_)
  or A, B
  ld (_input_), A
  ld B, A
// line 283
  ld A, (_input_old_)
  cpl
  and A, B
  ld (_input_trg_), A
__endasm;
}



void inputMain(void) __z88dk_fastcall
{
    
    inputKey_();
    if (input_joy_mode_ < INPUT_JOY_MODE_MZ1X03_DETECTING) {
        
        inputAM7J_();
        if (input_joy_mode_ < INPUT_JOY_MODE_AM7J_DETECTED) {
            
            if (input_joy_ != INPUT_MASK_NC) {
                
                input_joy_mode_++;
            } else {
                
                input_joy_mode_ = INPUT_JOY_MODE_MZ1X03_DETECTING;
            }
        } else {
            
            if (input_joy_ == INPUT_MASK_NC) {
                
                input_joy_mode_++;
            } else {
                
                input_joy_mode_ = INPUT_JOY_MODE_AM7J_DETECTED;
            }
        }
    } else {
        
        if (input_joy_mode_ < INPUT_JOY_MODE_MZ1X03_DETECTED) {
            
            if (input_joy_ != INPUT_MASK_NC) {
                
                input_joy_mode_++;
            } else {
                
                input_joy_mode_ = INPUT_JOY_MODE_AM7J_DETECTING;
            }
        } else {
            
            if (input_joy_ == INPUT_MASK_NC) {
                
                input_joy_mode_++;
                if (INPUT_JOY_MODE_MAX < input_joy_mode_) {
                    input_joy_mode_ = INPUT_JOY_MODE_AM7J_DETECTING;
                }
            } else {
                
                input_joy_mode_ = INPUT_JOY_MODE_MZ1X03_DETECTED;
            }
        }
    }

    inputMain2_();
}






#pragma save
#pragma disable_warning 85          

void inputSetMZ1X03Enabled(bool const enabled)  __z88dk_fastcall
{
__asm
// line 353
  extern inputMZ1X03_enabled
// line 354
  extern _input_joy_mode_
// line 355
  extern _input_joy_
// line 362
  dec L
// line 363
  ld HL, 0 + inputMZ1X03_enabled
// line 364
  ld (HL), 0 + OPCODE_JP_NZ
// line 365
  jr nz, inputSetMZ1X03Enabled__0
// line 366
      ld (HL), 0 + OPCODE_LD_BC_NN
inputSetMZ1X03Enabled__0:
// line 371
  ld A, 0 + INPUT_JOY_MODE_MZ1X03_DETECTED
// line 372
  ld (_input_joy_mode_), A
// line 374
  xor A, A
// line 375
  ld (_input_joy_), A
__endasm;
}

#pragma restore


#pragma save
#pragma disable_warning 59          

bool inputIsMZ1X03Enabled(void) 
{
__asm
// line 386
  extern inputMZ1X03_enabled
// line 389
  ld A, (inputMZ1X03_enabled)
  cp A, 0 + OPCODE_JP_NZ
// line 390
  ld L, 0 + false
  ret z
// line 391
  inc L
__endasm;
}

#pragma restore


#pragma save
#pragma disable_warning 85          

void inputMZ1X03ButtonVSyncAxis1(u8 const mz1x03_sensitivity)  __z88dk_fastcall
{
__asm
// line 430
  extern inputMZ1X03_button_1
// line 431
  extern inputMZ1X03_vSyncLoop
// line 432
  extern inputMZ1X03_wait1Loop
// line 433
  extern inputMZ1X03_wait2Loop
// line 434
  extern inputMZ1X03_notDetected
// line 435
  extern _input_joy_
// line 437
  BANKH_VRAM_MMIO C
// line 439
  ld E, L
// line 441
#if  DEBUG
  ld A, (MMIO_8255_PORTC)
  and A, A
// line 444
#endif 
// line 446
  ld D, 0 + MMIO_ETC_JA1_MASK | MMIO_ETC_JA2_MASK
// line 447
  ld HL, 0 + MMIO_ETC
// line 448
  ld A, (HL)
// line 449
  cpl
// line 450
  and A, D
// line 453
  rrca
// line 454
  ld (inputMZ1X03_button_1 + 1), A
// line 457
  ld A, H
// line 458
  ld L, 0 + MMIO_8255_PORTC & 0xff
// line 460
inputMZ1X03_vSyncLoop:
// line 461
    and A, (HL)
  jp m, 0 + inputMZ1X03_vSyncLoop
// line 463
inputMZ1X03_vSync:
// line 466
  ld L, 0 + MMIO_ETC & 0xff
// line 467
  ld B, 0 + 6
// line 468
inputMZ1X03_wait1Loop:
  djnz B, 0 + inputMZ1X03_wait1Loop
// line 473
  ld A, (HL)
// line 474
  and A, D
// line 475
inputMZ1X03_enabled:
// line 476
  jp nz, 0 + inputMZ1X03_notDetected
// line 480
  ld A, E
// line 481
  add A, A
  add A, E
// line 482
  add A, A
  add A, A
  add A, A
  add A, A
// line 483
  add A, 0 + 9
// line 484
  exx
// line 485
    ld B, A
// line 486
inputMZ1X03_wait2Loop:
// line 487
      inc HL
    djnz B, 0 + inputMZ1X03_wait2Loop
  exx
// line 493
inputMZ1X03_readA:
// line 494
  ld A, (HL)
// line 497
  and A, D
// line 498
  rrca
// line 499
  ld (_input_joy_), A
// line 501
  BANKH_RAM C
// line 503
  ret
// line 505
inputMZ1X03_notDetected:
// line 506
  ld A, 0 + INPUT_MASK_NC
  ld (_input_joy_), A
// line 507
  BANKH_RAM C
__endasm;
}

#pragma restore


void inputMZ1X03Axis2(void)  __naked
{
__asm
// line 515
  extern _input_joy_
// line 516
  extern INPUT_MZ1X03_TAB
// line 519
  ld A, (_input_joy_)
// line 520
  cp A, 0 + INPUT_MASK_NC
  ret z
// line 521
  ld E, A
// line 523
  BANKH_VRAM_MMIO C
// line 524
inputMZ1X03_readB:
// line 525
  ld A, (MMIO_ETC)
// line 528
  and A, 0 + MMIO_ETC_JA1_MASK | MMIO_ETC_JA2_MASK
// line 529
  add A, A
// line 530
  or A, E
// line 531
  ld HL, 0 + INPUT_MZ1X03_TAB
// line 532
  ld D, 0 + 0x00
  ld E, A
  add HL, DE
// line 533
  ld A, (HL)
// line 534
inputMZ1X03_button_1:
  or A, 0 + 0x00
// line 535
  BANKH_RAM C
// line 536
  ld (_input_joy_), A
// line 537
  ret
// line 539
INPUT_MZ1X03_TAB:
// line 540
db INPUT_MASK_R | INPUT_MASK_D, INPUT_MASK_NC, INPUT_MASK_NC, INPUT_MASK_NC
// line 541
db INPUT_MASK_D, INPUT_MASK_L | INPUT_MASK_D, INPUT_MASK_NC, INPUT_MASK_NC
// line 542
db INPUT_MASK_R, INPUT_MASK_NC, INPUT_MASK_R | INPUT_MASK_U, INPUT_MASK_NC
// line 543
db 0, INPUT_MASK_L, INPUT_MASK_U, INPUT_MASK_L | INPUT_MASK_U
__endasm;
}

