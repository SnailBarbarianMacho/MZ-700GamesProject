/**** This file is made by ../tools/aal80.php.  DO NOT MODIFY! ****/





#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"

#include "addr.h"
#include "input.h"
#include "vvram.h"
#include "math.h"


u8 b_vram_trans_enabled_;
#if DEBUG
static u16 ct_8253_ct12_val_;  
static u16 ct_8253_ct12_diff_; 
u8  vram_8253_ct2_;            
u16 vram_trans_v_counter_;     
#endif


#define CT_8253_CT1_NTSC    262 
#define CT_8253_CT1_PAL     312 


#define ADDR_TMP_SP    (VVRAM_TMP_WORK + 16) 



static void vramTransInit_(void) __z88dk_fastcall
{
__asm
    
    macro   POP_10_BYTES_TO_REGISTERS
        pop     HL
        pop     DE
        pop     BC
        exx
        pop     DE
        pop     BC
    endm

    
    macro   PUSH_10_BYTES_FROM_REGISTERS
        push    BC
        push    DE
        exx
        push    BC
        push    DE
        push    HL
    endm

    macro   PUSH_DE_10_BYTES
        push    DE
        push    DE
        push    DE
        push    DE
        push    DE
    endm

    BANKH_VRAM_MMIO                    

    
#if DEBUG
    
    
    
    
    ld      HL, 0 + MMIO_8253_CTRL
    ld      (HL), 0 + MMIO_8253_CTRL_RL_LATCH_MASK |MMIO_8253_CTRL_CT1_MASK
    dec     L       
    ld      A, (HL) 
    ld      (_vram_8253_ct2_), A
    ld      B, A
    dec     L       
    ld      E, (HL) 
    ld      D, (HL) 
    srl     D       
    rr      E
    srl     E
    srl     E
    srl     E
    ld      D, 0x00
    ld      HL, 0 + VRAM_TRANS_DEBUG_CT_TAB
    add     HL, DE
    ld      C, (HL)
section rodata_compiler
VRAM_TRANS_DEBUG_CT_TAB:
    db      0x00, 0x0f, 0x1e, 0x2d
    db      0x3c, 0x4b, 0x5a, 0x69
    db      0x78, 0x87, 0x96, 0xa5
    db      0xb4, 0xc3, 0xd2, 0xe1
    db      0xf0
section code_compiler
    ld      HL, (_ct_8253_ct12_val_)
    ld      (_ct_8253_ct12_val_), BC
    sbc     HL, BC 
    ld      (_ct_8253_ct12_diff_), HL
#endif
    
    ld      HL, 0 + VVRAM_TEXT_ADDR(0, 0)
    ld      (vvramTrans_textSrc0_1 + 1), HL  
    ld      HL, 0 + VRAM_TEXT_ADDR(10, 0)
    ld      (vvramTrans_textDst0_1 + 1), HL  

    
#if 0
    
    
    
    
    
    
    
    xor     A
    ld      HL,    0 + MMIO_8255_PORTC
VBLK_SYNC0:
    or      (HL)                            
    jp      A, p,    VBLK_SYNC0
VBLK_SYNC1:
    and     (HL)                            
    jp      A, m,    VBLK_SYNC1
#endif

    BANKH_RAM                               
__endasm;
}





#pragma save
#pragma disable_warning 85                              
static void  vvramTransMain_(const u8 lines) __z88dk_fastcall
{
__asm
  // line 138
    // line 140
    extern vvramTrans_restoreSP_1, vvramTrans_loop
    // line 141
    extern vvramTrans_textSrc0_1, vvramTrans_textDst0_1
    // line 142
    extern vvramTrans_textSrc1_1, vvramTrans_textDst1_1
    // line 143
    extern vvramTrans_textSrc2_1, vvramTrans_textDst2_1
    // line 144
    extern vvramTrans_textSrc3_1, vvramTrans_textDst3_1
    // line 145
    extern vvramTrans_atbSrc0_1, vvramTrans_atbDst0_1
    // line 146
    extern vvramTrans_atbSrc1_1, vvramTrans_atbDst1_1
    // line 147
    extern vvramTrans_atbSrc2_1, vvramTrans_atbDst2_1
    // line 148
    extern vvramTrans_atbSrc3_1, vvramTrans_atbDst3_1
    // line 150
    ld A, L
    and A, A
    ret z
    // line 151
    ld (vvramTrans_restoreSP_1+1), SP
    // line 152
    BANKH_VRAM_MMIO 
    // line 155
vvramTrans_loop:
    // line 156
      // line 158
vvramTrans_textSrc0_1:
      ld SP, 0
      // line 159
      pop HL
      pop DE
      pop BC
      exx
      pop DE
      pop BC
      // line 160
      ld (vvramTrans_textSrc1_1+1), SP
      // line 161
vvramTrans_textDst0_1:
      ld SP, 0
      // line 162
      push BC
      push DE
      exx
      push BC
      push DE
      push HL
      // line 163
      ld HL, 20
      add HL, SP
      // line 164
      ld (vvramTrans_textDst1_1+1), HL
      // line 167
vvramTrans_textSrc1_1:
      ld SP, 0
      // line 168
      pop HL
      pop DE
      pop BC
      exx
      pop DE
      pop BC
      // line 169
      ld (vvramTrans_textSrc2_1+1), SP
      // line 170
vvramTrans_textDst1_1:
      ld SP, 0
      // line 171
      push BC
      push DE
      exx
      push BC
      push DE
      push HL
      // line 172
      ld HL, 20
      add HL, SP
      // line 173
      ld (vvramTrans_textDst2_1+1), HL
      // line 176
vvramTrans_textSrc2_1:
      ld SP, 0
      // line 177
      pop HL
      pop DE
      pop BC
      exx
      pop DE
      pop BC
      // line 178
      ld (vvramTrans_textSrc3_1+1), SP
      // line 179
vvramTrans_textDst2_1:
      ld SP, 0
      // line 180
      push BC
      push DE
      exx
      push BC
      push DE
      push HL
      // line 181
      ld HL, 20
      add HL, SP
      // line 182
      ld (vvramTrans_textDst3_1+1), HL
      // line 185
vvramTrans_textSrc3_1:
      ld SP, 0
      // line 186
      pop HL
      pop DE
      pop BC
      exx
      pop DE
      pop BC
      // line 187
      ld HL, 0 + VVRAM_GAPX
      add HL, SP
      // line 188
      ld (vvramTrans_atbSrc0_1+1), HL
      // line 189
vvramTrans_textDst3_1:
      ld SP, 0
      // line 190
      push BC
      push DE
      exx
      push BC
      push DE
      push HL
      // line 191
      ld HL, 0 + (VRAM_ATB-VRAM_TEXT)-20
      add HL, SP
      // line 192
      ld (vvramTrans_atbDst0_1+1), HL
      // line 195
vvramTrans_atbSrc0_1:
      ld SP, 0
      // line 196
      pop HL
      pop DE
      pop BC
      exx
      pop DE
      pop BC
      // line 197
      ld (vvramTrans_atbSrc1_1+1), SP
      // line 198
vvramTrans_atbDst0_1:
      ld SP, 0
      // line 199
      push BC
      push DE
      exx
      push BC
      push DE
      push HL
      // line 200
      ld HL, 20
      add HL, SP
      // line 201
      ld (vvramTrans_atbDst1_1+1), HL
      // line 204
vvramTrans_atbSrc1_1:
      ld SP, 0
      // line 205
      pop HL
      pop DE
      pop BC
      exx
      pop DE
      pop BC
      // line 206
      ld (vvramTrans_atbSrc2_1+1), SP
      // line 207
vvramTrans_atbDst1_1:
      ld SP, 0
      // line 208
      push BC
      push DE
      exx
      push BC
      push DE
      push HL
      // line 209
      ld HL, 20
      add HL, SP
      // line 210
      ld (vvramTrans_atbDst2_1+1), HL
      // line 213
vvramTrans_atbSrc2_1:
      ld SP, 0
      // line 214
      pop HL
      pop DE
      pop BC
      exx
      pop DE
      pop BC
      // line 215
      ld (vvramTrans_atbSrc3_1+1), SP
      // line 216
vvramTrans_atbDst2_1:
      ld SP, 0
      // line 217
      push BC
      push DE
      exx
      push BC
      push DE
      push HL
      // line 218
      ld HL, 20
      add HL, SP
      // line 219
      ld (vvramTrans_atbDst3_1+1), HL
      // line 222
vvramTrans_atbSrc3_1:
      ld SP, 0
      // line 223
      pop HL
      pop DE
      pop BC
      exx
      pop DE
      pop BC
      // line 224
      ld HL, 0 + VVRAM_WIDTH-VRAM_WIDTH-VVRAM_GAPX-VRAM_WIDTH
      add HL, SP
      // line 225
      ld (vvramTrans_textSrc0_1+1), HL
      // line 226
vvramTrans_atbDst3_1:
      ld SP, 0
      // line 227
      push BC
      push DE
      exx
      push BC
      push DE
      push HL
      // line 228
      ld HL, 0 + (VRAM_TEXT-VRAM_ATB)+VRAM_WIDTH-20
      add HL, SP
      // line 229
      ld (vvramTrans_textDst0_1+1), HL
      // line 232
      dec A
      jp nz, 0 + vvramTrans_loop
    // line 235
vvramTrans_restoreSP_1:
    ld SP, 0
    // line 236
    BANKH_RAM 
__endasm;
}

#pragma restore









#pragma save
#pragma disable_warning 85                              
static void  WAIT_VSYNC(int reg_A, int reg_HL) __naked
{
__asm
WAIT_VSYNC macro reg_A, reg_HL
local WAIT_VSYNC__loop_top0, WAIT_VSYNC__loop_end0, WAIT_VSYNC__loop_exit0, WAIT_VSYNC__loop_top1, WAIT_VSYNC__loop_end1, WAIT_VSYNC__loop_exit1
  // line 251
    // line 254
    xor A, A
    // line 255
    WAIT_VSYNC__loop_top0: // do {
        or A, (HL)
    WAIT_VSYNC__loop_end0:
    jp p, WAIT_VSYNC__loop_top0 // } while (p)
    WAIT_VSYNC__loop_exit0: // loop exit

    // line 256
    WAIT_VSYNC__loop_top1: // do {
        and A, (HL)
    WAIT_VSYNC__loop_end1:
    jp m, WAIT_VSYNC__loop_top1 // } while (m)
    WAIT_VSYNC__loop_exit1: // loop exit

endm
__endasm;
}

#pragma restore


void  vvramInit(void) __z88dk_fastcall __naked
{
__asm
  // line 264
    // line 266
    extern _b_vram_trans_enabled_
    // line 268
    extern _vvramClear
    // line 270
    ld A, true
    ld (_b_vram_trans_enabled_), A
    // line 280
    BANKH_VRAM_MMIO 
    // line 283
    ld HL, 0 + MMIO_8253_CTRL
    // line 284
    ld (HL), 0 + MMIO_8253_CTRL_CT1(MMIO_8253_CTRL_RL_LH_MASK,MMIO_8253_CTRL_MODE2_MASK)
    // line 285
#if  DEBUG
    ld (HL), 0 + MMIO_8253_CTRL_CT2(MMIO_8253_CTRL_RL_L_MASK,MMIO_8253_CTRL_MODE2_MASK)
    // line 287
    dec L
    // line 288
    ld (HL), 0
    // line 289
#endif 
    // line 297
    ld HL, 0 + MMIO_8255_PORTC
    // line 298
    WAIT_VSYNC A, HL
    // line 299
    ld BC, 768
    vvramInit__loop_top0: // do {
        dec BC
        ld A, B
        or A, C
    vvramInit__loop_end0:
    jr nz, vvramInit__loop_top0 // } while (nz_jr)
    vvramInit__loop_exit0: // loop exit

    // line 301
    bit 0 + MMIO_8255_PORTC_VBLK_SHIFT, (HL)
    // line 302
    ld DE, 0 + CT_8253_CT1_NTSC
    ld BC, 2269
    // line 303
    jr nz, vvramInit__endif1
        // line 304
        ld DE, 0 + CT_8253_CT1_PAL
        ld BC, 2696
    vvramInit__endif1:

    // line 314
    WAIT_VSYNC A, HL
    // line 315
    vvramInit__loop_top2: // do {
        dec BC
        ld A, B
        or A, C
    vvramInit__loop_end2:
    jr nz, vvramInit__loop_top2 // } while (nz_jr)
    vvramInit__loop_exit2: // loop exit

    // line 317
    ld L, 0 + MMIO_8253_CT1&0xff
    // line 318
    ld (HL), E
    ld (HL), D
    // line 320
    BANKH_RAM 
    // line 324
    jp 0 + _vvramClear
__endasm;
}



#pragma save
#pragma disable_warning 59                              
static u8 vvramGetVCounter_(void) __z88dk_fastcall
{
__asm
    BANKH_VRAM_MMIO                                     
    
    ld      HL, 0 + MMIO_8253_CTRL
    ld      (HL), 0 + MMIO_8253_CTRL_RL_LATCH_MASK | MMIO_8253_CTRL_CT1_MASK
    ld      L,  0 + (MMIO_8253_CT1 & 0xff)
    ld      A,  (HL)                                    
    ld      H,  (HL)                                    
    ld      L,  A
    BANKH_RAM                                           
    
#if 0 
#define VCT 260
    cmp     A, 0 + (VCT & 0xff)
    jp      nz, VRAM_GET_VCOUNTER_TEST1 + 1
    ld      A, H
    cmp     A, 0 + (VCT >> 8)
    jp      nz, VRAM_GET_VCOUNTER_TEST1 + 1
VRAM_GET_VCOUNTER_TEST1 + 1
    nop;
#undef VCT
#endif
    
    dec     HL
    srl     H
    rr      L
    srl     L
    srl     L
    
    ld      A, 0 + (VRAM_HEIGHT - 4)
    cmp     A, L
    ret     nc
    ld      L, A
__endasm;
}
#pragma restore


void vvramTrans(void) __z88dk_fastcall __naked
{
    
    if (!b_vram_trans_enabled_) {
        b_vram_trans_enabled_ = true; 
__asm
        ret
__endasm;
    }
    vramTransInit_();

    if (inputGetJoyMode() < INPUT_JOY_MODE_MZ1X03_DETECTING) {
        
        vvramTransMain_(VRAM_HEIGHT);
    } else {
        
        
        
        
        
        
        

        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        

        u8 vc = vvramGetVCounter_(); 
        u8  s = inputGetMZ1X03sensitivity(); 
        vvramTransMain_(vc);
        
        inputMZ1X03ButtonVSyncAxis1(s);
        s = 4 - s;
        vvramTransMain_(s);
        inputMZ1X03Axis2();
        vvramTransMain_(VRAM_HEIGHT - vc - s);
    }

    vvramClear();                                       
}



#if DEBUG
u16 vramDebugGetProcessTime(void)
{
    
    
    return ct_8253_ct12_diff_ / 16;  
}
#endif



void vvramClear(void) __z88dk_fastcall
{
__asm
    ld      (VRAM_CLEAR_SP_RESTORE + 1), SP             

    ld      B,  VRAM_HEIGHT                             
    ld      DE, 0x0000
    ld      SP, 0 + VVRAM_TEXT_ADDR(VRAM_WIDTH, 0)
VVRAM_CLEAR_LOOP:
    PUSH_DE_10_BYTES                                    
    PUSH_DE_10_BYTES
    PUSH_DE_10_BYTES
    PUSH_DE_10_BYTES
    ld      HL, 0 + VRAM_WIDTH + VVRAM_GAPX + VRAM_WIDTH
    add     HL, SP
    ld      SP, HL
    PUSH_DE_10_BYTES                                    
    PUSH_DE_10_BYTES
    PUSH_DE_10_BYTES
    PUSH_DE_10_BYTES
    ld      HL, 0 + (VVRAM_WIDTH - VVRAM_GAPX)
    add     HL, SP
    ld      SP, HL
    djnz    B, VVRAM_CLEAR_LOOP
VRAM_CLEAR_SP_RESTORE:
    ld      SP, 0000                                    
__endasm;
}



#pragma save
#pragma disable_warning 85                              
void vvramFillRect(const u8* const draw_addr, const u16 wh, const u16 code)
{
__asm
    pop     HL                      
    pop     HL                      
    pop     BC                      
    pop     DE                      
    ld      A, C                    
    exx
        ld  B, A                    
    exx
    ld      A, B                    

    push    BC                      
    push    HL                      

    
    exx
VRAM_FILL_RECT_TEXT_Y:
        exx
        ld      C, L                
        ld      B, A                
VRAM_FILL_RECT_TEXT_X:
            ld      (HL), E         
            inc     L               
            djnz    B, VRAM_FILL_RECT_TEXT_X

        ld      L, C                
        inc     H                   
        exx
            djnz B, VRAM_FILL_RECT_TEXT_Y
        exx

    
    pop     HL                      
    ld      BC, 0 + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))
    add     HL, BC
    pop     BC                      
    ld      A, C                    
    exx
        ld  B, A                    
    exx
    ld      A, B                    

    exx
VRAM_FILL_RECT_ATB_Y:
        exx
        push    HL
        ld      B, A                
VRAM_FILL_RECT_ATB_X:
            ld      (HL), D         
            inc     L               
            djnz    B, VRAM_FILL_RECT_ATB_X

        pop     HL
        inc     H                   
        exx
            djnz B, VRAM_FILL_RECT_ATB_Y
        

    
    ld      HL, -8
    add     HL, SP
    ld      SP, HL
__endasm;
}
#pragma restore


#pragma save
#pragma disable_warning 85                              
void vramFill(const u16 code) __z88dk_fastcall
{
    
    
__asm
    ld      (VRAM_FILL_SP_RESTORE + 1), SP              
    BANKH_VRAM_MMIO                                     

    ld      BC, HL                                      
    ld      SP, 0 + VRAM_TEXT_ADDR(10, 0)
    ld      A,  VRAM_HEIGHT * 4                         

VRAM_FILL_LOOP:
        
        ld      D, C                                    
        ld      E, C                                    
        PUSH_DE_10_BYTES

        ld  HL, 10 + VRAM_ATB - VRAM_TEXT
        add HL, SP
        ld  SP, HL

        
        ld      D, B                                    
        ld      E, B                                    
        PUSH_DE_10_BYTES

        ld  HL, 20 + VRAM_TEXT - VRAM_ATB
        add HL, SP
        ld  SP, HL

        
        dec A
        jr  nz, VRAM_FILL_LOOP

    BANKH_RAM                                           
VRAM_FILL_SP_RESTORE:
    ld      SP, 0x0000                                  
__endasm;
}
#pragma restore



void vvramDrawRect(const u8* const draw_addr, const u8* const stc_addr, const u16 wh)
{
__asm
    pop     HL                                          
    pop     DE                                          
    pop     HL                                          
    pop     BC                                          

    
    push    DE                                          

    
    ld      A, C                                        
    ld      C, B                                        
    ld      B, 0                                        
    exx
        ld  B, A                                        
        ld  C, A                                        
VRAM_DRAW_RECT_TEXT_Y:
    exx
    push    BC                                          
    ld      A, E                                        
    ldir
    pop     BC
    
    ld      E, A                                        
    inc     D
    exx
        djnz B, VRAM_DRAW_RECT_TEXT_Y
    exx
    pop     DE                                          

    
    
    push    HL
    ld      HL, 0 + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
    add     HL, DE
    ld      DE, HL
    pop     HL
    exx
        ld  B, C                                        
VRAM_DRAW_RECT_ATB_Y:
    exx
    push    BC                                          
    ld      A, E                                        
    ldir
    pop     BC
    ld      E, A                                        
    inc     D                                           
    exx
        djnz B, VRAM_DRAW_RECT_ATB_Y
    exx

    
    ld      HL, -8
    add     HL, SP
    ld      SP, HL
__endasm;
}


void vvramDrawRectTransparent(const u8* const draw_addr, const u8* const stc_addr, const u16 wh)
{
__asm
    pop     HL                                          
    pop     DE                                          
    pop     HL                                          
    pop     BC                                          

    
    
    ld      A, C                                        
    ld      C, B                                        
    exx
        ld  B, A                                        
VRAM_DRAW_RECT_TRANS_Y:
    exx
    ld      B, C                                        
VRAM_DRAW_RECT_TRANS_X:
        
        ld      A, (HL)
        inc     HL
        or      A
        jr      z, VRAM_DRAW_RECT_TRANS                 
            ld      (DE), A
            
            ld      A, E
            add     A, 0 + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
            ld      E, A
            
            ld      A, (HL)
            inc     HL
            ld      (DE), A
            
            ld      A, E
            sub     A, 0 + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
            ld      E, A
VRAM_DRAW_RECT_TRANS:
    
    inc     E
    djnz    B, VRAM_DRAW_RECT_TRANS_X
    
    ld      A, E
    sub     A, C
    ld      E, A
    
    inc     D
    exx
        djnz B, VRAM_DRAW_RECT_TRANS_Y
    exx

    
    ld      HL, -8
    add     HL, SP
    ld      SP, HL
__endasm;
}


void vramDrawRect(const u8* const draw_addr, const u8* const stc_addr, const u16 wh)
{
__asm
    ld      (VRAM_DRAW_RECT_SP_RESTORE + 1), SP         
    pop     HL                                          
    pop     DE                                          
    pop     HL                                          
    pop     BC                                          

    ld      SP, 0 + ADDR_TMP_SP                         

    ld      A, C                                        
    BANKH_VRAM_MMIO                                     
    ld      C, A                                        

    
    push    DE                                          

    ld      A, C                                        
    ld      C, B                                        
    ld      B, 0                                        
    exx
        ld  B, A                                        
        ld  C, A                                        
RVRAM_DRAW_RECT_TEXT_LOOP_Y:
    exx
    push    BC                                          
    push    DE
      ldir
    pop     DE
    pop     BC
    push    HL                                          
      ld    HL, 0 + VRAM_WIDTH
      add   HL, DE
      ld    DE, HL
    pop     HL
    exx
        djnz B, RVRAM_DRAW_RECT_TEXT_LOOP_Y
    exx
    pop     DE                                          

    
    push    HL
      ld    HL, 0 + (VRAM_ATB_ADDR(0, 0) - VRAM_TEXT_ADDR(0, 0))
      add   HL, DE
      ld    DE, HL
    pop     HL

    exx
        ld  B, C                                        
RVRAM_DRAW_RECT_ATB_LOOP_Y:
    exx
    push    BC                                          
    push    DE
      ldir
    pop     DE
    pop     BC
    push    HL                                          
      ld      HL, 0 + VRAM_WIDTH
      add     HL, DE
      ld      DE, HL
    pop     HL
    exx
        djnz B, RVRAM_DRAW_RECT_ATB_LOOP_Y
    

    
    BANKH_RAM                                           
VRAM_DRAW_RECT_SP_RESTORE:
    ld      SP, 0x0000                                  
__endasm;
}



void vvramDraw1x1(const u8* const draw_addr, const u16 code)
{
__asm
    pop     HL                                          
    pop     HL                                          
    pop     DE                                          

    
    ld      (HL), E

    
    ld      A, 0 + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))
    add     A, L
    ld      L, A

    
    ld      (HL), D

    
    ld      HL, -6
    add     HL, SP
    ld      SP, HL

    ret
__endasm;
}



void vvramDraw1x3(const u8* const draw_addr, const u8* const stc_addr)
{
__asm
    pop     HL                                          
    pop     DE                                          
    pop     HL                                          

    
    ldi
    ldi
    ldi

    
    ld      A, 0 + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) - 3)
    add     A, E
    ld      E, A

    
    ldi
    ldi
    ldi

    
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
__endasm;
}



void vvramDraw3x3(const u8* const draw_addr, const u8* const stc_addr)
{
__asm
    pop     HL                                          
    pop     DE                                          
    pop     HL                                          

    
    ldi
    ldi
    ldi
    inc     D                                           
    dec     E
    dec     E
    dec     E
    ldi
    ldi
    ldi
    inc     D
    dec     E
    dec     E
    dec     E
    ldi
    ldi
    ldi

    
    push    HL
    ld      HL, 0 + ((VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) - 0x0203)
    add     HL, DE
    ld      DE, HL
    pop     HL

    
    ldi
    ldi
    ldi
    inc     D
    dec     E
    dec     E
    dec     E
    ldi
    ldi
    ldi
    inc     D
    dec     E
    dec     E
    dec     E
    ldi
    ldi
    ldi

    
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
__endasm;
}


void vvramDraw3x3Transparent(const u8* const draw_addr, const u8* const stc_addr)
{
__asm





    macro VRAM_DRAW_3x3T
        local   label
        
        ld      A, (HL)
        inc     HL
        or      A
        jr      z, label
            ld      (DE), A
            
            ld      A, E
            add     A, C
            ld      E, A
            
            ld      A, (HL)
            inc     HL
            ld      (DE), A
            
            ld      A, E
            sub     A, C
            ld      E, A
label:
            inc     E
    endm


    macro   VRAM_DRAW_3x3T_
        local   label
        
        ld      A, (HL)
        inc     HL
        or      A
        jr      z, label
            ld      (DE), A
            
            ld      A, E
            add     A, C
            ld      E, A
            
            ld      A, (HL)
            inc     HL
            ld      (DE), A
            
            ld      A, E
            sub     A, C
            ld      E, A
label:
    endm


    pop     HL                      
    pop     DE                      
    pop     HL                      

    ld      C, 0 + (0x00 | (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)))
    VRAM_DRAW_3x3T
    VRAM_DRAW_3x3T
    VRAM_DRAW_3x3T_
    inc     D
    dec     E
    dec     E
    VRAM_DRAW_3x3T
    VRAM_DRAW_3x3T
    VRAM_DRAW_3x3T_
    inc     D
    dec     E
    dec     E
    VRAM_DRAW_3x3T
    VRAM_DRAW_3x3T
    VRAM_DRAW_3x3T_

    
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
__endasm;
}



void vvramDraw4x4(const u8* const draw_addr, const u8* const stc_addr)
{
__asm
    pop     HL                      
    pop     DE                      
    pop     HL                      
    ld      BC, 0x04ff              

    
    ldi
    ldi
    ldi
    ldi
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi

    
    push    HL
    ld      HL, 0 + ((VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) - 0x0304)
    add     HL, DE
    ld      DE, HL
    pop     HL

    
    ldi
    ldi
    ldi
    ldi
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi

    
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
__endasm;
}



void vvramDraw5x5(const u8* const draw_addr, const u8* const stc_addr)
{
__asm
    pop     HL                      
    pop     DE                      
    pop     HL                      
    ld      BC, 0x05ff              

    
    ldi
    ldi
    ldi
    ldi
    ldi
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
#if 1
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
#endif
    
    push    HL
#if 1
    ld      HL, 0 + ((VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) - 0x0405)
#else
    ld      HL, 0 + ((VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) - 0x0305)
#endif
    add     HL, DE
    ld      DE, HL
    pop     HL

    
    ldi
    ldi
    ldi
    ldi
    ldi
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
#if 1
    inc     D                       
    ld      A, E
    sub     A, B                    
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
#endif
    
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
__endasm;
}


#if 0 
void vvramDraw5x5Transparent(const u8* const draw_addr, const u8* const stc_addr)
{

__asm




    macro VRAM_DRAW_5x5T
        local   label
        
        ld      A, (HL)
        inc     HL
        or      A
        jr      z, label
            ld      (DE), A
            
            ld      A, E
            add     A, C
            ld      E, A
            
            ld      A, (HL)
            inc     HL
            ld      (DE), A
            
            ld      A, E
            sub     A, C
            ld      E, A
label:
        inc     E
    endm


    macro VRAM_DRAW_5x5T_
        local label
        
        ld      A, (HL)
        inc     HL
        or      A
        jr      z, label
            ld      (DE), A
            
            ld      A, E
            add     A, C
            ld      E, A
            
            ld      A, (HL)
            inc     HL
            ld      (DE), A
label:
    endm


    pop     HL                      
    pop     DE                      
    pop     HL                      

    ld      BC, 0 + (0x0500 | (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)))
VRAM_DRAW_5x5T_VLOOP:
    push    DE
    VRAM_DRAW_5x5T
    VRAM_DRAW_5x5T
    VRAM_DRAW_5x5T
    VRAM_DRAW_5x5T
    VRAM_DRAW_5x5T_
    inc     D
    dec     E
    dec     E
    dec     E
    dec     E
    djnz    B, RAM_DRAW_5x5T_VLOOP

    
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
__endasm;
}
#endif