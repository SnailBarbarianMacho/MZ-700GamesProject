/**
 * アセンブラ マクロ
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef ASM_MACROS_H_INCLUDED
#define ASM_MACROS_H_INCLUDED

static void asm_macros_(void) __naked
{
__asm
    /** 何もしないコードです. 空の引数の代わりに. */
    macro   NO_OP
    endm

    // ---------------------------------------------------------------- 8 bit インクリメント2/デクリメント

    /** 2つの8bit レジスタをインクリメントします */
    macro   INC2    reg1, reg2
        inc     reg1
        inc     reg2
    endm

    /** 2つの8bit レジスタをデクリメントします */
    macro   DEC2    reg1, reg2
        dec     reg1
        dec     reg2
    endm

    // ---------------------------------------------------------------- 8 bit ビット反転・回転・抽出
    /** Aレジスタのビット反転です */
    macro   NOT8A
        cpl
    endm

    /** A レジスタを任意回数左ローテートします
     * - N は回数を, 最後の A は対象レジスタを示します
     * @param n 回数
     * - n <  0 なら右回転します
     * - n == 0 なら何もしません
     * - n >  4 ならば反対回転します
     */
    macro   RLCNA  n
        if      (n) < 0
            if      ((-(n)) & 7) <= 4
                rept    (-(n)) & 7
                    rlca
                endr
            else
                rept    8 - ((-(n)) & 7)
                    rrca
                endr
            endif
        else
            if      ((n) & 7) == 0
            elif    ((n) & 7) <= 4
                rept    (n) & 7
                    rrca
                endr
            else
                rept    8 - ((n) & 7)
                    rlca
                endr
            endif
        endif
    endm

    /** A レジスタを任意回数右ローテートします
     * - 'N' は回数を, 最後の 'A' は対象レジスタを示します
     * @param n 回数
     * - n <  0 なら左回転します
     * - n == 0 なら何もしません
     * - n >  4 ならば反対回転します
     */
    macro   RRCNA   n
        if      (n) < 0
            if      ((-(n)) & 7) <= 4
                rept    (-(n)) & 7
                    rrca
                endr
            else
                rept    8 - ((-(n)) & 7)
                    rlca
                endr
            endif
        else
            if      ((n) & 7) == 0
            elif    ((n) & 7) <= 4
                rept    (n) & 7
                    rlca
                endr
            else
                rept    8 - ((n) & 7)
                    rrca
                endr
            endif
        endif
    endm

    /** A レジスタからパラメータを抽出します
     * - 最後の A は対象レジスタを示します
     * @param mask  マスクかける値
     * @param shift シフト値(右が正, 負の場合は左回転します)
     * @example PEXTA mask, shift
     */
    macro   PEXTA mask, shift
        if      (mask) != 0xff
            and     A,  mask
        endif
        if      (shift) < 0
            if      ((-(shift)) & 7) <= 4
                rept    (-(shift)) & 7
                    rlca
                endr
            else
                rept    8 - ((-(shift)) & 7)
                    rrca
                endr
            endif
        else
            if      ((shift) & 7) == 0
            elif    ((shift) & 7) <= 4
                rept    (shift) & 7
                    rrca
                endr
            else
                rept    8 - ((shift) & 7)
                    rlca
                endr
            endif
        endif
    endm


    // ---------------------------------------------------------------- 8 bit 即値加算

    /** A レジスタに 8 bit 即値を加算します */
    macro   ADDA    val
        if ((val) & 0xff) == 0xff
            dec     A
            exitm
        endif
        if ((val) & 0xff) == 0x00
            exitm
        endif
        if ((val) & 0xff) == 0x01
            inc     A
            exitm
        endif
        add     A,  val
    endm

    /** A レジスタに 8 bit 即値を減算します */
    macro   SUBA    val
        if ((val) & 0xff) == 0xff
            inc     A
            exitm
        endif
        if ((val) & 0xff) == 0x00
            exitm
        endif
        if ((val) & 0xff) == 0x01
            dec     A
            exitm
        endif
        sub     A,  val
    endm

    /** 8 bit レジスタに 8 bit 即値を加算します
     * - A レジスタをテンポラリとして破壊される可能性があります
     * - 処理時間
     *                  need_old_r_to_a の値
     *   d = to - from  !true               true
     *   ---------------------------------------------------
     *    d  == 0        0(T-states)         8
     *   |d| == 1        4                  12
     *   |d| == 2        8                  16
     *   |d| == 3       12                  20
     *   |d| >= 3       15 A レジスタ破壊   22
     * @param r 移動したいレジスタ, (H,L,B,C,D,E)
     *          ※(HL) や A は推奨しません
     * @param val  加算 8bit 値
     * @param A    A レジスタを破壊するので, 引数で明示します
     * @param need_old_r_to_A false/true. true にすると, A に元の r の値を返します
     */
    macro   ADD8T r, val, A, need_old_r_to_A
        if ((val) & 0xff) == 0xfd
            if need_old_r_to_A
                ld      A,  r
            endif
            dec     r
            dec     r
            dec     r
            exitm
        endif
        if ((val) & 0xff) == 0xfe
            if need_old_r_to_A
            endif
            dec     r
            dec     r
            exitm
        endif
        if ((val) & 0xff) == 0xff
            if need_old_r_to_A
                // inc r でいけますよ
                error macro_ADD8T_inc_r
            endif
            dec     r
            exitm
        endif
        if ((val) & 0xff) == 0x00
            if need_old_r_to_A
                // 値は変わらないので不要です
                error macro_ADD8T_need_old_r_to_a_is_not_needed
            endif
            exitm
        endif
        if ((val) & 0xff) == 0x01
            if need_old_r_to_A
                // dec r でいけますよ
                error macro_ADD8T_dec_r
            endif
            inc     r
            exitm
        endif
        if ((val) & 0xff) == 0x02
            if need_old_r_to_A
                ld      A,  r
            endif
            inc     r
            inc     r
            exitm
        endif
        if ((val) & 0xff) == 0x03
            if need_old_r_to_A
                ld      A,  r
            endif
            inc     r
            inc     r
            inc     r
            exitm
        endif
        // ±4 以上の場合
        ld      A,  r
        add     A,  0 + (val)
        ld      r,  A
        if need_old_r_to_A
            sub     A,  0 + (val)
        endif
    endm

    /** 8 bit レジスタに 8 bit 即値を加算します
     * - A レジスタは破壊されません
     * - ±3以上の移動の場合はエラーになります
     * - 処理時間は ADD8T を参照してください
     * @param r   移動したいレジスタ, (H,L,B,C,D,E)
     *            ※(HL) や A は推奨しません
     * @param val 加算 8bit 値
     */
    macro   ADD8 r, val
        if ((val) & 0xff) == 0xfd
            dec     r
            dec     r
            dec     r
            exitm
        endif
        if ((val) & 0xff) == 0xfe
            dec     r
            dec     r
            exitm
        endif
        if ((val) & 0xff) == 0xff
            dec     r
            exitm
        endif
        if ((val) & 0xff) == 0x00
            exitm
        endif
        if ((val) & 0xff) == 0x01
            inc     r
            exitm
        endif
        if ((val) & 0xff) == 0x02
            inc     r
            inc     r
            exitm
        endif
        if ((val) & 0xff) == 0x03
            inc     r
            inc     r
            inc     r
            exitm
        endif
        // ±4 以上の場合はエラー
        error ADD8_value_overflow
    endm


    // ---------------------------------------------------------------- 8/16 bit 演算

    /** A -> 16bit へ符号拡張します
     * - 4 bytes, 16 T-states
     * - A レジスタを破壊するので, 引数で明示します
     */
    macro   EXT16T  dst_h, dst_l, A
        ld      dst_l,  A
        rlca                        // cf = 符号ビット
        sbc     A,  A               // a = 0x00(正)/0xff(負)
        ld      dst_h,  A
    endm

    /** 16bit += A(unsigned)
     * - 5 bytes, 19(繰り上がり有り)/20(同無し) T-states
     * - 次のコード(4 bytes, 21 T-states)より速いです
     *   ld  B, 0
     *   ld  C, A
     *   add HL, BC
     * @param dst_h H, D, B, IXH, IYH など
     * @param dst_l L, E, C, IXL, IYL など
     * @param A     A レジスタを破壊するので, 引数で明示します
     * @note
     * - A が符号付きの場合は, 次のように書きます.
     *   EXT16T D, E, A
     *   add    HL, DE
     */
    macro   ADD16_U8T    dst_h, dst_l, A
        local   ADD16_U8T_100
        add     A,  dst_l
        ld      dst_l,  A
        jr      nc, ADD16_U8T_100
            inc     dst_h
ADD16_U8T_100:
    endm


    // ---------------------------------------------------------------- 16 bit 符号反転, シフト

    /** 16bit 値の符号反転します
     * - 6 bytes, 24 T-states
     * - A レジスタを破壊するので, 引数で明示します.
     * @return
     * - dst_h, dst_l: 反転した 16bit 値
     * - A             dst_h
     * - zf            A == 0 なら '1'
     */
    macro   NEG16T  dst_h, dst_l, src_h, src_l, A
        xor     A,      A
        sub     A,      src_l
        ld      dst_l,  A
        sbc     A,      A
        sub     A,      src_h
        ld      dst_h,  A
    endm

    /** 16bit 算術右シフトします
     * - 下位バイトの演算結果をフラグに反映します
     * @attention
     * - 負値を右シフトし続けると 0xffff になります.
     *   0 にしたければ DIV2_16 を使用してください
     * @return
     * - reg_h, reg_l: シフトした値
     * - cf: 押し出された最下位ビット
     */
    macro   SRA16   reg_h, reg_l
        sra     reg_h
        rr      reg_l
    endm

    /** signed 16bit 値を 2 で割ります
     * - レジスタを二度手間指定するのは仕方が無い。。。
     * @return
     * - reg_hl, reg_h, reg_l: シフトした値
     * - cf: 押し出された最下位ビット
     */
    macro   DIV2_16 reg_hl, reg_h, reg_l
        bit     7,  reg_h
        jr      z,  asmpc + 2 + 1
        inc     reg_hl
        sra     reg_h
        rr      reg_l
    endm

    /** 16bit 論理右シフトします
     * - 下位バイトの演算結果をフラグに反映します
     * @return
     * - reg_h, reg_l: シフトした値
     * - cf: 押し出された最下位ビット
     */
    macro   SRL16   reg_h, reg_l
        srl     reg_h
        rr      reg_l
    endm

    /** 16bit 算術左シフトします
     * - 上位バイトの演算結果をフラグに反映します
     * @return
     * - reg_h, reg_l: シフトした値
     * - cf: 押し出された最上位ビット
     */
    macro   SLA16   reg_h, reg_l
        sla     reg_l
        rl      reg_h
    endm


    // ---------------------------------------------------------------- 16 bit 即値加算

    /** 16 bit レジスタに 16 bit 即値を加算します
     * - 最後の 'T' はテンポラリを意味します
     * - 処理時間
     *    d  == 0           0 T-states
     *   |d| == 1           6 T-states
     *   |d| == 2          12 T-states
     *   |d| == 3          18 T-states
     *   |d| >= 4          21 T-states tmp レジスタ破壊
     * @param HL   移動したいレジスタ (HL固定)
     * @param val  加算値(16bit)
     * @param tmp  テンポラリ レジスタ (BC or DE) 値が破壊されます
     */
    macro   ADD16T  HL, val, tmp
        if (val & 0xffff) == 0xfffd
            dec     HL
            dec     HL
            dec     HL
            exitm
        endif
        if ((val) & 0xffff) == 0xfffe
            dec     HL
            dec     HL
            exitm
        endif
        if ((val) & 0xffff) == 0xffff
            dec     HL
            exitm
        endif
        if ((val) & 0xffff) == 0x0000
            exitm
        endif
        if ((val) & 0xffff) == 0x0001
            inc     HL
            exitm
        endif
        if ((val) & 0xffff) == 0x0002
            inc     HL
            inc     HL
            exitm
        endif
        if ((val) & 0xffff) == 0x0003
            inc     HL
            inc     HL
            inc     HL
            exitm
        endif
        ld      tmp, 0 + (val)
        ADD     HL, tmp
    endm

    /** 16 bit レジスタに 16 bit 即値を加算します
     * - 他のレジスタは破壊されません
     * - ±3以上の移動の場合はエラーになります
     * - 処理時間は ADD16T を参照してください
     * @param HL   移動したいレジスタ (HL固定)
     * @param val  加算値(16bit)
     */
    macro   ADD16 HL, val
        if ((val) & 0xffff) == 0xfffd
            dec     HL
            dec     HL
            dec     HL
            exitm
        endif
        if ((val) & 0xffff) == 0xfffe
            dec     HL
            dec     HL
            exitm
        endif
        if ((val) & 0xffff) == 0xffff
            dec     HL
            exitm
        endif
        if ((val) & 0xffff) == 0x0000
            exitm
        endif
        if ((val) & 0xffff) == 0x0001
            inc     HL
            exitm
        endif
        if ((val) & 0xffff) == 0x0002
            inc     HL
            inc     HL
            exitm
        endif
        if ((val) & 0xffff) == 0x0003
            inc     HL
            inc     HL
            inc     HL
            exitm
        endif
        // ±4 以上の場合はエラー
        error ADD16_value_overflow
    endm


    // ---------------------------------------------------------------- 16 bit ロード

    /** ld DE, (HL) 相当の挙動をします
     * @param HL HL のうち, L が +1 されるので, 明示します
     * @example
     *   LDI16_INC8  D, E, (HL)   // DE = (HL)
     */
    macro   LDI16_INC8  reg_dh, reg_dl, reg_hl
        ld      reg_dl, (HL)
        inc     L
        ld      reg_dh, (HL)
    endm

    /** ld DE, (HL) 相当の挙動をします
     * @param HL HL のうち, HL が +1 されるので, 明示します
     * @example
     *   LDI16_INC16  D, E, (HL)   // DE = (HL)
     */
    macro   LDI16_INC16  reg_dh, reg_dl, reg_hl
        ld      reg_dl, (HL)
        inc     HL
        ld      reg_dh, (HL)
    endm


    // ---------------------------------------------------------------- 16 bit 演算

    /** HL = (DE + BC) / 2  (unsigned 版)
     * - 8+11+8+8 = 35 T-states
     * @param HL    平均
     * @param DE    引数    破壊しませんが明示します
     * @param BC    引数    破壊しませんが明示します
     */
    macro   AVEU16      HL, DE, BC
        ld      HL, DE
        add     HL, BC
        sra     H
        rr      L
    endm

    /** HL = (DE + BC) / 2  (signed 版)
     * - 最短: 8+11+8+12 +8+8 = 55 T-states
     * - 最長: 8+11+8+7+6+8+8 = 56 T-states
     * @param HL    平均
     * @param DE    引数    破壊しませんが明示します
     * @param BC    引数    破壊しませんが明示します
     */
    macro   AVES16
        ld      HL, DE
        add     HL, BC
        bit     7,  H
        jr      z,  asmpc + 2 + 1
        inc     HL
        sra     H
        rr      L
    endm


    // ---------------------------------------------------------------- テーブルジャンプ

    /**
     * テーブル ジャンプを検索して, ジャンプ先アドレスを HL に返します
     * - 58 T-states, 13 bytes
     * @param A   番号. 破壊されるので明示します
     * @param HL  HL が破壊されるので明示します
     * @param table テーブル ジャンプのアドレス
     * @return HL ジャンプ先アドレスを返します
     */
    macro   TAB_JP_HL   A, HL, table
        add     A,  A
        add     A,  table % 256
        ld      L,  A
        adc     A,  table / 256
        sub     A,  L
        ld      H,  A

        ld      A,  (HL)
        inc     HL
        ld      H,  (HL)
        ld      L,  A
    endm

    /**
     * テーブル ジャンプを検索して, ジャンプ先アドレスを DE に返します
     * - 54 T-states, 12 bytes
     */
    macro   TAB_JP_DE   A, HL, table
        add     A,  A
        add     A,  table % 256
        ld      L,  A
        adc     A,  table / 256
        sub     A,  L
        ld      H,  A

        ld      E,  (HL)
        inc     HL
        ld      D,  (HL)
    endm

    /**
     * テーブル ジャンプを検索して, ジャンプ先アドレスを HL に返します
     * - テーブルは 256 バイト境界内になければなりません
     * - 44 T-states, 10 bytes
     */
    macro   TAB_JP_HL_ALIGN256  A, HL, table
        ADD     A,  A
        ld      H,  table / 256
        add     A,  table % 256
        ld      L,  A

        ld      A,  (HL)
        inc     L
        ld      H,  (HL)
        ld      L,  A
    endm

    /**
     * テーブル ジャンプを検索して, ジャンプ先アドレスを DE に返します
     * - テーブルは 256 バイト境界内になければなりません
     * - 40 T-states, 9 bytes
     */
    macro   TAB_JP_DE_ALIGN256  A, HL, table
        ADD     A,  A
        ld      H,  table / 256
        add     A,  table % 256
        ld      L,  A

        ld      E,  (HL)
        inc     L
        ld      D,  (HL)
    endm

__endasm;
}



#endif // ASM_MACROS_H_INCLUDED
