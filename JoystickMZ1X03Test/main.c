/**
 * 三重和音サウンド テスト
 *
 * @author Snail Barbarian Macho (NWK)
 */
#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "../src-common/display_code.h"

#define INPUT_MASK_A 0x01   // A ボタン
#define INPUT_MASK_B 0x02   // B ボタン
#define INPUT_MASK_L 0x04   // 左
#define INPUT_MASK_R 0x08   // 右
#define INPUT_MASK_D 0x10   // 下
#define INPUT_MASK_U 0x20   // 上
#define INPUT_MASK_NC   0x80  // 未検出フラグ
#define INPUT_SHIFT_NC  0x7   // 未検出フラグ

#define VRAM_TEXT_JA1    (VRAM_TEXT + 40 * 1)
#define VRAM_TEXT_JA2    (VRAM_TEXT + 40 * 10)
#define VRAM_TEXT_DETECT (VRAM_TEXT + 40 * 20)
#define VRAM_ATB_JA1     (VRAM_ATB + 40 * 1)
#define VRAM_ATB_JA2     (VRAM_ATB + 40 * 10)

#define INPUT_AXIS_NR_SAMPLES (80*7)  // サンプリング数. 4 の倍数でないと暴走する
static u8 axis_data[INPUT_AXIS_NR_SAMPLES];

/** シャープ純正ジョイスティックの軸をサンプリングします
 * - VBLANK を待ち, 更に 512x28 T states かかります
 */
static void inputMZ1X03Axis_() __z88dk_fastcall __naked
{
__asm
    ld      DE, #_axis_data
    ld      BC, #INPUT_AXIS_NR_SAMPLES
    // -------- /VBLK の立下がりを待つ
    xor     A
    ld      HL, #MMIO_8255_PORTC
INPUT_MZ1X03_VBLK_SYNC10:
    or      A, (HL)                 // /VBLK = H になるまで待つ
    jp      p,  INPUT_MZ1X03_VBLK_SYNC10
INPUT_MZ1X03_VBLK_SYNC11:
    and     A, (HL)                 // /VBLK = L になるまで待つ
    jp      m,  INPUT_MZ1X03_VBLK_SYNC11

    // -------- 軸サンプリング
    ld      L,  #((MMIO_ETC + 1) & 0xff)  // 7

INPUT_MZ1X03_SAMPLE_LOOP:
    dec     L                       // 4
    ldi                             // 16
    nop                             // 4
    nop                             // 4        小計28

    dec     L                       // 4
    ldi                             // 16
    nop                             // 4
    nop                             // 4        小計28

    dec     HL                      // 6
    ldi                             // 16
    nop                             // 4        小計26

    dec     L                       // 4
    ldi                             // 16       BC!=0 ならば P/V=1(PE)
    jp      pe, INPUT_MZ1X03_SAMPLE_LOOP// 10/10    小計30

    ret
__endasm;
}


/** シャープ純正ジョイスティックの簡易入力版です
 * - VBLANK を待ち, 更に 63 ライン分 (約4msec) かかります
 * @return 00UD_RLBA. 未検出なら INPUT_MASK_NC が立ちます
 */
static u8 inputMZ1X03Simple_() __z88dk_fastcall __naked
{
__asm
    // -------- /VBLK の立下がりを待つ
    xor     A
    ld      HL, #MMIO_8255_PORTC
INPUT_MZ1X03_VBLK_SYNC00:
    or      A, (HL)                 // /VBLK = H になるまで待つ
    jp      p,  INPUT_MZ1X03_VBLK_SYNC00
INPUT_MZ1X03_VBLK_SYNC01:
    and     A, (HL)                 // /VBLK = L になるまで待つ
    jp      m,  INPUT_MZ1X03_VBLK_SYNC01

    // -------- /VBLK 直後 150 くらいで '0' が読めなかったら未検出扱い
    ld      L,  #(MMIO_ETC & 0xff)  // 7

    ld      B,  #10                 // 7
INPUT_MZ1X03_WAIT:
    djnz    B,  INPUT_MZ1X03_WAIT   // 13 * 10 - 5
    ld      A, (HL)                 // 7
    and     A,  #(MMIO_ETC_JA1_MASK | MMIO_ETC_JA2_MASK)// 7
    jp      nz, INPUT_MZ1X03_NOT_DETECT                 // 10 接続されてないなら '1' が読める筈
    // 小計 7+7+(13*10-5)+7+7+10 = 163

    // -------- /VBLK 直後 300 で '1' ならば左 or 上
    ld      B,  #11                 // 7
INPUT_MZ1X03_WAIT1:
    djnz    B,  INPUT_MZ1X03_WAIT1  // 13 * 11 - 5
    ld      A, (HL)                 // 7    ****_*YX*
    // 小計 7+(13*11-5)+7 = 157

    // -------- /VBLK ここから更に 7000 くらいで '0' ならば右 or 下
    ld      E,  #7                  // 7
INPUT_MZ1X03_WAIT2:
    ld      B,  #76                 // 7
INPUT_MZ1X03_WAIT3:
    djnz    B,  INPUT_MZ1X03_WAIT3  // 13 * 76 - 5
    dec     E                       // 4
    jp      NZ, INPUT_MZ1X03_WAIT2  // 10
    ld      D, (HL)                 // 7    ****_*yx*
    // 小計 7+((7+13*76-5)+4+10)*4 = 7035

    // -------- 上下左右判定
INPUT_MZ1X03:
    and     A,  #(MMIO_ETC_JA1_MASK | MMIO_ETC_JA2_MASK)
    rrca                            //      0000_00YX
    ld      E, A
    ld      A, D                    //      ****_*yx*
    and     A,  #(MMIO_ETC_JA1_MASK | MMIO_ETC_JA2_MASK)
    add     A, A                    //      0000_yx00
    or      A, E                    //      0000_yxYX
section rodata_compiler
INPUT_MZ1X03_TAB:
    db      INPUT_MASK_R | INPUT_MASK_D, INPUT_MASK_NC,               INPUT_MASK_NC              , INPUT_MASK_NC               // 0000, 0001, 0010, 0011
    db                     INPUT_MASK_D, INPUT_MASK_L | INPUT_MASK_D, INPUT_MASK_NC              , INPUT_MASK_NC               // 0100, 0101, 0110, 0111
    db      INPUT_MASK_R,                INPUT_MASK_NC,               INPUT_MASK_R | INPUT_MASK_U, INPUT_MASK_NC               // 1000, 1001, 1010, 1011
    db      0,                           INPUT_MASK_L,                INPUT_MASK_U,                INPUT_MASK_L | INPUT_MASK_U // 1100, 1101, 1110, 1111
section code_compiler
    ld      HL, INPUT_MZ1X03_TAB
    ld      D,  #0x00
    ld      E,  A
    add     HL, DE
    ld      E,  (HL)                //      00UD_RL00
    bit     #INPUT_SHIFT_NC, E
    jp      nz, INPUT_MZ1X03_NOT_DETECT // 未検出 (違うプロトコルのジョイスティックが繋がってる可能性)

    // -------- /VBLK 外になるまで待つ
    ld      HL, #MMIO_8255_PORTC
INPUT_MZ1X03_VBLK_SYNC2:
    bit     #MMIO_8255_PORTC_VBLK_SHIFT, (HL)// /VBLK = H になるまで待つ
    jp      Z,  INPUT_MZ1X03_VBLK_SYNC2

    // -------- 少し待ってからボタンを読む
    ld      B,  200
INPUT_MZ1X03_WAIT4:
    djnz    B,  INPUT_MZ1X03_WAIT4

    ld      L,  #(MMIO_ETC & 0xff)
    ld      A,  (HL)                //      ****_**BA*
    cpl     A                       // ビット反転
    and     A,  #(MMIO_ETC_JA1_MASK | MMIO_ETC_JA2_MASK)
    rrca                            //      0000_00BA
    or      A,  E                   //      00UD_RLBA
    ld      L,  A
    ret

    // -------- 未検出
INPUT_MZ1X03_NOT_DETECT:
    ld      L, #INPUT_MASK_NC
    ret

__endasm;
}


/** 手抜き print */
static void print_(volatile u8* addr, const u8* str)
{
    for (u8 s; (s = *str) != 0; str++)  {
        *addr++ = s;
    }
}


/** 手抜き 16進数表示 */
static void printHex_(volatile u8* addr, const u8 val)
{
    u8 v = val >> 4;
    *addr++ = (10 <= v) ? (v - 10 + DC_A) : (v + DC_0);
    v = val & 0x0f;
    *addr   = (10 <= v) ? (v - 10 + DC_A) : (v + DC_0);
}


/** 画面クリア */
static void clearScreen_()
{
    volatile u8* vt = (volatile u8*)VRAM_TEXT;
    volatile u8* va = (volatile u8*)VRAM_ATB;
    volatile u8* va1 = (volatile u8*)VRAM_ATB_JA1;
    volatile u8* va2 = (volatile u8*)VRAM_ATB_JA2;
    for (int i = 0; i < 1000; ++i) {
        vt[i] = 0x00;
        va[i] = 0x70;
    }
    // 最初の 10 x 28 T states の色
    for (int i = 0; i < 10/2; ++i) {
        va1[i] = 0x40;
        va2[i] = 0x40;
    }
    // (10 + 256) x 28 T states より後ろの色
    for (int i = 266/2; i < INPUT_AXIS_NR_SAMPLES/2; ++i) {
        va1[i] = 0x40;
        va2[i] = 0x40;
    }
    // /VBLK 外(NTSC) (228 は 1ラインの T states) の色
    for (int i = 62 * 228 / 28 / 2; i < INPUT_AXIS_NR_SAMPLES / 2; ++i) {
        va1[i] = 0x30;
        va2[i] = 0x30;
    }
    // 10 x 28 T states 単位で読みやすいように背景色を決める
    for (int i = 0; i < INPUT_AXIS_NR_SAMPLES / 2; ++i) {
        int b = (i / 5) & 1;
        va1[i] |= b;
        va2[i] |= b;
    }

    static const u8 str_ja1[] = { DC_J, DC_A, DC_1, DC_COLON, 0 };
    static const u8 str_ja2[] = { DC_J, DC_A, DC_2, DC_COLON, 0 };
    static const u8 str_detect[] = { DC_D, DC_E, DC_T, DC_E, DC_C, DC_T, DC_COLON, 0 };
    print_((volatile u8*)(VRAM_TEXT_JA1    - 40), str_ja1);
    print_((volatile u8*)(VRAM_TEXT_JA2    - 40), str_ja2);
    print_((volatile u8*)(VRAM_TEXT_DETECT - 40), str_detect);
}


/** 軸サンプリングデータ表示 */
static void dispAxis_()
{
    inputMZ1X03Axis_();
    volatile u8* v1 = (volatile u8*)VRAM_TEXT_JA1;
    volatile u8* v2 = (volatile u8*)VRAM_TEXT_JA2;
    u8* a = axis_data;
    for (int i = 0; i < INPUT_AXIS_NR_SAMPLES; i += 2) {
        u8 d1 = *a++;
        u8 d2 = *a++;
        u8 x = ((d1 & MMIO_ETC_JA1_MASK) ? 0xf1 : 0xf0) | ((d2 & MMIO_ETC_JA1_MASK) ? 0x02 : 0x00);
        u8 y = ((d1 & MMIO_ETC_JA2_MASK) ? 0xf1 : 0xf0) | ((d2 & MMIO_ETC_JA2_MASK) ? 0x02 : 0x00);
        *v1++ = x;
        *v2++ = y;
    }

    int v = 0;
    a = axis_data + 10;
    for (int i = 0; i < 255; i++) {
        if (*a++ & MMIO_ETC_JA1_MASK) {
            break;
        }
        v++;
    }
    printHex_((volatile u8*)(VRAM_TEXT_JA1 - 40 + 4), v);

    v = 0;
    a = axis_data + 10;
    for (int i = 0; i < 255; i++) {
        if (*a++ & MMIO_ETC_JA2_MASK) {
            break;
        }
        v++;
    }
    printHex_((volatile u8*)(VRAM_TEXT_JA2 - 40 + 4), v);
}

/** ボタン&簡易表示 */
static void dispInput_()
{
    u8           joy = inputMZ1X03Simple_();
    volatile u8* v   = (volatile u8*)VRAM_TEXT_DETECT;
    if (joy == INPUT_MASK_NC) {
        for (int i = 6; i != 0; --i) {
            *v++ = 0x6d; // 'X'
        }
    } else {
        *v++ = (joy & INPUT_MASK_U) ? DC_CURSOR_UP    : DC_MINUS;
        *v++ = (joy & INPUT_MASK_D) ? DC_CURSOR_DOWN  : DC_MINUS;
        *v++ = (joy & INPUT_MASK_R) ? DC_CURSOR_RIGHT : DC_MINUS;
        *v++ = (joy & INPUT_MASK_L) ? DC_CURSOR_LEFT  : DC_MINUS;
        *v++ = (joy & INPUT_MASK_B) ? DC_B            : DC_MINUS;
        *v   = (joy & INPUT_MASK_A) ? DC_A            : DC_MINUS;
    }
}


void main() __naked
{
    clearScreen_();
    // -------- メインループ
    while (1) {
        dispAxis_();
        dispInput_();
    }
}
