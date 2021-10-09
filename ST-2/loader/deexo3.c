/**
 * Exomizer 3 depacker for SDCC inline asm
 *
 * アセンブラ マクロを覚えるのが面倒なので C プリプロセッサを活用してみた版
 *
 * @author Snail Barbarian Macho (NWK)
 * original code: Copyright (c) 2019-2021 uniabis
 *
 */
#include "../../src-common/common.h"
#include "deexo3.h"

#pragma save
#pragma disable_warning 85
void deexo3(const u8* const src, u8* const dest) __naked
{
__asm
; Exomizer 3 depacker for Z80 sjasmplus
;
; license:zlib license
;
; Copyright (c) 2019-2021 uniabis
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
;
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it and redistribute it
; freely, subject to the following restrictions:
;
;   1. The origin of this software must not be misrepresented; you must not
;   claim that you wrote the original software. If you use this software
;   in a product, an acknowledgment in the product documentation would be
;   appreciated but is not required.
;
;   2. Altered source versions must be plainly marked as such, and must not be
;   misrepresented as being the original software.
;
;   3. This notice may not be removed or altered from any source
;   distribution.
;

// inlining for depacking speed but size of depacker
#define     INLINE_GETBIT	0
#define     INLINE_GETBITS8	1
#define     INLINE_FILBIT	0

#define     PFLAG_BITS_ORDER_BE	    (1<<0)
#define     PFLAG_BITS_COPY_GT_7	(1<<1)
#define    	PFLAG_IMPL_1LITERAL	    (1<<2)
#define    	PFLAG_BITS_ALIGN_START	(1<<3)
#define    	PFLAG_4_OFFSET_TABLES	(1<<4)
#define    	PFLAG_REUSE_OFFSET	    (1<<5)

#ifndef     PFLAG_CODE
// -P0 (Exomizer2 raw compatible)
//#define     PFLAG_CODE 0
// -P7 (Exomizer3.0 raw default)
//#define     PFLAG_CODE (PFLAG_BITS_ORDER_BE | PFLAG_BITS_COPY_GT_7 | PFLAG_IMPL_1LITERAL)
// -P39 (Exomizer3.1 raw default)
#define     PFLAG_CODE (PFLAG_BITS_ORDER_BE | PFLAG_BITS_COPY_GT_7 | PFLAG_IMPL_1LITERAL | PFLAG_REUSE_OFFSET)
#endif

OPCODE_ADD_HL   = 0x29        // add HL, HL
OPCODE_JP_Z     = 0xca        // jp  z, nnnn
OPCODE_JP_C     = 0xda        // jp  c, nnnn

#if (PFLAG_CODE & PFLAG_4_OFFSET_TABLES)
TBL_BYTES       = (16 + 16 + 16 + 16 + 4)
#else
TBL_BYTES       = (16 + 16 + 16 + 4)
#endif

TBL_SHIFT       = (OPCODE_ADD_HL - TBL_BYTES - TBL_BYTES)
TBL_OFS_BITS	= (TBL_SHIFT)
TBL_OFS_LO		= (TBL_SHIFT + TBL_BYTES)
TBL_OFS_HI  	= (TBL_SHIFT + TBL_BYTES + TBL_BYTES)
TBL_SIZE_ALL	= (TBL_BYTES + TBL_BYTES + TBL_BYTES)

#if (PFLAG_CODE & PFLAG_BITS_ORDER_BE)
#define M_GETBIT1()\
    add	a
#define M_FILBIT1()\
    ld	a, (hl)     ;\
    inc	hl          ;\
    rla
#else
#define M_GETBIT1()\
    srl	a
#define M_FILBIT1()\
    ld	a, (hl)     ;\
    inc	hl          ;\
    rra
#endif

// macro M_GETBIT
// [out]
//  CF :bit data
// [affect]
//  af, hl :bit buffer & pointer
#if (INLINE_GETBIT == 1)
#define M_GETBIT(label_bufremain)\
    M_GETBIT1()             ;\
    jr	nz, label_bufremain ;\
    M_FILBIT1()             ;\
                            ;\
label_bufremain:
#else
#define M_GETBIT(label_bufremain)\
    M_GETBIT1()             ;\
    call	z, p_fillbitbuf
#endif

#define	M_GETBITS8I(label_lp1, label_bufremain)\
    ex	af, af' ;'          ;\
                            ;\
label_lp1:                  ;\
    M_GETBIT(label_bufremain);\
    rl	c                   ;\
    djnz	label_lp1       ;\
                            ;\
    ex	af, af'	;'


// macro M_GETBITS8
// [in]
//  b :required bits length(0 < b <= 8)
//  c :must be 0
// [out]
//  b :always 0
//  c :bits data
// [affect]
//  af', hl :bit buffer & pointer
#if (INLINE_GETBITS8 == 1)
#define 	M_GETBITS8(label_lp, label_bufremain)\
    M_GETBITS8I(label_lp, bufremain)
#else
#define 	M_GETBITS8(label_lp, label_bufremain)\
    call	p_getbits8
#endif

// entry point
    pop bc  // return addr
    pop bc  // src
    pop de  // dest
    ld  hl, -6
    add hl, sp
    ld  sp, hl
    ld  hl, bc // src

// [in]
//  hl:source
//  de:dest
//deexo3:
    ld	iy, exo_mapbasebits - TBL_SHIFT

    cp	a	    ; set ZF, reset CF
    ex	af, af' ; '

    ld	bc, #(TBL_BYTES * 256 + 16)


#if (PFLAG_CODE & PFLAG_BITS_ALIGN_START)
    scf
#else
    ;scf		; set CF
    or	a	    ; reset CF(bit data padding in first byte)
#endif

gb4:
    ld	a, (hl)
    inc	hl

init:
get4:
#if (PFLAG_CODE & PFLAG_BITS_ORDER_BE)
    adc	a, a
#else
    rr	a
#endif
    jr	z, gb4
    rl	c
    jr	nc, get4

    ex	af, af' ;'
    ld	a, c

    exx

    ld	hl, 1
    jr	nz, skp1
    ld	d, h
    ld	e, l
    ld	c, 16
skp1:
    ld	(iy + TBL_OFS_BITS), a

#if (PFLAG_CODE & PFLAG_BITS_COPY_GT_7)

    rrca
    jr	nc, skp2
    xor	#0x88
skp2:
#endif
    inc	a

    ld	b, a
    ld	(iy+TBL_OFS_LO), e
    ld	(iy+TBL_OFS_HI), d

setbit:
    djnz	setbit - 1

    add	hl, de
    ex	de, hl

    inc	iy
    dec	c
    ex	af, af' ;'
    exx

    ld	c, #16
    or	a	    ; reset CF
    djnz	init

#if (PFLAG_CODE & PFLAG_REUSE_OFFSET)
    ld	c, b	; BC = 0
#endif

#if (PFLAG_CODE & PFLAG_IMPL_1LITERAL)

  #if (INLINE_FILBIT == 1)
    jr	literal_one
  #endif

#elif (INLINE_FILBIT == 1)
    jr	next
#else
    defb	OPCODE_JP_C
#endif

#if (INLINE_FILBIT == 1)
filbit:
    M_FILBIT1()
    jr	nc, start_copy
#endif

literal_one:

    ldi

next:
#if (INLINE_FILBIT == 1)

    M_GETBIT1
    jr	z, filbit
    jr	c, literal_one
start_copy:

#else

    M_GETBIT(bufremain2)
    jr	c, literal_one

#endif

#if (PFLAG_CODE & PFLAG_REUSE_OFFSET)

    push	bc

#endif

    ld	bc, 0x00ff
alpha:
    inc	c
    M_GETBIT(bufremain3)
    jr	nc, alpha

    ex	af, af'	;'

    ld	a, c
    sub	#16

#if (PFLAG_CODE & PFLAG_REUSE_OFFSET)

    jr	z, reuse_exit

#else

    ret	z

#endif

    jr	nc, literal

    call	p_readtable

#if (PFLAG_CODE & PFLAG_REUSE_OFFSET)

    exx

    pop	bc

    ld	a, b
    or	c

    exx

    jr	nz, new_offset

    ex	af, af'	;'

    M_GETBIT(bufremain4)

    jr	c, reuse_offset_ix

    ex	af, af'	;'

new_offset:

    ld	a, b
    or	a

#endif

    push	bc

    jr	nz, defaultofs
    dec	c
    jr	z, ofs1
    dec	c
    jr	z, ofs2

#if (PFLAG_CODE & PFLAG_4_OFFSET_TABLES)
    dec	c
    jr	z, ofs3

defaultofs:
    ld	c, #0x01
    defb	OPCODE_JP_Z

ofs3:
#else

defaultofs:
    ld	c, #0x01
    defb	OPCODE_JP_Z

ofs2:
#endif

    ld	c, #0x02
ofsb4:
    ld	b, #0x04
getofs:
    M_GETBITS8(lp1, label_bufremain1)

    call	p_readtable

#if (PFLAG_CODE & PFLAG_REUSE_OFFSET)

    push	bc
    pop	ix

reuse_offset_bc:

#endif

    ex	(sp), hl
    push	hl

    ld	h, d
    ld	l, e
    ;or	a	    ; clear CF
    sbc	hl, bc

    pop	bc

    ldir

    pop	hl

    ex	af, af'	;'

#if (PFLAG_CODE & PFLAG_REUSE_OFFSET)

    inc	c	    ; BC=1

#endif

    jp	next

#if (PFLAG_CODE & PFLAG_4_OFFSET_TABLES)

ofs1:
    ld	bc, #0x0210
    jr	getofs

ofs2:
    ld	c, #03h
    jr	ofsb4

#else

ofs1:
    ld	bc, #0x020c
    jr	getofs

#endif

literal:

#if (PFLAG_CODE & PFLAG_BITS_COPY_GT_7)
    ld	b, (hl)
    inc	hl

    ld	c, (hl)
    inc	hl
#else

    ld	c, b
    ld	b, #0x10

    call	p_getbits16_b
#endif

    ldir

    ex	af, af'	;'

#if (PFLAG_CODE & PFLAG_REUSE_OFFSET)

    pop	bc
    dec	bc

#endif

    jp	next

#if (PFLAG_CODE & PFLAG_REUSE_OFFSET)

reuse_exit:
    pop	bc
    ret

reuse_offset_ix:

    ex	af, af'	;'

    push	bc

    push	ix
    pop	    bc

    ;or	a	    ; clear CF

    jp	reuse_offset_bc

#endif

//p_readtable procedure
//[in]
// b :must be 0
// c :tableindex (0 <= c < TBL_BYTES)
//[out]
// bc :bits data
// ZF :set if high byte of output bits data equals zero
//[affect]
// af', hl :bit buffer & pointer
//[work]
// af, iy
p_readtable:
    ld	iy, exo_mapbasebits - TBL_SHIFT
    add	iy, bc

    ld	c, b

#if (PFLAG_CODE & PFLAG_BITS_COPY_GT_7)

    ld	b, (iy+TBL_OFS_BITS)
    srl	b

    jr	z, skp3

    M_GETBITS8(lp2, label_bufremain2)

skp3:
    jr	nc, skp4

    ld	b, c
    ld	c, (hl)
    inc	hl

skp4:

#else

    ld	a, (iy+TBL_OFS_BITS)
    or	a

    call	nz, p_getbits16_a

#endif

    ld	a, (iy+TBL_OFS_LO)
    add	c
    ld	c, a
    ld	a, (iy+TBL_OFS_HI)
    adc	b
    ld	b, a

    ret

#if (PFLAG_CODE & PFLAG_BITS_COPY_GT_7)
#else
// p_getbits16_a procedure
// [in]
//  a :length of bits to read(1-16)
//  c :must be 0
// [out]
//  bc :bits data
// [affect]
//  af', hl :bit buffer & pointer
// [work]
//  af
p_getbits16_a:
    ld	b, a
// p_getbits16_b procedure
// [in]
//  b :length of bits to read(1-16)
//  c :must be 0
// [out]
//  bc :bits data
// [affect]
//  af', hl :bit buffer & pointer
// [work]
//  af
p_getbits16_b:
    ld	a, d
    ld	d, c

    ex	af, af'	;'
lp3:

    M_GETBIT(bufremain5)
    rl	c
    rl	d

    djnz	lp3

    ex	af, af'	;'

    ld	b, d
    ld	d, a

    ret

#endif

#if (INLINE_GETBIT == 1)

#else

p_fillbitbuf:

    M_FILBIT1()

    ret

#endif


#if (INLINE_GETBITS8 == 1)

#else

p_getbits8:
    M_GETBITS8I(lp2, bufremain2)

    ret

#endif

// work
exo_mapbasebits = ADDR_DEEXO_WORK // Makefile で定義
//    defs   TBL_SIZE_ALL
__endasm;
}
#pragma restore