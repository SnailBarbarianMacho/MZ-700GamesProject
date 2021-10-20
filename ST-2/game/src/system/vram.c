/**
 * 仮想 VRAM 管理, 文字や画像表示
 *
 * @author Snail Barbarian Macho (NWK)
 */
#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "addr.h"
#include "vram.h"

// ---------------------------------------------------------------- 変数
static u8 sbVramTransEnabled;
#if DEBUG
static u16 s8253Ch1Ct; // 処理時間計測に使う
#endif

// ---------------------------------------------------------------- マクロ
#define _8253CH1CT (262 * 4)   // 8253のカウンタ. 1カウンタ1ライン. 262ラインで1フレーム

// 一時的に使うワークエリア
#define ADDR_TMP_SP    (VVRAM_TMP_WORK + 16) // 臨時スタックポインタ

// ---------------------------------------------------------------- システム
#define POP_10_BYTES_TO_REGISTERS() \
    pop     HL  \
    pop     DE  \
    pop     BC  \
    exx         \
    pop     DE  \
    pop     BC
#define PUSH_10_BYTES_FROM_REGISTERS() \
    push    BC  \
    push    DE  \
    exx         \
    push    BC  \
    push    DE  \
    push    HL
#define PUSH_DE_40_BYTES() \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE \
    push    DE

void vramInit() __z88dk_fastcall __naked
{
__asm;
    ld      A, 1
    ld      (_sbVramTransEnabled), A
#if DEBUG
    ld      H, A
    ld      L, A
    ld      (_s8253Ch1Ct), HL
#endif
    jp      _vramClear // 仮想画面クリアして終了
__endasm;
}

void vramTrans() __z88dk_fastcall __naked
{
__asm
    // ---------------- VRAM 転送許可
    ld      A, (_sbVramTransEnabled)
    and     A
    ld      A, 1
    ld      (_sbVramTransEnabled), A// 次回は転送許可に
    ret     z

    // ---------------- 準備
    ld      (VRAM_TRANS_SP_RESTORE + 1), SP// SP 保存(自己書換)
    BANK_VRAM_IO                        // バンク切替
__endasm;

__asm
    // ---------------- デバッグ用タイマ
#if DEBUG
    ld     HL, #MIO_8253_CH1

    ld      C, (HL)
    ld      B, (HL)
    ld      (_s8253Ch1Ct), BC  // デバッグ時はポーリング前のタイマ値を保存します

    // カウンタの再セット
    // 8253 チャンネル 1 のクロックは, 水平周期と同じ.
    // MZ-700は 1/60 で 262 ラインなので, 1/20 秒にしたければ 262 * 3 にする
    ld     BC, #_8253CH1CT
    ld     (HL), C
    ld     (HL), B
#endif
__endasm;

__asm
    // -------- アドレス初期化
    ld      HL, #VVRAM_TEXT_ADDR(0, 0)
    ld      (VRAM_TRANS_SRC_0 + 1), HL  // 転送元(自己書換)
    ld      HL, #VRAM_TEXT_ADDR(10, 0)
    ld      (VRAM_TRANS_DST_0 + 1), HL  // 転送先(自己書換)

    // ----------------
    // V-Blank を待ちます. 既に V-Blank 中ならばそのまま行っちゃいます
    // 8253 ポート C bit 7 == 0 ならば, ブランキング中

    // VRAM 転送速度と画面リフレッシュ速度は同じなので,
    // 画面の乱れが少ないなら, 同期を待たなくてもいいようです
#if 0
    ld      HL, #MIO_8255_PORTC
VRAM_BLANK_0:
    ld      A, (HL)
    rlca
    jp      c, VRAM_BLANK_0             // V表示中なら待つ
#endif

    // ----------------
    // 転送!
    // 10 バイト転送 x 4 で 1行分転送します
    // ---- loop
    ld      A, VRAM_HEIGHT              // ループ カウンタ
VRAM_TRANS_LOOP:

    // ---- TEXT 1 番目の 10 bytes
VRAM_TRANS_SRC_0:
    ld      SP, #0x0000                 // 転送元
    POP_10_BYTES_TO_REGISTERS()
    ld      (VRAM_TRANS_SRC_1 + 1), SP  // 転送元(自己書換)
VRAM_TRANS_DST_0:
    ld      SP, #0x0000                 // 転送先
    PUSH_10_BYTES_FROM_REGISTERS()
    ld      HL, #20                     // 転送先アドレス移動
    add     HL, SP
    ld      (VRAM_TRANS_DST_1 + 1), HL  // 転送先(自己書換)

    // ---- TEXT 2 番目の 10 bytes
VRAM_TRANS_SRC_1:
    ld      SP, #0x0000                 // 転送元
    POP_10_BYTES_TO_REGISTERS()
    ld      (VRAM_TRANS_SRC_2 + 1), SP  // 転送元(自己書換)
VRAM_TRANS_DST_1:
    ld      SP, #0x0000                 // 転送先
    PUSH_10_BYTES_FROM_REGISTERS()
    ld      HL, #20                     // 転送先アドレス移動
    add     HL, SP
    ld      (VRAM_TRANS_DST_2 + 1), HL  // 転送先(自己書換)

    // ---- TEXT 3 番目の 10 bytes
VRAM_TRANS_SRC_2:
    ld      SP, #0x0000                 // 転送元
    POP_10_BYTES_TO_REGISTERS()
    ld      (VRAM_TRANS_SRC_3 + 1), SP  // 転送元(自己書換)
VRAM_TRANS_DST_2:
    ld      SP, #0x0000                 // 転送先
    PUSH_10_BYTES_FROM_REGISTERS()
    ld      HL, #20                     // 転送先アドレス移動
    add     HL, SP
    ld      (VRAM_TRANS_DST_3 + 1), HL  // 転送先(自己書換)

    // ---- TEXT 4 番目の 10 bytes
VRAM_TRANS_SRC_3:
    ld      SP, #0x0000                 // 転送元
    POP_10_BYTES_TO_REGISTERS()
    ld      HL, VVRAM_GAPX              // 転送元アドレスを, 仮想 ATB に移動
    add     HL, SP
    ld      (VRAM_TRANS_SRC_4 + 1), HL  // 転送元(自己書換)
VRAM_TRANS_DST_3:
    ld      SP, #0x0000                 // 転送先
    PUSH_10_BYTES_FROM_REGISTERS()
    ld      HL, #(0x0800 - 20)          // 転送先アドレスを, ATB に移動します
    add     HL, SP
    ld      (VRAM_TRANS_DST_4 + 1), HL  // 転送先(自己書換)

    // ---- ATB 1 番目の 10 bytes
VRAM_TRANS_SRC_4:
    ld      SP, #0x0000                 // 転送元
    POP_10_BYTES_TO_REGISTERS()
    ld      (VRAM_TRANS_SRC_5 + 1), SP  // 転送元(自己書換)
VRAM_TRANS_DST_4:
    ld      SP, #0x0000                 // 転送先
    PUSH_10_BYTES_FROM_REGISTERS()
    ld      HL, #20                     // 転送先アドレス移動
    add     HL, SP
    ld      (VRAM_TRANS_DST_5 + 1), HL  // 転送先(自己書換)

    // ---- ATB 2 番目の 10 bytes
VRAM_TRANS_SRC_5:
    ld      SP, #0x0000                 // 転送元
    POP_10_BYTES_TO_REGISTERS()
    ld      (VRAM_TRANS_SRC_6 + 1), SP  // 転送元(自己書換)
VRAM_TRANS_DST_5:
    ld      SP, #0x0000                 // 転送先
    PUSH_10_BYTES_FROM_REGISTERS()
    ld      HL, #20                     // 転送先アドレス移動
    add     HL, SP
    ld      (VRAM_TRANS_DST_6 + 1), HL  // 転送先(自己書換)

    // ---- ATB 3 番目の 10 bytes
VRAM_TRANS_SRC_6:
    ld      SP, #0x0000                 // 転送元
    POP_10_BYTES_TO_REGISTERS()
    ld      (VRAM_TRANS_SRC_7 + 1), SP  // 転送元(自己書換)
VRAM_TRANS_DST_6:
    ld      SP, #0x0000                 // 転送先
    PUSH_10_BYTES_FROM_REGISTERS()
    ld      HL, #20                     // 転送先アドレス移動
    add     HL, SP
    ld      (VRAM_TRANS_DST_7 + 1), HL  // 転送先(自己書換)

    // ---- ATB 4 番目の 10 bytes
VRAM_TRANS_SRC_7:
    ld      SP, #0x0000                 // 転送元
    POP_10_BYTES_TO_REGISTERS()
    ld      HL, VVRAM_WIDTH - VRAM_WIDTH - VVRAM_GAPX - VRAM_WIDTH  // 転送元アドレスを, TEXT の次の行に移動
    add     HL, SP
    ld      (VRAM_TRANS_SRC_0 + 1), HL  // 転送元(自己書換)
VRAM_TRANS_DST_7:
    ld      SP, #0x0000                 // 転送先
    PUSH_10_BYTES_FROM_REGISTERS()
    ld      HL, #(-0x0800 + VRAM_WIDTH - 20) // 転送先アドレスを TEXT の次の行に移動します
    add     HL, SP
    ld      (VRAM_TRANS_DST_0 + 1), HL  // 転送先(自己書換)

    // ---- loop end
    dec     A
    jp      nz, VRAM_TRANS_LOOP
__endasm;

__asm
    // ---------------- 終了
    BANK_RAM                        // バンク切替
VRAM_TRANS_SP_RESTORE:
    ld      SP, #0x0000             // SP 復活

    jp      _vramClear // 仮想画面クリアして終了
__endasm;
}


void vramSetTransDisabled()__z88dk_fastcall
{
    sbVramTransEnabled = false;
}


#if DEBUG
void vramSyncVBlank()__z88dk_fastcall __naked
{
__asm
    BANK_VRAM_IO                    // バンク切替

    ld      HL, #MIO_8255_PORTC
    xor     A
VRAM_BLANK_1:
    or      (HL)
    jp      p, VRAM_BLANK_1

    BANK_RAM                        // バンク切替

    ret
__endasm;
}
#endif

// ---------------------------------------------------------------- デバッグ
#if DEBUG
u16 vramDebugGetProcessTime()
{
    //return (u16)(_8253CH1CT - s8253Ch1Ct) * 1000 / 15700;
    //return (u16)(_8253CH1CT - s8253Ch1Ct) * 16 / 251;
    //return (u16)(_8253CH1CT - s8253Ch1Ct) * 16 / 256;  近似値
    return (u16)(_8253CH1CT - s8253Ch1Ct) * 65 / 1024;
}
#endif

// ---------------------------------------------------------------- クリア
void vramClear() __z88dk_fastcall __naked
{
__asm
    ld      (VRAM_CLEAR_SP_RESTORE + 1), SP// SP 保存(自己書換)

    ld      B,  #VRAM_HEIGHT        // loop counter
    ld      DE, 0x0000
    ld      SP, #VVRAM_TEXT_ADDR(VRAM_WIDTH, 0)
VVRAM_CLEAR_LOOP:
    PUSH_DE_40_BYTES()              // TEXT
    ld      HL, #(VRAM_WIDTH + VVRAM_GAPX + VRAM_WIDTH)
    add     HL, SP
    ld      SP, HL
    PUSH_DE_40_BYTES()              // ATB
    ld      HL, #(VVRAM_WIDTH - VVRAM_GAPX)
    add     HL, SP
    ld      SP, HL
    djnz    B, VVRAM_CLEAR_LOOP
VRAM_CLEAR_SP_RESTORE:
    ld      SP, #0000               // SP 復活
    ret
__endasm;
}

// ---------------------------------------------------------------- 塗りつぶし(fill)
#pragma disable_warning 85
#pragma save
void vVramFillRect(const u8* const drawAddr, const u16 wh, const u16 code) __naked
{
__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     HL                      // drawAddr
    pop     BC                      // wh
    pop     DE                      // code
    ld      A, C                    // h
    exx
        ld  B, A                    // h
    exx
    ld      A, B                    // w 保存

    push    BC                      // wh   ※引数エリアを壊してしまうが問題ない
    push    HL                      // addr ※引数エリアを壊してしまうが問題ない

    // -------- TEXT
    exx
VRAM_FILL_RECT_TEXT_Y:
        exx
        ld      C, L                // L 保存
        ld      B, A                // w 復帰
VRAM_FILL_RECT_TEXT_X:
            ld      (HL), E         // ディスプレイ コード
            inc     L               // 仮想 VRAM x++
            djnz    B, VRAM_FILL_RECT_TEXT_X

        ld      L, C                // L 復帰
        inc     H                   // 仮想 VRAM y++
        exx
            djnz B, VRAM_FILL_RECT_TEXT_Y
        exx

    // -------- ATB
    pop     HL                      // addr
    ld      BC, #(VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))
    add     HL, BC
    pop     BC                      // wh
    ld      A, C                    // h
    exx
        ld  B, A                    // h
    exx
    ld      A, B                    // w

    exx
VRAM_FILL_RECT_ATB_Y:
        exx
        push    HL
        ld      B, A                // w
VRAM_FILL_RECT_ATB_X:
            ld      (HL), D         // ATB
            inc     L               // 仮想 VRAM x++
            djnz    B, VRAM_FILL_RECT_ATB_X

        pop     HL
        inc     H                   // 仮想 VRAM y++
        exx
            djnz B, VRAM_FILL_RECT_ATB_Y
        //exx   よく考えてみたら最後の exx は不要

    // スタックポインタの位置を戻して ret
    ld      HL, -8
    add     HL, SP
    ld      SP, HL
    ret
__endasm;
}
#pragma restore


#pragma disable_warning 85
#pragma save
void vramFill(const u16 code) __z88dk_fastcall __naked
{
__asm
    ld      (VRAM_FILL_SP_RESTORE + 1), SP// SP 保存(自己書換)
    BANK_VRAM_IO                    // バンク切替

    // -------- 画面クリアします
    ld      B,  VRAM_HEIGHT         // loop counter
    ld      D,  L                   // ディスプレイコード
    ld      E,  L
    ld      SP, #VRAM_TEXT_ADDR(0, VRAM_HEIGHT)
RVRAM_TEXT_FILL_LOOP:
    PUSH_DE_40_BYTES()
    djnz    B, RVRAM_TEXT_FILL_LOOP
    ld      B,  VRAM_HEIGHT         // loop counter
    ld      D,  H                   // ATB
    ld      E,  H
    ld      SP, #VRAM_ATB_ADDR(0, VRAM_HEIGHT)
RVRAM_ATB_FILL_LOOP:
    PUSH_DE_40_BYTES()
    djnz    B, RVRAM_ATB_FILL_LOOP

    BANK_RAM                        // バンク切替
VRAM_FILL_SP_RESTORE:
    ld      SP, #0x0000             // SP 復活
    ret
__endasm;
}
#pragma restore

// ---------------------------------------------------------------- 描画(draw)(任意の矩形)
void vVramDrawRect(const u8* const drawAddr, const u8* const srcAddr, const u16 wh) __naked
{
__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     DE                      // drawAddr
    pop     HL                      // srcAddr
    pop     BC                      // wh

    // -------- TEXT
    push    DE                      // 転送先保存

    // h ループカウンタを B' に移動
    ld      A, C                    // h
    ld      C, B                    // w
    ld      B, 0                    // w
    exx
        ld  B, A                    // h
        ld  C, A                    // h
VRAM_DRAW_RECT_TEXT_Y:
    exx
    push    BC                      // w ※引数エリアを壊してしまうが問題ない
    ld      A, E                    // E 保存 ※ D は変化しない
    ldir
    pop     BC
    // 仮想 VRAM y++
    ld      E, A                    // E 復帰
    inc     D
    exx
        djnz B, VRAM_DRAW_RECT_TEXT_Y
    exx
    pop     DE                      // 転送先復帰

    // -------- ATB
    // 仮想 VRAM TEXT → ATB
    push    HL
    ld      HL, #VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
    add     HL, DE
    ld      DE, HL
    pop     HL
    exx
        ld  B, C                    // h
VRAM_DRAW_RECT_ATB_Y:
    exx
    push    BC                      // w ※引数エリアを壊してしまうが問題ない
    ld      A, E                    // E 保存 ※ D は変化しない
    ldir
    pop     BC
    ld      E, A                    // E 復帰
    inc     D                       // 仮想 VRAM y++
    exx
        djnz B, VRAM_DRAW_RECT_ATB_Y
    exx

    // スタックポインタの位置を戻して ret
    ld      HL, -8
    add     HL, SP
    ld      SP, HL
    ret
__endasm;
}


void vVramDrawRectTransparent(const u8* const drawAddr, const u8* const srcAddr, const u16 wh) __naked
{
__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     DE                      // drawAddr
    pop     HL                      // srcAddr
    pop     BC                      // wh

    // -------- TEXT
    // h ループカウンタを B' に移動
    ld      A, C                    // h
    ld      C, B                    // w
    exx
        ld  B, A                    // h
VRAM_DRAW_RECT_TRANS_Y:
    exx
    ld      B, C                    // w
VRAM_DRAW_RECT_TRANS_X:
        // TEXT
        ld      A, (HL)
        inc     HL
        or      A
        jr      z, VRAM_DRAW_RECT_TRANS // 分岐することは少ないので jr を使う
            ld      (DE), A
            // 仮想 VRAM TEXT → ATB
            ld      A, E
            add     A, #VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
            ld      E, A
            // ATB
            ld      A, (HL)
            inc     HL
            ld      (DE), A
            // 仮想 VRAM ATB → TEXT
            ld      A, E
            sub     A, #VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
            ld      E, A
VRAM_DRAW_RECT_TRANS:
    // 仮想 VRAM x++
    inc     E
    djnz    B, VRAM_DRAW_RECT_TRANS_X
    // 仮想 VRAM x-= w
    ld      A, E
    sub     A, C
    ld      E, A
    // 仮想 VRAM y++
    inc     D
    exx
        djnz B, VRAM_DRAW_RECT_TRANS_Y
    exx

    // スタックポインタの位置を戻して ret
    ld      HL, -8
    add     HL, SP
    ld      SP, HL
    ret
__endasm;
}


void vramDrawRect(const u8* const drawAddr, const u8* const srcAddr, const u16 wh) __naked
{
__asm
    ld      (VRAM_DRAW_RECT_SP_RESTORE + 1), SP   // VRAM を切り替えるので, SP 保存(自己書換)
    pop     HL                      // リターン アドレス(捨てる)
    pop     DE                      // drawAddr
    pop     HL                      // srcAddr
    pop     BC                      // wh

    ld      SP, #ADDR_TMP_SP        // 臨時スタックポインタ
    ld      A, C                    // h 保存
    BANK_VRAM_IO                    // バンク切替 C 破壊
    ld      C, A                    // h 復帰

    // -------- TEXT
    push    DE                      // 転送先保存

    ld      A, C                    // h
    ld      C, B                    // w
    ld      B, 0                    // BC = w
    exx
        ld  B, A                    // h
        ld  C, A                    // h
RVRAM_DRAW_RECT_TEXT_LOOP_Y:
    exx
    push    BC                      // w
    push    DE
      ldir
    pop     DE
    pop     BC
    push    HL                      // 次の行へ
      ld    HL, #VRAM_WIDTH
      add   HL, DE
      ld    DE, HL
    pop     HL
    exx
        djnz B, RVRAM_DRAW_RECT_TEXT_LOOP_Y
    exx
    pop     DE                      // 転送先復帰

    // -------- ATB
    push    HL
      ld    HL, #(VRAM_ATB_ADDR(0, 0) - VRAM_TEXT_ADDR(0, 0))
      add   HL, DE
      ld    DE, HL
    pop     HL

    exx
        ld  B, C                    // h
RVRAM_DRAW_RECT_ATB_LOOP_Y:
    exx
    push    BC                      // w ※引数エリアを壊してしまうが問題ない
    push    DE
      ldir
    pop     DE
    pop     BC
    push    HL                      // 次の行へ
      ld      HL, #VRAM_WIDTH
      add     HL, DE
      ld      DE, HL
    pop     HL
    exx
        djnz B, RVRAM_DRAW_RECT_ATB_LOOP_Y
    //exx   よく考えてみたら最後の exx は不要

    // -------- 終了
    BANK_RAM                        // バンク切替
VRAM_DRAW_RECT_SP_RESTORE:
    ld      SP, #0x0000             // SP 復帰
    ret
__endasm;
}


// ---------------------------------------------------------------- 描画(draw)(1x1)
void vVramDraw1x1(const u8* const drawAddr, const u16 code) __naked
{
__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     HL                      // drawAddr
    pop     DE                      // code

    // TEXT
    ld      (HL), E

    // L += addr
    ld      A, #(VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))
    add     A, L
    ld      L, A

    // ATB
    ld      (HL), D

    // スタックポインタの位置を戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL

    ret
__endasm;
}

// ---------------------------------------------------------------- 描画(draw)(1x3)
void vVramDraw1x3(const u8* const drawAddr, const u8* const srcAddr) __naked
{
__asm
    pop     HL                      // リターン アドレス
    pop     DE                      // drawAddr
    pop     HL                      // srcAddr

    // TEXT
    ldi
    ldi
    ldi

    // E += addr
    ld      A, #(VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) - 3)
    add     A, E
    ld      E, A

    // ATB
    ldi
    ldi
    ldi

    // スタックポインタの位置を戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
    ret
__endasm;
}

// ---------------------------------------------------------------- 描画(draw)(3x3)
void vVramDraw3x3(const u8* const drawAddr, const u8* const srcAddr) __naked
{
__asm
    pop     HL                      // リターン アドレス
    pop     DE                      // drawAddr
    pop     HL                      // srcAddr

    // TEXT
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
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

    // DE += addr
    push    HL
    ld      HL, #((VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) - 0x0203)
    add     HL, DE
    ld      DE, HL
    pop     HL

    // ATB
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

    // スタックポインタの位置を戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
    ret
__endasm;
}

void vVramDraw3x3Transparent(const u8* const drawAddr, const u8* const srcAddr) __naked
{

// HL: source
// DE: dest
// B: VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)-1
// C: VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
#define VRAM_DRAW_3x3T(label)\
    ; TEXT                  \
    ld      A, (HL)         \
    inc     HL              \
    or      A               \
    jr      z, label        \
        ld      (DE), A     \
        ; addr 移動 TEXT → ATB\
        ld      A, E        \
        add     A, C        \
        ld      E, A        \
        ; ATB               \
        ld      A, (HL)     \
        inc     HL          \
        ld      (DE), A     \
        ; addr 移動 ATB → TEXT\
        ld      A, E        \
        sub     A, C        \
        ld      E, A        \
        ;\
label:\
        inc     E


#define VRAM_DRAW_3x3T_(label)\
    ; TEXT                  \
    ld      A, (HL)         \
    inc     HL              \
    or      A               \
    jr      z, label        \
        ld      (DE), A     \
        ; addr 移動 TEXT → ATB\
        ld      A, E        \
        add     A, C        \
        ld      E, A        \
        ; ATB               \
        ld      A, (HL)     \
        inc     HL          \
        ld      (DE), A     \
        ; addr 移動 ATB → TEXT\
        ld      A, E        \
        sub     A, C        \
        ld      E, A        \
        ;\
label:

__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     DE                      // drawAddr
    pop     HL                      // srcAddr

    ld      C, #(0x00 | (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)))
    VRAM_DRAW_3x3T(VRAM_DRAW_3x3T_00)
    VRAM_DRAW_3x3T(VRAM_DRAW_3x3T_01)
    VRAM_DRAW_3x3T_(VRAM_DRAW_3x3T_02)
    inc     D
    dec     E
    dec     E
    VRAM_DRAW_3x3T(VRAM_DRAW_3x3T_10)
    VRAM_DRAW_3x3T(VRAM_DRAW_3x3T_11)
    VRAM_DRAW_3x3T_(VRAM_DRAW_3x3T_12)
    inc     D
    dec     E
    dec     E
    VRAM_DRAW_3x3T(VRAM_DRAW_3x3T_20)
    VRAM_DRAW_3x3T(VRAM_DRAW_3x3T_21)
    VRAM_DRAW_3x3T_(VRAM_DRAW_3x3T_22)

    // スタックポインタの位置を戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
    ret
__endasm;
}

// ---------------------------------------------------------------- 描画(draw)(4x4)
void vVramDraw4x4(const u8* const drawAddr, const u8* const srcAddr) __naked
{
__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     DE                      // drawAddr
    pop     HL                      // srcAddr
    ld      BC, 0x04ff              // BC は ldi 命令でデクリメントするが, B は変化しない

    // TEXT
    ldi
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x04
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x04
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x04
    ld      E, A
    ldi
    ldi
    ldi
    ldi

    // DE += addr
    push    HL
    ld      HL, #((VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) - 0x0304)
    add     HL, DE
    ld      DE, HL
    pop     HL

    // ATB
    ldi
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x04
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x04
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x04
    ld      E, A
    ldi
    ldi
    ldi
    ldi

    // スタックポインタの位置を戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
    ret
__endasm;
}

// ---------------------------------------------------------------- 描画(draw)(5x5)
void vVramDraw5x5(const u8* const drawAddr, const u8* const srcAddr) __naked
{
__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     DE                      // drawAddr
    pop     HL                      // srcAddr
    ld      BC, 0x05ff              // BC は ldi 命令でデクリメントするが, B は変化しない

    // TEXT
    ldi
    ldi
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x05
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x05
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x05
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
#if 1
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x05
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
#endif
    // DE += addr
    push    HL
#if 1
    ld      HL, #((VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) - 0x0405)
#else
    ld      HL, #((VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) - 0x0305)
#endif
    add     HL, DE
    ld      DE, HL
    pop     HL

    // ATB
    ldi
    ldi
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x05
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x05
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x05
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
#if 1
    inc     D                       // 1行次へ
    ld      A, E
    sub     A, B                    // B = 0x05
    ld      E, A
    ldi
    ldi
    ldi
    ldi
    ldi
#endif
    // スタックポインタの位置を戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
    ret
__endasm;
}

#if 0 //今回は使ってない
void vVramDraw5x5Transparent(const u8* const drawAddr, const u8* const srcAddr) __naked
{

// HL: source
// DE: dest
// C: VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
#define VRAM_DRAW_5x5T(label)\
    ; TEXT                  \
    ld      A, (HL)         \
    inc     HL              \
    or      A               \
    jr      z, label        \
        ld      (DE), A     \
        ; addr 移動 TEXT → ATB\
        ld      A, E        \
        add     A, C        \
        ld      E, A        \
        ; ATB               \
        ld      A, (HL)     \
        inc     HL          \
        ld      (DE), A     \
        ; addr 移動 ATB → TEXT\
        ld      A, E        \
        sub     A, C        \
        ld      E, A        \
        ;\
label:\
        inc     E

#define VRAM_DRAW_5x5T_(label)\
    ; TEXT                  \
    ld      A, (HL)         \
    inc     HL              \
    or      A               \
    jr      z, label        \
        ld      (DE), A     \
        ; addr 移動 TEXT → ATB\
        ld      A, E        \
        add     A, C        \
        ld      E, A        \
        ; ATB               \
        ld      A, (HL)     \
        inc     HL          \
        ld      (DE), A     \
        ;\
label:

__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     DE                      // drawAddr
    pop     HL                      // srcAddr

    ld      BC, #(0x0500 | (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)))
VRAM_DRAW_5x5T_VLOOP:
    push    DE
    VRAM_DRAW_5x5T(VRAM_DRAW_5x5T_0)
    VRAM_DRAW_5x5T(VRAM_DRAW_5x5T_1)
    VRAM_DRAW_5x5T(VRAM_DRAW_5x5T_2)
    VRAM_DRAW_5x5T(VRAM_DRAW_5x5T_3)
    VRAM_DRAW_5x5T_(VRAM_DRAW_5x5T_4)
    inc     D
    dec     E
    dec     E
    dec     E
    dec     E
    djnz    B, RAM_DRAW_5x5T_VLOOP

    // スタックポインタの位置を戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
    ret
__endasm;
}
#endif