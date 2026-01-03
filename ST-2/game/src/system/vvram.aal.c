/**
 * 仮想 VRAM 転送, クリア色, Joystick 入力, 転送
 *
 * @author Snail Barbarian Macho (NWK)
 */
#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "../../../../src-common/aal80.h"
#include "addr.h"
#include "input.h"
#include "vvram.h"
#include "math.h"

// ---------------------------------------------------------------- 変数
u8 b_vram_trans_enabled_;
#if DEBUG
static u16 ct_8253_ct12_val_;  // 8253 ct1/2 のカウント値(256 = 15.7ms)
static u16 ct_8253_ct12_diff_; // 8253 ct1/2 の差分(256 = 15.7ms)
u8  vram_8253_ct2_;            // 8253 ct2 のカウンタ
u16 vram_trans_v_counter_;     // vramTrans() 時の Vカウンタ
#endif

// ---------------------------------------------------------------- マクロ
#define CT_8253_CT1_NTSC    262 // 8253 のカウンタ. 1 カウンタ 1 ライン. 262 ラインで 1 フレーム
#define CT_8253_CT1_PAL     312 // PAL の場合

// 一時的に使うワークエリア
#define ADDR_TMP_SP    (VVRAM_TMP_WORK + 16) // 臨時スタックポインタ

// ---------------------------------------------------------------- VRAM 転送サブルーチン
/** VRAM 転送 の初期化 */
static void vramTransInit_(void) __z88dk_fastcall
{
__asm
    // 10+10+10+4+10+10 = 54
    macro   POP_10_BYTES_TO_REGISTERS
        pop     HL
        pop     DE
        pop     BC
        exx
        pop     DE
        pop     BC
    endm

    // 11+11+4+11+11+11 = 59
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

    BANKH_VRAM_MMIO C                  // バンク切替

    // ---------------- デバッグ用カウント計測
#if DEBUG
    // ct1 (1～262) を 0-255 に変換して, 下位バイトとし,
    // ct2 の 8bit を上位バイトとして 16bit の値を作ります
    // たまに ct1 と 2 の間でズレが起きるだろうが, デバッグだからいいか
    // だいたい 1msec くらいの精度で値が出ればいい
    ld      HL, 0 + MMIO_8253_CTRL
    ld      (HL), 0 + MMIO_8253_CTRL_RL_LATCH_MASK |MMIO_8253_CTRL_CT1_MASK
    dec     L       // ct2
    ld      A, (HL) // ct2-L
    ld      (_vram_8253_ct2_), A
    ld      B, A
    dec     L       // ct1
    ld      E, (HL) // ct1 L
    ld      D, (HL) // ct1 H
    srl     D       // DE >>= 4 で, 0～262 が, 0～16 になります
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
    sbc     HL, BC // 1くらいは誤差!  カウンタはカウントダウンしていくので, 前の値の方が大きい
    ld      (_ct_8253_ct12_diff_), HL
#endif
    // ---------------- vramTransMain_() のアドレス初期化
    ld      HL, 0 + VVRAM_TEXT_ADDR(0, 0)
    ld      (vvramTrans_textSrc0_1 + 1), HL  // 転送元(自己書換)
    ld      HL, 0 + VRAM_TEXT_ADDR(10, 0)
    ld      (vvramTrans_textDst0_1 + 1), HL  // 転送先(自己書換)

    // ---------------- V-Blank (/VBLK の立下がり)を待ちます. (遅いのでボツ)
#if 0
    // 速度低下が著しいならば, 多少の画面の乱れは無視して同期を待たなくてもいいです.
    //
    // 時間計測結果(Debug, Caravan, 最初のステージ)
    // ---------------------------------------------------------
    // V-Blank 待ち有り, 200 ライン = 50msec
    // V-Blank 待ち無し, 200 ライン = 38msec
    // V-Blank 待ち無し, 262 ライン = 41msec
    xor     A
    ld      HL,    0 + MMIO_8255_PORTC
VBLK_SYNC0:
    or      (HL)                            // /VBLK = H になるまで待つ
    jp      A, p,    VBLK_SYNC0
VBLK_SYNC1:
    and     (HL)                            // /VBLK = L になるまで待つ
    jp      A, m,    VBLK_SYNC1
#endif

    BANKH_RAM C                             // バンク切替
__endasm;
}


/** VRAM 転送本体
 * @param lines キャラクタ行数. 0 なら何も転送しない
 */
#pragma save
#pragma disable_warning 85                              // 引数未使用警告抑止
static void vvramTransMain_(const u8 lines) __aal __z88dk_fastcall
{
    AAL_DEF_VARS;
    extern vvramTrans_restoreSP_1, vvramTrans_loop;
    extern vvramTrans_textSrc0_1, vvramTrans_textDst0_1;
    extern vvramTrans_textSrc1_1, vvramTrans_textDst1_1;
    extern vvramTrans_textSrc2_1, vvramTrans_textDst2_1;
    extern vvramTrans_textSrc3_1, vvramTrans_textDst3_1;
    extern vvramTrans_atbSrc0_1, vvramTrans_atbDst0_1;
    extern vvramTrans_atbSrc1_1, vvramTrans_atbDst1_1;
    extern vvramTrans_atbSrc2_1, vvramTrans_atbDst2_1;
    extern vvramTrans_atbSrc3_1, vvramTrans_atbDst3_1;

   A = L; A &= A; ret_z();                             // A = ループ カウンタ
    mem[vvramTrans_restoreSP_1 + 1] = SP;               // スタック保存
    BANKH_VRAM_MMIO(C);                                 // バンク切替

    // 10 バイト転送 x 4 で 1行分転送します
vvramTrans_loop:
    {
        // ---- TEXT 0 番目の 10 bytes
vvramTrans_textSrc0_1:  SP = 0x0000;                    // 転送元(自己書換)
        pop(HL, DE, BC); exx(); pop(DE, BC);
        mem[vvramTrans_textSrc1_1 + 1] = SP;            // 次の転送元(自己書換)
vvramTrans_textDst0_1:  SP = 0x0000;                    // 転送先(自己書換)
        push(BC, DE); exx(); push(BC, DE, HL);
        HL = 20; HL += SP;                              // 転送先アドレス移動
        mem[vvramTrans_textDst1_1 + 1] = HL;            // 次の転送先

        // ---- TEXT 1 番目の 10 bytes
vvramTrans_textSrc1_1:  SP = 0x0000;                    // 転送元(自己書換)
        pop(HL, DE, BC); exx(); pop(DE, BC);
        mem[vvramTrans_textSrc2_1 + 1] = SP;            // 転送元(自己書換)
vvramTrans_textDst1_1:  SP = 0x0000;                    // 転送先(自己書換)
        push(BC, DE); exx(); push(BC, DE, HL);
        HL = 20; HL += SP;                              // 転送先アドレス移動
        mem[vvramTrans_textDst2_1 + 1] = HL;            // 転送先(自己書換)

        // ---- TEXT 2 番目の 10 bytes
vvramTrans_textSrc2_1:  SP = 0x0000;                    // 転送元(自己書換)
        pop(HL, DE, BC); exx(); pop(DE, BC);
        mem[vvramTrans_textSrc3_1 + 1] = SP;            // 転送元(自己書換)
vvramTrans_textDst2_1:  SP = 0x0000;                    // 転送先(自己書換)
        push(BC, DE); exx(); push(BC, DE, HL);
        HL = 20; HL += SP;                              // 転送先アドレス移動
        mem[vvramTrans_textDst3_1 + 1] = HL;            // 転送先(自己書換)

        // ---- TEXT 3 番目の 10 bytes
vvramTrans_textSrc3_1:  SP = 0x0000;                    // 転送元(自己書換)
        pop(HL, DE, BC); exx(); pop(DE, BC);
        HL = VVRAM_GAPX; HL += SP;                      // 転送元アドレスを, 仮想 ATB に移動
        mem[vvramTrans_atbSrc0_1 + 1] = HL;             // 転送元(自己書換)
vvramTrans_textDst3_1:  SP = 0x0000;                    // 転送先(自己書換)
        push(BC, DE); exx(); push(BC, DE, HL);
        HL =(VRAM_ATB - VRAM_TEXT) - 20; HL += SP;      // 転送先アドレスを ATB に移動
        mem[vvramTrans_atbDst0_1 + 1] = HL;             // 転送先(自己書換)

        // ---- ATB 0 番目の 10 bytes
vvramTrans_atbSrc0_1:   SP = 0x0000;                    // 転送先(自己書換)
        pop(HL, DE, BC); exx(); pop(DE, BC);
        mem[vvramTrans_atbSrc1_1 + 1] = SP;             // 転送元(自己書換)
vvramTrans_atbDst0_1:   SP = 0x0000;                    // 転送先(自己書換)
        push(BC, DE); exx(); push(BC, DE, HL);
        HL = 20; HL += SP;                              // 転送先アドレス移動
        mem[vvramTrans_atbDst1_1 + 1] = HL;             // 転送先(自己書換)

        // ---- ATB 1 番目の 10 bytes
vvramTrans_atbSrc1_1:   SP = 0x0000;                    // 転送元(自己書換)
        pop(HL, DE, BC); exx(); pop(DE, BC);
        mem[vvramTrans_atbSrc2_1 + 1] = SP;             // 転送元(自己書換)
vvramTrans_atbDst1_1:   SP = 0x0000;                    // 転送先(自己書換)
        push(BC, DE); exx(); push(BC, DE, HL);
        HL = 20; HL += SP;                              // 転送先アドレス移動
        mem[vvramTrans_atbDst2_1 + 1] = HL;             // 転送先(自己書換)

        // ---- ATB 2 番目の 10 bytes
vvramTrans_atbSrc2_1:   SP = 0x0000;                    // 転送元(自己書換)
        pop(HL, DE, BC); exx(); pop(DE, BC);
        mem[vvramTrans_atbSrc3_1 + 1] = SP;             // 転送元(自己書換)
vvramTrans_atbDst2_1:   SP = 0x0000;                    // 転送先(自己書換)
        push(BC, DE); exx(); push(BC, DE, HL);
        HL = 20; HL += SP;                              // 転送先アドレス移動
        mem[vvramTrans_atbDst3_1 + 1] = HL;             // 転送先(自己書換)

        // ---- ATB 3 番目の 10 bytes
vvramTrans_atbSrc3_1:   SP = 0x0000;                    // 転送元(自己書換)
        pop(HL, DE, BC); exx(); pop(DE, BC);
        HL = VVRAM_WIDTH - VRAM_WIDTH - VVRAM_GAPX - VRAM_WIDTH; HL += SP;  // 転送元アドレスを, 仮想 TEXT の次の行に移動
        mem[vvramTrans_textSrc0_1 + 1] = HL;            // 転送元(自己書換)
vvramTrans_atbDst3_1:   SP = 0x0000;                    // 転送先(自己書換)
        push(BC, DE); exx(); push(BC, DE, HL);
        HL = (VRAM_TEXT - VRAM_ATB) + VRAM_WIDTH - 20; HL += SP; // 転送先アドレスを TEXT の次行に移動
        mem[vvramTrans_textDst0_1 + 1] = HL;            // 転送先(自己書換)

        // ---- Loop end
        A--; jp_nz(vvramTrans_loop);
    }

vvramTrans_restoreSP_1: SP = 0x0000;                    // スタック復帰(自己書換)
    BANKH_RAM(C);
}
#pragma restore


// ---------------------------------------------------------------- システム
/**
 * /VBLK の立下がりを待ちます.
 * 立ち下がってから, 最大 17 T-states だけの遅延があります
 * @param BANK MMIO に切り替わってること
 * @param HL: MMIO_8255_PORTC
 */
#pragma save
#pragma disable_warning 85                              // 引数未使用警告抑止
static void WAIT_VSYNC(int reg_A, int reg_HL) __aal_macro __naked
{
    AAL_DEF_VARS;

    A ^= A;
    do { A |= mem[HL]; } while (p);                     //          /VBLK = H になるまで待つ. '0' ならループ
    do { A &= mem[HL]; } while (m);                     //  7+10    /VBLK = L になるまで待つ. '1' ならばループ

    AAL_ENDM;
}
#pragma restore


void vvramInit(void) __aal __z88dk_fastcall __naked
{
    AAL_DEF_VARS;
    extern _b_vram_trans_enabled_;
//    extern vvramInitRestoreSP_1;
    extern _vvramClear;

    A = true; mem[_b_vram_trans_enabled_] = A;

//    mem[vvramInitRestoreSP_1 + 1] = SP;                 // SP 保存(自己書換)
//    SP = ADDR_TMP_SP;

    // ---- 8253 の CT1 を, 現在のスキャンラインを知るのに使います
    // デバッグ時は CT1 と CT2 を使って, 1フレームでかかった時間を知るのに使います.
    // CT1 は モード2 (ct=262),
    // CT2 は モード0 (ct=65535)
    // で運用します
    BANKH_VRAM_MMIO(C);                                 // バンク切替

    // -------- V カウンタの設定(1)
    HL = MMIO_8253_CTRL;
    mem[HL] = MMIO_8253_CTRL_CT1(MMIO_8253_CTRL_RL_LH_MASK, MMIO_8253_CTRL_MODE2_MASK);
#if DEBUG
    mem[HL] = MMIO_8253_CTRL_CT2(MMIO_8253_CTRL_RL_L_MASK, MMIO_8253_CTRL_MODE2_MASK);
    L--;                                                // L = CT2
    mem[HL] = 0x00;
#endif

    // -------- NTSC/PAL の判別
    // /VBLANK == 'L' の時間は,
    // - NTSC: 228   * 62  = 14136 T-states
    // - PAL:  227.2 * 112 = 25446 T-states
    // なので,
    // /VBLANK の立下がりを検出してから約 20000 T-states 経過後, まだ /VBLANK だったら PAL と判定します
    HL = MMIO_8255_PORTC;
    WAIT_VSYNC(A, HL);
    BC = 768; do { BC--; A = B; A |= C; } while (nz_jr);// (6+4+4+12)*770-5 = 19963 T-states

    bit(MMIO_8255_PORTC_VBLK_SHIFT, mem[HL]);
    DE     = CT_8253_CT1_NTSC; BC = 2269;               // 待ち時間(NTSC)
    if (z_jr) {                                         // まだ /VBLK 中ならば PAL
        DE = CT_8253_CT1_PAL;  BC = 2696;               // 待ち時間(PAL)
    }

    // -------- Vカウンタの設定(2)
    // 再び, /VBLANK の立下がりを検出して, 次の /VBLK の 1クロック前(日本版なら 228 T-states) からカウント開始!
    //
    //       1フレーム分の時間(T-states)   1クロック前とオーバーヘッドを考慮したBC値
    // -----------------------------------------------------------------------------------
    // NTSC: 228   * ( 62+198) = 59280     (59280 - 228 - 17 + 5-7-7-7) / (6+4+4+12) = 2269
    // PAL:  227.2 * (112+198) = 70432     (70432 - 284 - 17 + 5-7-7-7) / (6+4+4+12) = 2696
    WAIT_VSYNC(A, HL);                                  // 最大 17 T-states
    do { BC--; A = B; A |= C; } while (nz_jr);          // (6+4+4+12)*BC-5
    // カウンタ 1 を設定してカウント開始! (2025.11 実際のカウント開始は1クロックくらい遅れるらしい)
    L = MMIO_8253_CT1 & 0xff;                           //  7
    mem[HL] = E; mem[HL] = D;                           //  7+7 カウンタ 1 L->H 順に書いてでカウント開始

    BANKH_RAM(C);                                       // バンク切替

//vvramInitRestoreSP_1: SP = 0x0000;                      // 設定復帰

    jp(_vvramClear);                                    // 仮想画面クリアして終了

    AAL_NO_RETURN;
}


#pragma save
#pragma disable_warning 59                              // 戻値未設定警告抑止
static u8 vvramGetVCounter_(void) __z88dk_fastcall
{
__asm
    BANKH_VRAM_MMIO C                                   // バンク切替
    // カウンタ ラッチ モードを使ってデータを読みだす
    ld      HL, 0 + MMIO_8253_CTRL
    ld      (HL), 0 + MMIO_8253_CTRL_RL_LATCH_MASK | MMIO_8253_CTRL_CT1_MASK
    ld      L,  0 + (MMIO_8253_CT1 & 0xff)
    ld      A,  (HL)                                    // ct1 L
    ld      H,  (HL)                                    // ct1 H
    ld      L,  A
    BANKH_RAM C                                         // バンク切替
    // HL = 1～262
#if 0 // カウンタと /VBLK の対比をチェックするデバッグ(VRAM_GET_VCOUNTER_TEST1でブレークをかける)
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
    // 1 引いて 8 で割る
    dec     HL
    srl     H
    rr      L
    srl     L
    srl     L
    // 21 に飽和して, L を返す if (21 < L) { L = 21 }
    ld      A, 0 + (VRAM_HEIGHT - 4)
    cmp     A, L
    ret     nc
    ld      L, A
__endasm;
}
#pragma restore


void vvramTrans(void) __z88dk_fastcall __naked
{
    // ---------------- VRAM 転送許可
    if (!b_vram_trans_enabled_) {
        b_vram_trans_enabled_ = true; // 次は転送許可
__asm
        ret
__endasm;
    }
    vramTransInit_();

    if (inputGetJoyMode() < INPUT_JOY_MODE_MZ1X03_DETECTING) {
        // AM7J モード
        vvramTransMain_(VRAM_HEIGHT);
    } else {
        // MZ-1X03 を検出するモード
        //
        // V ... /VBLK H->L 立下がり待ち
        // B ... ボタンチェック+/VBLK立下がり+検出チェック+1回目の軸チェック
        // A ... 2回目の軸チェック
        // - ... 画面転送
        // = ... 画面転送 8行x4

        // NTSC 版
        // v-count  ...|182|183|...|190|191|...|196|197|198|199|200|201|.......|261| 0 | 1 |...
        // 8253 ct1 ...| 17| 16|...| 9 | 8 |...| 3 | 2 | 1 |262|261|260|.......|200|199|198|...
        // PAL 版
        // v-count  ...|182|183|...|190|191|...|196|197|198|199|200|201|.......|311| 0 | 1 |...
        // 8253 ct1 ...| 17| 16|...| 9 | 8 |...| 3 | 2 | 1 |312|311|310|.......|200|199|198|...
        //          ..._____..._____________..._________________                    ________...
        // /VBLK           :           :                       |________...____..._|
        // (ct1-1)/8       :           :                       :
        // 0               :           :                   |BBBBBB|=====...==|A|------------...(8行x21)
        // 0               :           : |BB...BBBBBBBBBBBBBBBBBBB|=====...==|A|------------...(8行x21)
        // 1               :           |----...(8行x1)-----|BBBBBB|=====...==|A|------------...(8行x20)
        // 1               :        |-------...(8行x1)--|BBBBBBBBB|=====...==|A|------------...(8行x20)
        // 2               |-...------------...(8行x2)-----|BBBBBB|=====...==|A|------------...(8行x19)
        // 2            |----...------------...(8行x2)--|BBBBBBBBB|=====...==|A|------------...(8行x19)
        // 21     |-...------...------------...(8行x21)-|BBBBBBBBB|=====...==|A|
        // 22    |--...------...(8行x21)---|...BBBBBBBBBBBBBBBBBBB|=====...==|A|
        // 32 |-----...--(8行x21)-|BBBBBBBBB...BBBBBBBBBBBBBBBBBBB|=====...==|A|

        u8 vc = vvramGetVCounter_(); // 0～32
        u8  s = inputGetMZ1X03sensitivity(); // MZ-1X03 感度(0鈍い～3敏感)
        vvramTransMain_(vc);
        // この時点で必ず /VBLK 外の筈!
        inputMZ1X03ButtonVSyncAxis1(s);
        s = 4 - s;
        vvramTransMain_(s);
        inputMZ1X03Axis2();
        vvramTransMain_(VRAM_HEIGHT - vc - s);
    }

    vvramClear();                                       // 仮想画面クリアして終了
}


// ---------------------------------------------------------------- デバッグ
#if DEBUG
u16 vramDebugGetProcessTime(void)
{
    // 256 = 15.7ms
    //return ct_8253_ct12_diff_ * 15.7 / 256; // 正確な値
    return ct_8253_ct12_diff_ / 16;  // 近似値
}
#endif


// ---------------------------------------------------------------- クリア
void vvramClear(void) __z88dk_fastcall
{
__asm
    ld      (VRAM_CLEAR_SP_RESTORE + 1), SP             // SP 保存(自己書換)

    ld      B,  VRAM_HEIGHT                             // loop counter
    ld      DE, 0x0000
    ld      SP, 0 + VVRAM_TEXT_ADDR(VRAM_WIDTH, 0)
VVRAM_CLEAR_LOOP:
    PUSH_DE_10_BYTES                                    // TEXT
    PUSH_DE_10_BYTES
    PUSH_DE_10_BYTES
    PUSH_DE_10_BYTES
    ld      HL, 0 + VRAM_WIDTH + VVRAM_GAPX + VRAM_WIDTH
    add     HL, SP
    ld      SP, HL
    PUSH_DE_10_BYTES                                    // ATB
    PUSH_DE_10_BYTES
    PUSH_DE_10_BYTES
    PUSH_DE_10_BYTES
    ld      HL, 0 + (VVRAM_WIDTH - VVRAM_GAPX)
    add     HL, SP
    ld      SP, HL
    djnz    B, VVRAM_CLEAR_LOOP
VRAM_CLEAR_SP_RESTORE:
    ld      SP, 0000                                    // SP 復活
__endasm;
}


// ---------------------------------------------------------------- 塗りつぶし(fill)
#pragma save
#pragma disable_warning 85                              // 引数未使用警告抑止
void vvramFillRect(const u8* const draw_addr, const u16 wh, const u16 code)
{
__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     HL                      // draw_addr
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
    ld      BC, 0 + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))
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
__endasm;
}
#pragma restore


#pragma save
#pragma disable_warning 85                              // 引数未使用警告抑止
void vramFill(const u16 code) __z88dk_fastcall
{
    // 10byte 単位で TEXT->ATB と書いていきます.
    // 1/60 sec でクリアされます.
__asm
    ld      (VRAM_FILL_SP_RESTORE + 1), SP              // SP 保存(自己書換)
    BANKH_VRAM_MMIO C                                   // バンク切替

    ld      BC, HL                                      // ATB + TEXT
    ld      SP, 0 + VRAM_TEXT_ADDR(10, 0)
    ld      A,  VRAM_HEIGHT * 4                         // loop counter

VRAM_FILL_LOOP:
        // TEXT の消去
        ld      D, C                                    // TEXT
        ld      E, C                                    // TEXT
        PUSH_DE_10_BYTES

        ld  HL, 10 + VRAM_ATB - VRAM_TEXT
        add HL, SP
        ld  SP, HL

        // ATB の消去
        ld      D, B                                    // ATB
        ld      E, B                                    // ATB
        PUSH_DE_10_BYTES

        ld  HL, 20 + VRAM_TEXT - VRAM_ATB
        add HL, SP
        ld  SP, HL

        // ループ末端
        dec A
        jr  nz, VRAM_FILL_LOOP

    BANKH_RAM C                                         // バンク切替
VRAM_FILL_SP_RESTORE:
    ld      SP, 0x0000                                  // SP 復活
__endasm;
}
#pragma restore


// ---------------------------------------------------------------- 描画(draw)(任意の矩形)
void vvramDrawRect(const u8* const draw_addr, const u8* const stc_addr, const u16 wh)
{
__asm
    pop     HL                                          // リターン アドレス(捨てる)
    pop     DE                                          // draw_addr
    pop     HL                                          // stc_addr
    pop     BC                                          // wh

    // -------- TEXT
    push    DE                                          // 転送先保存

    // h ループカウンタを B' に移動
    ld      A, C                                        // h
    ld      C, B                                        // w
    ld      B, 0                                        // w
    exx
        ld  B, A                                        // h
        ld  C, A                                        // h
VRAM_DRAW_RECT_TEXT_Y:
    exx
    push    BC                                          // w ※引数エリアを壊してしまうが問題ない
    ld      A, E                                        // E 保存 ※ D は変化しない
    ldir
    pop     BC
    // 仮想 VRAM y++
    ld      E, A                                        // E 復帰
    inc     D
    exx
        djnz B, VRAM_DRAW_RECT_TEXT_Y
    exx
    pop     DE                                          // 転送先復帰

    // -------- ATB
    // 仮想 VRAM TEXT → ATB
    push    HL
    ld      HL, 0 + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
    add     HL, DE
    ld      DE, HL
    pop     HL
    exx
        ld  B, C                                        // h
VRAM_DRAW_RECT_ATB_Y:
    exx
    push    BC                                          // w ※引数エリアを壊してしまうが問題ない
    ld      A, E                                        // E 保存 ※ D は変化しない
    ldir
    pop     BC
    ld      E, A                                        // E 復帰
    inc     D                                           // 仮想 VRAM y++
    exx
        djnz B, VRAM_DRAW_RECT_ATB_Y
    exx

    // スタックポインタの位置を戻して ret
    ld      HL, -8
    add     HL, SP
    ld      SP, HL
__endasm;
}


void vvramDrawRectTransparent(const u8* const draw_addr, const u8* const stc_addr, const u16 wh)
{
__asm
    pop     HL                                          // リターン アドレス(捨てる)
    pop     DE                                          // draw_addr
    pop     HL                                          // stc_addr
    pop     BC                                          // wh

    // -------- TEXT
    // h ループカウンタを B' に移動
    ld      A, C                                        // h
    ld      C, B                                        // w
    exx
        ld  B, A                                        // h
VRAM_DRAW_RECT_TRANS_Y:
    exx
    ld      B, C                                        // w
VRAM_DRAW_RECT_TRANS_X:
        // TEXT
        ld      A, (HL)
        inc     HL
        or      A
        jr      z, VRAM_DRAW_RECT_TRANS                 // 分岐することは少ないので jr を使う
            ld      (DE), A
            // 仮想 VRAM TEXT → ATB
            ld      A, E
            add     A, 0 + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
            ld      E, A
            // ATB
            ld      A, (HL)
            inc     HL
            ld      (DE), A
            // 仮想 VRAM ATB → TEXT
            ld      A, E
            sub     A, 0 + VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
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
__endasm;
}


void vramDrawRect(const u8* const draw_addr, const u8* const stc_addr, const u16 wh)
{
__asm
    ld      (VRAM_DRAW_RECT_SP_RESTORE + 1), SP         // VRAM を切り替えるので, SP 保存(自己書換)
    pop     HL                                          // リターン アドレス(捨てる)
    pop     DE                                          // draw_addr
    pop     HL                                          // stc_addr
    pop     BC                                          // wh

    ld      SP, 0 + ADDR_TMP_SP                         // 臨時スタックポインタ

    ld      A, C                                        // h 保存
    BANKH_VRAM_MMIO C                                   // バンク切替(C 破壊)
    ld      C, A                                        // h 復帰

    // -------- TEXT
    push    DE                                          // 転送先保存

    ld      A, C                                        // h
    ld      C, B                                        // w
    ld      B, 0                                        // BC = w
    exx
        ld  B, A                                        // h
        ld  C, A                                        // h
RVRAM_DRAW_RECT_TEXT_LOOP_Y:
    exx
    push    BC                                          // w
    push    DE
      ldir
    pop     DE
    pop     BC
    push    HL                                          // 次の行へ
      ld    HL, 0 + VRAM_WIDTH
      add   HL, DE
      ld    DE, HL
    pop     HL
    exx
        djnz B, RVRAM_DRAW_RECT_TEXT_LOOP_Y
    exx
    pop     DE                                          // 転送先復帰

    // -------- ATB
    push    HL
      ld    HL, 0 + (VRAM_ATB_ADDR(0, 0) - VRAM_TEXT_ADDR(0, 0))
      add   HL, DE
      ld    DE, HL
    pop     HL

    exx
        ld  B, C                                        // h
RVRAM_DRAW_RECT_ATB_LOOP_Y:
    exx
    push    BC                                          // w ※引数エリアを壊してしまうが問題ない
    push    DE
      ldir
    pop     DE
    pop     BC
    push    HL                                          // 次の行へ
      ld      HL, 0 + VRAM_WIDTH
      add     HL, DE
      ld      DE, HL
    pop     HL
    exx
        djnz B, RVRAM_DRAW_RECT_ATB_LOOP_Y
    //exx   よく考えてみたら最後の exx は不要

    // -------- 終了
    BANKH_RAM C                                         // バンク切替
VRAM_DRAW_RECT_SP_RESTORE:
    ld      SP, 0x0000                                  // SP 復帰
__endasm;
}


// ---------------------------------------------------------------- 描画(draw)(1x1)
void vvramDraw1x1(const u8* const draw_addr, const u16 code)
{
__asm
    pop     HL                                          // リターン アドレス(捨てる)
    pop     HL                                          // draw_addr
    pop     DE                                          // code

    // TEXT
    ld      (HL), E

    // L += addr
    ld      A, 0 + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0))
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
void vvramDraw1x3(const u8* const draw_addr, const u8* const stc_addr)
{
__asm
    pop     HL                                          // リターン アドレス
    pop     DE                                          // draw_addr
    pop     HL                                          // stc_addr

    // TEXT
    ldi
    ldi
    ldi

    // E += addr
    ld      A, 0 + (VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0) - 3)
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
__endasm;
}


// ---------------------------------------------------------------- 描画(draw)(3x3)
void vvramDraw3x3(const u8* const draw_addr, const u8* const stc_addr)
{
__asm
    pop     HL                                          // リターン アドレス
    pop     DE                                          // draw_addr
    pop     HL                                          // stc_addr

    // TEXT
    ldi
    ldi
    ldi
    inc     D                                           // 1行次へ
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
    ld      HL, 0 + ((VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) - 0x0203)
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
__endasm;
}


void vvramDraw3x3Transparent(const u8* const draw_addr, const u8* const stc_addr)
{
__asm

// HL: source
// DE: dest
// B: VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)-1
// C: VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
    macro VRAM_DRAW_3x3T
        local   label
        // TEXT
        ld      A, (HL)
        inc     HL
        or      A
        jr      z, label
            ld      (DE), A
            // addr 移動 TEXT → ATB
            ld      A, E
            add     A, C
            ld      E, A
            // ATB
            ld      A, (HL)
            inc     HL
            ld      (DE), A
            // addr 移動 ATB → TEXT
            ld      A, E
            sub     A, C
            ld      E, A
label:
            inc     E
    endm


    macro   VRAM_DRAW_3x3T_
        local   label
        // TEXT
        ld      A, (HL)
        inc     HL
        or      A
        jr      z, label
            ld      (DE), A
            // addr 移動 TEXT → ATB
            ld      A, E
            add     A, C
            ld      E, A
            // ATB
            ld      A, (HL)
            inc     HL
            ld      (DE), A
            // addr 移動 ATB → TEXT
            ld      A, E
            sub     A, C
            ld      E, A
label:
    endm


    pop     HL                      // リターン アドレス(捨てる)
    pop     DE                      // draw_addr
    pop     HL                      // stc_addr

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

    // スタックポインタの位置を戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
__endasm;
}


// ---------------------------------------------------------------- 描画(draw)(4x4)
void vvramDraw4x4(const u8* const draw_addr, const u8* const stc_addr)
{
__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     DE                      // draw_addr
    pop     HL                      // stc_addr
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
    ld      HL, 0 + ((VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) - 0x0304)
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
__endasm;
}


// ---------------------------------------------------------------- 描画(draw)(5x5)
void vvramDraw5x5(const u8* const draw_addr, const u8* const stc_addr)
{
__asm
    pop     HL                      // リターン アドレス(捨てる)
    pop     DE                      // draw_addr
    pop     HL                      // stc_addr
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
    ld      HL, 0 + ((VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) - 0x0405)
#else
    ld      HL, 0 + ((VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)) - 0x0305)
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
__endasm;
}


#if 0 //今回は使ってない
void vvramDraw5x5Transparent(const u8* const draw_addr, const u8* const stc_addr)
{

__asm

// HL: source
// DE: dest
// C: VVRAM_ATB_ADDR(0, 0) - VVRAM_TEXT_ADDR(0, 0)
    macro VRAM_DRAW_5x5T
        local   label
        // TEXT
        ld      A, (HL)
        inc     HL
        or      A
        jr      z, label
            ld      (DE), A
            // addr 移動 TEXT → ATB
            ld      A, E
            add     A, C
            ld      E, A
            // ATB
            ld      A, (HL)
            inc     HL
            ld      (DE), A
            // addr 移動 ATB → TEXT
            ld      A, E
            sub     A, C
            ld      E, A
label:
        inc     E
    endm


    macro VRAM_DRAW_5x5T_
        local label
        // TEXT
        ld      A, (HL)
        inc     HL
        or      A
        jr      z, label
            ld      (DE), A
            // addr 移動 TEXT → ATB
            ld      A, E
            add     A, C
            ld      E, A
            // ATB
            ld      A, (HL)
            inc     HL
            ld      (DE), A
label:
    endm


    pop     HL                      // リターン アドレス(捨てる)
    pop     DE                      // draw_addr
    pop     HL                      // stc_addr

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

    // スタックポインタの位置を戻して ret
    ld      HL, -6
    add     HL, SP
    ld      SP, HL
__endasm;
}
#endif