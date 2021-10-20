/**
 * ロゴ ステップ
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/sound.h"
#include "../system/obj.h"
#include "../game/stars.h"
#include "../game/score.h"
#include "stepTitleDemo.h"
#include "stepLogo.h"

static const u8 imageData[] = {
    // cos1_inv1_atb1_score41
    0x96, 0xdc, 0x36, 0x36,
    0x37, 0x42, 0x30, 0x30,
    0x32, 0x13, 0x4e, 0x43,
    0x32, 0x64, 0x3f, 0x34,
    0x00, 0x1a, 0x33, 0x1b,
    0x42, 0x09, 0x33, 0x38,
    0x7f, 0x00, 0x33, 0x3a,
    0x7f, 0x3d, 0x37, 0x32,

    0x00, 0x14, 0x37, 0x36,
    0x00, 0xcb, 0x37, 0x30,
    0x32, 0x9e, 0x33, 0x43,
    0x4d, 0x56, 0x3f, 0x34,
    0x1d, 0x56, 0x7f, 0x1b,
    0x4d, 0x8a, 0x38, 0x38,
    0x81, 0x85, 0x3a, 0x3a,
    0x32, 0xc9, 0x32, 0x32,

    0x97, 0x3f, 0x5b, 0x3b,
    0x8d, 0x7f, 0x37, 0x7f,
    0x5b, 0x3f, 0x4a, 0xcc,
    0x4d, 0x33, 0x50, 0xcf,
    0x37, 0x00, 0x06, 0x0b,
    0x32, 0x00, 0x1a, 0x18,
    0x32, 0x00, 0xef, 0x60,
    0x00, 0x00, 0x11, 0xea,

    0x06, 0x8f, 0x0a, 0x0a,
    0x3b, 0x5b, 0x0c, 0x0c,
    0x4e, 0x37, 0xef, 0xef,
    0x00, 0x00, 0xa8, 0x8f,
    0x32, 0x00, 0x37, 0x7f,
    0x37, 0x00, 0x4d, 0x4e,
    0x6c, 0x33, 0x3a, 0x3a,
    0x32, 0x3b, 0x32, 0x32,

    0x36, 0x36, 0x36, 0x36,
    0x33, 0xac, 0x65, 0x47,
    0x00, 0x20, 0x15, 0x51,
    0x00, 0x04, 0x02, 0x3f,
    0x6b, 0x07, 0x10, 0x7f,
    0x41, 0x04, 0x02, 0x3f,
    0x09, 0x10, 0x07, 0x06,
    0xcd, 0x10, 0x11, 0x0a,
};

#define ADDR_TMP_STACK  (VVRAM_TMP_WORK + 0) // スタックを保存するアドレス
#define ADDR_TMP_SRC    (VVRAM_TMP_WORK + 2) // 転送元を保存するアドレス
#define ADDR_TMP_DST    (VVRAM_TMP_WORK + 4) // 転送先を保存するアドレス
#define LOGO_WIDTH  4               // 寸法(chara)
#define LOGO_HEIGHT 5               // 寸法(chara)
#define LOGO_POS_X  18              // 表示位置の X[0, 39](chara)
#define LOGO_POS_Y  6               // 表示位置の Y[1, 22](chara) だけど, 何故か 8 位が限界

// もっと下に表示出来る筈ですが, 上手く行かない. 謎

// -------------------------------- ロゴ転送
/**
 * VBLANK で同期をもってロゴを表示します.
 * @param bDraw == false なら表示しません
 */
#pragma disable_warning 85
#pragma save
void logoTrans(const bool bDraw) __z88dk_fastcall __naked
{
__asm
    // ---------------- 準備
    ld      (#ADDR_TMP_STACK), SP   // スタック ポインタを保存
    BANK_VRAM_IO                // バンク切替

    // ---------------- 転送準備
    ld      D, L                // 0 / 1 = しない/表示する

    ld      HL, #_imageData
    ld      (#ADDR_TMP_SRC), HL
    ld      HL, #(VRAM_TEXT_ADDR(LOGO_POS_X, LOGO_POS_Y) + 4)
    ld      (#ADDR_TMP_DST), HL
    ld      B, LOGO_HEIGHT      // outer loop

    exx
        ld      B, 8            // inner loop
        ld      DE, 0x4343      // 表示しない時の文字
        ld      HL, DE
    exx

    ld      A, D
    or      A
    jp      z, LOGO_INIT_END

    exx
        ld      SP, (#ADDR_TMP_SRC)
        pop     DE
        pop     HL
        ld      (#ADDR_TMP_SRC), SP
        ld      SP, (#ADDR_TMP_DST)
    exx
LOGO_INIT_END:

    // ----------------
    // V-Blank を待ちます. 既に V-Blank 中ならばそのまま行っちゃいます
    // 8255 ポート C bit 7 == 0 ならば, ブランキング中
    ld      HL, #MIO_8255_PORTC // 8255 ポート C
    ld      A, H
WAIT_BLANK_1:                   // V 表示中
    and     (HL)
    jp      m, WAIT_BLANK_1
WAIT_BLANK_0:
    or      (HL)                //
    jp      p, WAIT_BLANK_0     // V ブランキング中
    // この時点で 0 ライン目は既に表示中

    ld      H, LOGO_POS_Y * 8 - 1 +1   // ライン待ちループ
WAIT_LINE:
    ld      A, (VRAM_TEXT)      // 次の H ブランクまで待つ
    // 56 クロック以上待つ
    ld      E, 10               // 7
WAIT_LOOP:
    dec     E                   // 4  * 10 = 40
    jp      nz, WAIT_LOOP       // 10 * 10 = 100
    dec     H                   // 4
    jp      nz, WAIT_LINE       // 10   7 + 40 + 100 + 4 + 10 = 161

    // ---------------- 表示しない
    dec     D                   // 4
    jp      z, LOGO_LOOP0       // 10
LOGO_ERASE_LOOP0:
    exx                         // 4
LOGO_ERASE_LOOP1:
        push    HL              // 11
        push    DE              // 11  4 + 10 + 4 + 11 + 11 = 40 間に合った!
        ld      (#ADDR_TMP_SRC), SP // 20
        ld      (#ADDR_TMP_SRC), SP // 20
        ld      SP, (#ADDR_TMP_DST) // 20
        djnz    B, LOGO_ERASE_LOOP1 // 13/8
        ld      B, 8            // 7
    exx                         // 4
    ld      HL, VRAM_WIDTH      // 10
    add     HL, SP              // 11
    ld      (#ADDR_TMP_DST), HL // 16
    ld      SP, HL              // 6
    djnz    B, LOGO_ERASE_LOOP0 // 13/8
    jp      LOGO_END
    // ---------------- 表示する
    // H ブランク期間(63 クロック以内に描画しないといけない)
LOGO_LOOP0:
    exx                         // 4
LOGO_LOOP1:
        push    HL              // 11
        push    DE              // 11  4 + 10 + 4 + 11 + 11 = 40 間に合った!
        ld      SP, (#ADDR_TMP_SRC) // 20
        pop     DE              // 10
        pop     HL              // 10
        ld      (#ADDR_TMP_SRC), SP // 20
        ld      SP, (#ADDR_TMP_DST) // 20
        djnz    B, LOGO_LOOP1   // 13/8
        ld      B, 8            // 7
    exx                         // 4
    ld      HL, 40              // 10
    add     HL, SP              // 11
    ld      (#ADDR_TMP_DST), HL // 16
    ld      SP, HL              // 6
    djnz    B, LOGO_LOOP0       // 13/8

    // ---------------- 後始末
LOGO_END:
    BANK_RAM                    // バンク切替
    ld      SP, (#ADDR_TMP_STACK)// スタック ポインタを復活
    ret
__endasm;
}
#pragma restore

u16 sFreq;

// ---------------------------------------------------------------- 初期化
void stepLogoInit()
{
#if DEBUG
    scoreSetStepString(nullptr);
#endif
    vramFill(VATB_CODE(0, 0, 0, 0x00));
    static const u8 logoData[] = {
        0x43, 0x43, 0x43, 0x43,
        0x43, 0x43, 0x43, 0x43,
        0x43, 0x43, 0x43, 0x43,
        0x43, 0x43, 0x43, 0x43,
        0x43, 0x43, 0x43, 0x43,

        0x87, 0x87, 0x87, 0x87,
        0x87, 0x87, 0x87, 0x87,
        0x85, 0x85, 0x86, 0x86,
        0x85, 0x85, 0x86, 0x86,
        0x87, 0x87, 0x87, 0x87,
    };
    vramDrawRect((u8*)VRAM_TEXT_ADDR(LOGO_POS_X, LOGO_POS_Y), logoData, W8H8(LOGO_WIDTH, LOGO_HEIGHT));

    vramSetTransDisabled();
    starsSetDisabled();
    scoreSetDisabled();

    objInit();
    sysSetStepCounter(700);
    sdSetEnabled(true);

    // サウンドの初期化はここではやらない. 一瞬音がなってしまうので
    //sdMake(P2I14(0, 0x0000));
    //sdSetEnabled(true);    // これをやると一瞬音が鳴ってしまう

    sFreq = 10;
}

// ---------------------------------------------------------------- メイン
void stepLogoMain(u16 stepCounter)
{
    logoTrans((100 < stepCounter) && (stepCounter < 600));

    vramSetTransDisabled();
    starsSetDisabled();
    scoreSetDisabled();

    if ((stepCounter < 600) && (200 <= stepCounter) && (stepCounter & 1)) {
        sdMake(sFreq);
        sFreq += 60;
    } else {
        sdMake(0x0000);
    }

    if (stepCounter == 0) {
        sysSetStep(stepTitleDemoInit, stepTitleDemoMain);
    }
}
