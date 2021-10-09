/**
 * 文字の表示
 * - 名前空間 PRINT_ CHAR_ または print
 *
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "addr.h"
#include "sys.h"
#include "vram.h"
#include "print.h"

// ---------------------------------------------------------------- 変数
static u8  vramPrintAtb = 0x70;
static u8* vramPrintAddr  = (u8*)VVRAM_TEXT_ADDR(0, 0);

// ---------------------------------------------------------------- 設定
void printSetAtb(const u8 col) __z88dk_fastcall
{
    vramPrintAtb = col;
}

void printSetAddr(u8* const addr) __z88dk_fastcall
{
    vramPrintAddr = addr;
}

void printAddAddr(u16 offset) __z88dk_fastcall
{
    vramPrintAddr += offset;
}

// ---------------------------------------------------------------- 表示(文字)
#pragma disable_warning 85
#pragma save
void printPutc(const u8 c) __z88dk_fastcall __naked
{
__asm
    ld      A, L                        // 引数 c
    ld      HL, (#_vramPrintAddr);

    // TEXT
    ld      (HL), A
    inc     L
    ld      (#_vramPrintAddr), HL

    // ATB
    ld      A, #(VRAM_WIDTH + VVRAM_GAPX - 1)
    add     A, L
    ld      L, A

    ld      A, (#_vramPrintAtb)
    ld      (HL),A

    ret
__endasm;
}
#pragma restore

// ---------------------------------------------------------------- 表示(文字列)
#pragma disable_warning 85
#pragma save
void printStringWithLength(const u8* const value, const s16 len) __naked
{
__asm
    // ---------------- HL = 文字列アドレス, DE = VRAM アドレス, BC = 文字数
    pop     HL                          // リターン アドレス(捨てる)
    pop     HL                          // 文字列アドレス
    pop     BC                          // 文字数(s16)
    dec     BC
    ld      A, B
    and     A, A                        // (B - 1) < 0 ならば終了
    jp      m, PRINT_EXIT
    inc     BC
    ld      DE, (#_vramPrintAddr)

    // ---------------- HL' = ATB アドレス, C' = 色, B'DE 未使用
    exx
        ld  HL, (#_vramPrintAddr)
        ld  (VVRAM_TMP_WORK), HL             // 改行の為に現在位置を保存

        ld  A, #(VRAM_WIDTH + VVRAM_GAPX)
        add A, L
        ld  L, A

        ld  A, (#_vramPrintAtb)
        ld  C, A
    exx

    // ---------------- print ループ
PRINT_LOOP:
    ld      A, (HL)

    // -------- null 末端
    and     A
    jp      z, PRINT_END        // ジャンプ することが少ないので, jp でなく, jr を使う

    // -------- 制御コードならばテーブル ジャンプ
    cp      A, CHAR_SP          // 0xf0
    jp      c, PRINT_DISP_CHAR

    add     A, A                // a = e0, e2, ..., fe
    push    HL
    ld      HL, #(PRINT_CTRL_JP_TAB - ((CHAR_SP * 2) & 0xff));
    add     A, L
    ld      L, A
    jr      nc, PRINT_CTRL_CALC_JP_TAB
    inc     H
PRINT_CTRL_CALC_JP_TAB:
    ld      A, (HL)
    inc     HL
    ld      H, (HL)
    ld      L, A
    ld      (PRINT_CTRL_JP + 1), HL // 自己書き換え
    pop     HL
PRINT_CTRL_JP:
    call    0
    jp      PRINT_LOOP_END
PRINT_CTRL_JP_TAB:                  // char.h の制御コードと同じ順番で!
    defw    PRINT_SPACE
    defw    PRINT_MOVE_RIGHT
    defw    PRINT_MOVE_DOWN
    defw    0x0000                  // reserved
    defw    PRINT_COL4
    defw    PRINT_COL5
    defw    PRINT_COL6
    defw    PRINT_COL7
    defw    PRINT_CAPS
    defw    PRINT_ATB
    defw    PRINT_CRLF
    defw    PRINT_CRLF2
    // -------- スペース
PRINT_SPACE:
    exx
        inc L       // HL'++
    exx
    inc     E       // DE++
    ret
    // -------- カーソル右へ
PRINT_MOVE_RIGHT:
    inc     HL
    ld      A, (HL)
    exx
        ld  B, A    // A 保存
        add A, L    // HL' += A
        ld  L, A
        ld  A, B    // A 復帰
    exx
    add     A, E    // DE += A
    ld      E, A
    ret
    // -------- カーソル下へ
PRINT_MOVE_DOWN:
    inc     HL
    ld      A, (HL)
    exx
        ld  B, A    // A 保存
        add A, H    // HL += A * 0x100
        ld  H, A
        ld  A, B    // A 復帰
    exx
    add     A, D    // DE += A * 0x100
    ld      D, A
    ret
    // -------- ATB 0x40～0x70
PRINT_COL4:
    exx
        ld  C, 0x40
    exx
    ret
PRINT_COL5:
    exx
        ld  C, 0x50
    exx
    ret
PRINT_COL6:
    exx
        ld  C, 0x60
    exx
    ret
PRINT_COL7:
    exx
        ld  C, 0x70
    exx
    ret
    // -------- 大文字 / 小文字 切替
PRINT_CAPS:
    exx
        ld  A, 0x80
        xor C
        ld  C, A
    exx
    ret
    // -------- ATB 変更
PRINT_ATB:
    inc     HL
    ld      A, (HL)
    exx
        ld  C, A
    exx
    ret
    // -------- 改行
PRINT_CRLF2:
    ld      DE, (VVRAM_TMP_WORK)
    inc     D
    jp      PRINT_CRLF_COMMON
PRINT_CRLF:
    ld      DE, (VVRAM_TMP_WORK)
PRINT_CRLF_COMMON:
    inc     D
    ld      (VVRAM_TMP_WORK), DE
    exx
        ld  HL, (VVRAM_TMP_WORK)
        ld  DE,  VRAM_WIDTH + VVRAM_GAPX
        add HL, DE
    exx
    ret


    // -------- 文字表示
PRINT_DISP_CHAR:
    ld      (DE), A // text
    inc     E
    exx
        ld  (HL), C // atb
        inc L
    exx

    // 文字数減算
    dec     BC
    ld      A, C
    or      B
    jp      z, PRINT_END

PRINT_LOOP_END:
    inc     HL
    jp      PRINT_LOOP

PRINT_END:
    ld      (#_vramPrintAddr), DE   // 表示位置の更新
    exx
        ld  A, C
        and A, 0x77                 // 小文字ひらがなは解除
        ld  (#_vramPrintAtb), A
    //exx                           // 不要

    // スタックポインタを戻して ret
PRINT_EXIT:
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
    ret
__endasm;
}
#pragma restore

// ---------------------------------------------------------------- 表示(8 bit 数値)
#pragma disable_warning 85
#pragma save
void printU8Right(const u8 value) __z88dk_fastcall __naked
{
__asm
    call    INIT_VRAM_AND_CALC_BCD8
    // ---------------- 表示
    // D = 表示許可フラグに使います(=0)
    // -------- 100 の桁の表示
    jp      z, PRINT_U8R_100 // 0 ならば表示しない
        add     A, C        // C = CHAR_0
        ld      (HL), A
        exx
            ld  (HL), C
        exx
        inc     D           // 表示許可フラグ = 1
PRINT_U8R_100:
    inc     L
    exx
        inc L
    exx
    // -------- 10 の桁の表示
    ld      A, B
    rrca
    rrca
    rrca
    rrca
    and     E           // E = 0x0f
    add     A, D        // D = 表示許可フラグ
    jp      z, PRINT_U8R_10 // if (A == 0 || D == 0) ならば表示しない
        sub A, D
        add A, C        // C = CHAR_0
        ld  (HL), A
        exx
            ld  (HL), C
        exx
PRINT_U8R_10:
    inc     L
    exx
        inc L
    exx
    // -------- 1 の桁の表示. 必ず表示
    ld      A, B
    and     E           // E = 0x0f
    add     A, C        // C = CHAR_0
    ld      (HL), A
    inc     L
    exx
        ld  (HL), C
    exx
    ld      (#_vramPrintAddr), HL   // 表示位置の更新

    ret
__endasm;

__asm
INIT_VRAM_AND_CALC_BCD8:
    // 表示アドレス関連の初期化 + 8 bit 値を BCD に変換します
    //
    // ---- 考え方 ----
    // 4 bit 単位(ニブル)に分割.
    // 最上位ニブルが 0x00～0x0f になるように右シフトして DAA.
    // 右シフトした回数分だけ,「左シフト → DAA」を繰り返す
    // 4 回「左シフト → DAA」毎に, 「次のニブルを DAA したもの」を加算して DAA する
    //
    // 例: 0xfe
    //     ニブル 1
    //       0x0f
    // daa   0x15
    // x2    0x2a
    // daa   0x30
    // x2    0x60
    // daa   0x60
    // x2    0xc0
    // daa  0x120      ニブル 2
    // x2   0x240        0x0f
    // daa  0x240   daa  0x15
    // +    0x255──────────┘
    // daa  0x255
    //
    // in:    L   ... 8bit 値
    // out:   AB  ... BCD 値(3桁)
    //        D   ... 0x00
    //        E   ... 0x0f
    //        HL  ... VRAM アドレス
    //        C   ... CHAR_0
    //        HL' ... ATB アドレス
    //        C'  ... ATB
    //        z   ... 100 の位が 0 ならば '1'
    // break: BC
    // ---------------- HL' = ATB アドレス, C' = ATB
    exx
        ld  HL, (#_vramPrintAddr)
        ld  A, VRAM_WIDTH + VVRAM_GAPX
        add A, L
        ld  L, A
        ld  A, (#_vramPrintAtb)
        ld  C, A
    exx

    ld      DE, 0x000f // D = 0, E = 0x0f, C = 100 の位
    // ---------------- 10 進変換 ニブル 1
    ld      A, L
    and     0xf0
    rrca
    rrca
    rrca
    rrca            // c = 0, h = 0           0x0f
    daa             //                        0x15
    // -------- x2
    add     A, A    // x2                     0x2a
    daa             //                        0x30
    // -------- x4
    add     A, A    // x2                     0x60
    daa             //                        0x60
    // -------- x8
    // 1, 10 の位 (CB の B)         B *= 2
    add     A, A    // x2                     0xc0
    daa             //                       0x120
    ld      B, A    // B に退避
    // 100 の位の (CB の C)         C = c
    sbc     A, A    // A = 0 or -1
    neg             // A = 0 or  1
    ld      C, A
    // -------- x16
    // 1, 10 の位 (CB の B)         B = B * 2 + c
    ld      A, B
    add     A, B    // x2                    0x240
    daa             //                       0x240
    ld      B, A
    // 100 の位の (CB の C)         C = C * 2 + c
    ld      A, C
    adc     A, C    // daa は不要
    ld      C, A

    // ---------------- 10 進変換 ニブル 2
    // 1, 10 の位 (CB の B)         C += L & 0x0f
    ld      A, L
    and     E       // E = 0x0f  c = 0, h = 1           0x0f
    add     A, D    // D = 0     c = 0, h = 0
    daa             //                                  0x0f
    add     A, B    //                     + 0x255 ──────┘
    daa             //                       0x255
    ld      B, A
    // 100 の位の (CB の C)         C = C * 2 + c
    ld      A, D    // D = 0
    adc     A, C    // ゼロならば z = 1

    // ---------------- HL = VRAM アドレス, C =CHAR_0
    ld      HL, (#_vramPrintAddr)
    ld      C, CHAR_0
    ret
__endasm;
}
#pragma restore


#pragma disable_warning 85
#pragma save
void printU8Left(const u8 value) __z88dk_fastcall __naked
{
__asm
    call    INIT_VRAM_AND_CALC_BCD8
    // ---------------- 表示
    // D = 表示許可フラグに使います(=0)
    // -------- 100 の桁の表示
    jp      z, PRINT_U8L_100 // 0 ならば表示しない
        add     A, C        // C = CHAR_0
        ld      (HL), A
        inc     L
        exx
            ld  (HL), C
            inc L
        exx
    inc     D           // 表示許可フラグ = 1
PRINT_U8L_100:
    // -------- 10 の桁の表示
    ld      A, B
    rrca
    rrca
    rrca
    rrca
    and     E           // E = 0x0f
    add     A, D        // D = 表示許可フラグ
    jp      z, PRINT_U8L_10 // if (A == 0 || D == 0) ならば表示しない
        sub     A, D
        add     A, C        // C = CHAR_0
        ld      (HL), A
        inc     L
        exx
            ld  (HL), C
            inc L
        exx
PRINT_U8L_10:
    // -------- 10 の桁の表示
    ld      A, B
    and     E           // E = 0x0f
    add     A, C        // C = CHAR_0
    ld      (HL), A
    inc     L
    exx
        ld  (HL), C
    exx

    ld      (#_vramPrintAddr), HL   // 表示位置の更新

    ret
__endasm;
}
#pragma restore

// ---------------------------------------------------------------- 表示(16 bit 数値)
#pragma disable_warning 85
#pragma save
void printU16Right(const u16 value) __z88dk_fastcall __naked
{
__asm
    call    INIT_VRAM_AND_CALC_BCD16
    // ---------------- 表示
    // -------- 10000 の桁の表示(ACD の A)
    jp      z, PRINT_U16R_10000 // 0 ならば表示しない
        add A, CHAR_0
        ld  (HL), A
        exx
            ld  (HL), C
        exx
        inc B           // 表示許可フラグ = 1
PRINT_U16R_10000:
    inc     L
    exx
        inc L
    exx
    // -------- 1000 の桁の表示(ACD の C)
    ld      A, C
    rrca
    rrca
    rrca
    rrca
    and     E           // E = 0x0f
    add     A, B        // B = 表示許可フラグ
    jp      z, PRINT_U16R_1000 // if (A == 0 || B == 0) ならば表示しない
        sub A, B
        add A, CHAR_0
        ld  (HL), A
        exx
            ld  (HL), C
        exx
        inc B           // 表示許可フラグ != 0
PRINT_U16R_1000:
    inc     L
    exx
        inc L
    exx
    // -------- 100 の桁の表示(ACD の C)
    ld      A, C
    and     E           // E = 0x0f
    add     A, B        // B = 表示許可フラグ
    jp      z, PRINT_U16R_100 // if (A == 0 || B == 0) ならば表示しない
        sub A, B
        add A, CHAR_0
        ld  (HL), A
        exx
            ld  (HL), C
        exx
        inc B           // 表示許可フラグ != 0
PRINT_U16R_100:
    inc     L
    exx
        inc L
    exx
    // -------- 10 の桁の表示(ACD の D)
    ld      A, D
    rrca
    rrca
    rrca
    rrca
    and     E           // E = 0x0f
    add     A, B        // B = 表示許可フラグ
    jp      z, PRINT_U16R_10 // if (A == 0 || B == 0) ならば表示しない
        sub A, B
        add A, CHAR_0
        ld  (HL), A
        exx
            ld  (HL), C
        exx
PRINT_U16R_10:
    inc     L
    exx
        inc L
    exx
    // -------- 1 の桁の表示(ACD の D)
    ld      A, D
    and     E           // E = 0x0f
    add     A, CHAR_0
    ld      (HL), A
    inc     L
    exx
        ld  (HL), C
    exx

    ld      (#_vramPrintAddr), HL   // 表示位置の更新

    ret
__endasm;
__asm
INIT_VRAM_AND_CALC_BCD16:
    // 表示アドレス関連の初期化 + 16 bit 値を BCD に変換します
    //
    // 例: 0xffff
    //     ニブル 1
    //         0x0f
    // daa     0x15
    // x2      0x2a
    // daa     0x30
    // x2      0x60
    // daa     0x60
    // x2      0xc0
    // daa    0x120       ニブル 2
    // x2     0x240        0x0f
    // daa    0x240   daa  0x15
    // +      0x255──────────┘
    // daa    0x255
    // x2     0x4AA
    // daa    0x510
    // x2     0xA20
    // daa   0x1020
    // x2    0x2040
    // daa   0x2040       ニブル 2
    // x2    0x4080        0x0f
    // daa   0x4080   daa  0x15
    // +     0x4095──────────┘
    // daa   0x4095
    // x2    0x812a
    // daa   0x8190
    // x2   0x10320
    // daa  0x16380
    // x2   0x2c700
    // daa  0x32760       ニブル 3
    // x2   0x64ec0        0x0f
    // daa  0x65520   daa  0x15
    // +    0x65535────────┘
    // daa  0x65535
    //
    // in:    HL  ... 8bit 値
    // out:   ACD ... BCD 値(5桁)
    //        B   ... 0x00
    //        E   ... 0x0f
    //        HL  ... VRAM アドレス
    //        HL' ... ATB アドレス
    //        C'  ... ATB
    //        z   ... 100 の位が 0 ならば '1'

    // -------- HL' = ATB アドレス, C' = 色
    exx
        ld  HL, (#_vramPrintAddr)
        ld  A,  VRAM_WIDTH + VVRAM_GAPX
        add A, L
        ld  L, A
        ld  A, (#_vramPrintAtb)
        ld  C, A
    exx

    // 5桁の BCD 値は, ACD レジスタに保存します
    ld      BC, 0x0000
    // ---------------- 10 進変換 ニブル 1
    ld      A, H
    and     0xf0
    rrca
    rrca
    rrca
    rrca            // c = 0, h = 0           0x0f
    daa             //                        0x15
    // -------- x2
    add     A, A    // x2                     0x2a
    daa             //                        0x30
    // -------- x4
    add     A, A    // x2                     0x60
    daa             //                        0x60
    // -------- x8
    // 1, 10 の位 (BCD の D)        D *= 2
    add     A, A    // x2                     0xc0
    daa             //                       0x120
    ld      D, A    // BCD の D
    // 100, 1000 の位 (BCD の C)    C = c
    ld      A, C    // C = 0
    adc     A, C
    ld      C, A
    // -------- x16
    // 1, 10 の位 (BCD の D)        D *= 2
    ld      A, D
    add     A, D    // x2                    0x240
    daa             //                       0x240
    ld      D, A
    // 100, 1000 の位 (BCD の C)    C = C * 2 + c
    ld      A, C
    adc     A, C    // まだ daa は不要
    ld      C, A

    // ---------------- 10 進変換 ニブル 2
    // 1, 10 の位 (BCD の D)        D += H & 0x0f
    ld      A, H    //  以下 H 使用可能
    ld      H, 0
    and     0x0f    //                                  0x0f
    add     A, 0    //  c = 0, h = 0
    daa             //                                  0x0f
    add     A, D    // BCD の D            + 0x255 ──────┘
    daa             //                       0x255
    ld      D, A
    // 100, 1000 の位 (BCD の C)    C += c
    ld      A, H    // H = 0
    adc     A, C    // まだ daa は必要
    ld      C, A
    // -------- x32
    // 1, 10 の位 (BCD の D)        D *= 2
    ld      A, D
    add     A, D    // x2                    0x4aa
    daa             //                       0x510
    ld      D, A
    // 100, 1000 の位 (BCD の C)    C = C * 2 + c
    ld      A, C
    adc     A, C    // まだ daa は不要
    ld      C, A
    // -------- x64
    // 1, 10 の位 (BCD の D)        D *= 2
    ld      A, D
    add     A, D    // x2                    0xa20
    daa             //                       0x1020
    ld      D, A
    // 100, 1000 の位 (BCD の C)    C = C * 2 + c
    ld      A, C
    adc     A, C
    daa             // ここから daa は必要
    ld      C, A
    // -------- x128
    // 1, 10 の位 (BCD の D)        D *= 2
    ld      A, D
    add     A, D    // x2                    0x2040
    daa             //                       0x2040
    ld      D, A
    // 100, 1000 の位 (BCD の C)    C = C * 2 + c
    ld      A, C
    adc     A, C
    daa
    ld      C, A
    // -------- x256
    // 1, 10 の位 (BCD の D)        D *= 2
    ld      A, D
    add     A, D    // x2                    0x4080
    daa             //                       0x4080
    ld      D, A
    // 100, 1000 の位 (BCD の C)    C = C * 2 + c
    ld      A, C
    adc     A, C
    daa
    ld      C, A
    // ---------------- 10 進変換 ニブル 3
    // 1, 10 の位 (BCD の D)        D += L >> 4
    ld      A, L
    and     0xf0
    rrca
    rrca
    rrca
    rrca            // c = 0, h = 0                     0x0f
    daa             //                                  0x15
    add     A, D    //                     + 0x4095───────┘
    daa             //                       0x4095
    ld      D, A
    // 100, 1000 の位 (BCD の C)    C += c
    ld      A, H    // H = 0
    adc     A, C
    daa
    ld      C, A
    // -------- x512
    // 1, 10 の位 (BCD の D)        D *= 2
    ld      A, D
    add     A, D    // x2                    0x812a
    daa             //                       0x8190
    ld      D, A
    // 100, 1000 の位 (BCD の C)    C = C * 2 + c
    ld      A, C
    adc     A, C
    daa
    ld      C, A
    // -------- x1024
    // 1, 10 の位 (BCD の D)        D *= 2
    ld      A, D
    add     A, D    // x2                   0x10320
    daa             //                      0x16380
    ld      D, A
    // 100, 1000 の位 (BCD の C)    C = C * 2 + c
    ld      A, C
    adc     C
    daa
    ld      C, A
    // 10000 の位 (BCD の B)        B = c
    ld      A, B
    adc     A, B
    ld      B, A
    // -------- x2048
    // 1, 10 の位 (BCD の D)        D *= 2
    ld      A, D
    add     A, D    // x2                   0x2c700
    daa             //                      0x32760
    ld      D, A
    // 100, 1000 の位 (BCD の C)    C = C * 2 + c
    ld      A, C
    adc     A, C
    daa
    ld      C, A
    // 10000 の位 (BCD の B)        B = B * 2 + c
    ld      A, B
    adc     A, B     // daa は不要
    ld      B, A
    // -------- x4096
    // 1, 10 の位 (BCD の D)        D *= 2
    ld      A, D
    add     A, D    // x2                   0x64ec0
    daa             //                      0x65520
    ld      D, A
    // 100, 1000 の位 (BCD の C)    C = C * 2 + c
    ld      A, C
    adc     A, C
    daa
    ld      C, A
    // 10000 の位 (BCD の B)        B = B * 2 + c
    ld      A, B
    adc     A, B    // daa は不要
    ld      B, A
    // ---------------- 10 進変換 ニブル 4
    // 1, 10 の位 (BCD の D)        D += B & 0x0f
    ld      A, L
    and     0x0f
    add     A, H    // H = 0   c = 0, h = 0             0x0f
    daa             //                                  0x15
    add     A, D    //                    + 0x65535───────┘
    daa             //                      0x65535
    ld      D, A
    // 100, 1000 の位 (BCD の C)    C += c
    ld      A, H    // H = 0
    adc     A, C
    daa
    ld      C, A
    // 10000 の位 (BCD の B)        B += c
    ld      A, H    // H = 0
    adc     A, B
    daa             // 0 なら z = 1

    // ---------------- HL = VRAM アドレス, B = 0, E = 0x0f
    ld      B, H    // H = 0 表示許可フラグ
    ld      HL, (#_vramPrintAddr)
    ld      E, 0x0f
    ret
__endasm;
}
#pragma restore

#pragma disable_warning 85
#pragma save
void printU16Left(const u16 value) __z88dk_fastcall __naked
{
__asm
    call    INIT_VRAM_AND_CALC_BCD16
    // ---------------- 表示
    // -------- 10000 の桁の表示(ACD の A)
    jp      z, PRINT_U16L_10000 // 0 ならば表示しない
        add A, CHAR_0
        ld  (HL), A
        inc L
        exx
            ld  (HL), C
            inc L
        exx
        inc B           // 表示許可フラグ = 1
PRINT_U16L_10000:
    // -------- 1000 の桁の表示(BCD の C)
    ld      A, C
    rrca
    rrca
    rrca
    rrca
    and     E           // E = 0x0f
    add     A, B        // B = 表示許可フラグ
    jp      z, PRINT_U16L_1000 // if (A == 0 || B == 0) ならば表示しない
        sub A, B
        add A, CHAR_0
        ld  (HL), A
        inc L
        exx
            ld  (HL), C
            inc L
        exx
        inc B           // 表示許可フラグ != 0
PRINT_U16L_1000:
    // -------- 100 の桁の表示(BCD の C)
    ld      A, C
    and     E           // E = 0x0f
    add     A, B        // B = 表示許可フラグ
    jp      z, PRINT_U16L_100 // if (A == 0 || B == 0) ならば表示しない
        sub A, B
        add A, CHAR_0
        ld  (HL), A
        inc L
        exx
            ld  (HL), C
            inc L
        exx
        inc B           // 表示許可フラグ != 0
PRINT_U16L_100:
    // -------- 10 の桁の表示(BCD の D)
    ld      A, D
    rrca
    rrca
    rrca
    rrca
    and     E           // E = 0x0f
    add     A, B        // B = 表示許可フラグ
    jp      z, PRINT_U16L_10 // if (A == 0 || B == 0) ならば表示しない
        sub A, B
        add A, CHAR_0
        ld  (HL), A
        inc L
        exx
            ld  (HL), C
            inc L
        exx
PRINT_U16L_10:
    // -------- 1 の桁の表示(BCD の D)
    ld      A, D
    and     E           // E = 0x0f
    add     A, CHAR_0
    ld      (HL), A
    inc     L
    exx
        ld  (HL), C
    exx

    ld      (#_vramPrintAddr), HL   // 表示位置の更新
    ret
__endasm;
}
#pragma restore

static const u8 printHexTab[] = {
    CHAR_0, CHAR_1, CHAR_2, CHAR_3,
    CHAR_4, CHAR_5, CHAR_6, CHAR_7,
    CHAR_8, CHAR_9, CHAR_A, CHAR_B,
    CHAR_C, CHAR_D, CHAR_E, CHAR_F,
};

#ifndef RELEASE
#pragma disable_warning 85
#pragma save
void printHex16(const u16 value) __z88dk_fastcall __naked
{
__asm
    // -------- HL' = ATB アドレス, C' = 色
    ld      BC, (#_vramPrintAddr)
    exx
        ld  HL, (#_vramPrintAddr)
        ld  BC, VRAM_WIDTH + VVRAM_GAPX
        add HL, BC
        ld  A, (#_vramPrintAtb)
        ld  C, A
    exx
    // ---------------- 1000 の桁
    ld      A, H
    rrca
    rrca
    rrca
    rrca
    and     0x0f
    call    PRINT_HEX_1
    // ---------------- 100 の桁
    ld      A, H
    and     0x0f
    call    PRINT_HEX_1
    // ---------------- 10 の桁
    ld      A, L
    rrca
    rrca
    rrca
    rrca
    and     0x0f
    call    PRINT_HEX_1
    // ---------------- 1 の桁
    ld      A, L
    and     0x0f
    call    PRINT_HEX_1

    ld      (#_vramPrintAddr), BC   // 表示位置の更新
    ret

    // ---------------- 1 文字表示
    // A:   値
    // BC:  VRAM アドレス
    // HL': ATB アドレス
    // C':  表示色
    // 破壊: A, DE
PRINT_HEX_1:
    push    HL
    ld      HL, #_printHexTab
    ld      D, 0
    ld      E, A
    add     HL, DE
    ld      A, (HL)
    ld      (BC),A
    inc     C
    exx
        ld  (HL), C
        inc L
    exx
    pop     HL
    ret

__endasm;
}
#pragma restore
#endif

// ---------------------------------------------------------------- 表示ユーティリティ
void printReady()
{
    if (sysIsGameMode()) {
        static const u8 str[] = { CHAR_R, CHAR_E, CHAR_A, CHAR_D, CHAR_Y, CHAR_EXCLAMATION, 0 };
        printSetAddr((u8*)VVRAM_TEXT_ADDR(17, 10));
        printString(str);
    }
}
