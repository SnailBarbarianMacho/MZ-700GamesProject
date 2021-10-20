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
    ld      (LOGO_TRANS_SP_RESTORE + 1), SP// SP を保存(自己書換)
    BANK_VRAM_IO                        // バンク切替
    dec     L                           // -1/0 = 表示しない/表示する
    jp      z, LOGO_TRANS_DRAW

    // -------------------------------- 表示しない(0x43で表示する)
    // -------- 転送準備
    ld      SP, #(VRAM_TEXT_ADDR(LOGO_POS_X, LOGO_POS_Y) + 4)
    ld      B, LOGO_HEIGHT              // outer loop
    exx
        ld      DE, 0x4343              // DE = 0x4343 表示しない時の文字
        //ld      DE, 0xffff// TEST
        ld      HL, DE                  // HL = 0x4343
    exx

    // -------- VBLANK を待ちます
    ld      HL, #MIO_8255_PORTC         // VBLANK 待ち用 8255 ポート C
    ld      A, H                        // 最上位 bit を立てる
LOGO_TRANS_ERASE_VBLANK_1:              // V表示中 ならばループ
    and     A, (HL)
    jp      m, LOGO_TRANS_ERASE_VBLANK_1
LOGO_TRANS_ERASE_VBLANK_0:              // VBLANK ならばループ
    or      A, (HL)                     // 7
    jp      p, LOGO_TRANS_ERASE_VBLANK_0// 10/10
    // この時点で 0 ライン目は既に H表示中で 10～27 cycles 経過

    // -------- 消去
    // 消すときはラスタのことは考えないでいいので気楽に処理します
LOGO_TRANS_ERASE_LOOP:
    exx                                 // 4
      push  HL                          // 11 計11 次の HBLANK を待つ
      push  DE                          // 11 計22
    exx                                 // 4  計26
    ld      HL, #(VRAM_WIDTH + 4)       // 10 計46 1行下へ移動
    add     HL, SP                      // 11 計57
    ld      SP, HL                      // 6  計63 HBLANK を抜ける
    djnz    B, LOGO_TRANS_ERASE_LOOP    // 13/8
    jp      LOGO_TRANS_END

    // -------------------------------- 表示する
LOGO_TRANS_DRAW:
    // -------- 転送準備
    ld      HL, #_imageData
    ld      (#ADDR_TMP_SRC), HL
    ld      HL, #(VRAM_TEXT_ADDR(LOGO_POS_X, LOGO_POS_Y) + 4)
    ld      (#ADDR_TMP_DST), HL
    ld      B, LOGO_HEIGHT              // outer loop
    exx
      ld    B, 8                        // inner loop
      ld    SP, (#ADDR_TMP_SRC)
      pop   DE
      pop   HL
      ld    (#ADDR_TMP_SRC), SP
      ld    SP, (#ADDR_TMP_DST)
    exx

    // -------- VBLANK を待ちます
    ld      HL, #MIO_8255_PORTC         // VBLANK 待ち用 8255 ポート C
    ld      A, H                        // 最上位 bit を立てる
LOGO_TRANS_DRAW_VBLANK_1:               // V表示中 ならばループ
    and     A, (HL)
    jp      m, LOGO_TRANS_DRAW_VBLANK_1
LOGO_TRANS_DRAW_VBLANK_0:               // VBLANK ならばループ
    or      A, (HL)                     // 7
    jp      p, LOGO_TRANS_DRAW_VBLANK_0 // 10/10
    // この時点で 0 ライン目は既に H表示中で 10～27 cycles 経過
    // ※VBLANK 立ち上がり時の HBLANK に注意
    // - 実機:     立下がって HBLANK が始まる
    // - EmuZ-700: 立ち上がって HBLANK が終わる

    // -------- ライン待ち
    ld      H, #(LOGO_POS_Y * 8)        // ライン待ちループ
    jr      LOGO_TRANS_DRAW_LINE0 + 3   // 最初の 1 ラインは VRAM にはアクセスしない(EmuZ-700 対策)

LOGO_TRANS_DRAW_LINE0:
      ld    A, (#VRAM_TEXT)             //  13 次の HBLANK まで待つ
      ld    E, 10                       //  7            計7
LOGO_TRANS_DRAW_LINE1:
        dec E                           //  4 * 10 =  40 計47
        jp  nz, LOGO_TRANS_DRAW_LINE1   // 10 * 10 = 100 計147 HBLANK を抜ける
      dec   H                           // 4             計151
      jp    nz, LOGO_TRANS_DRAW_LINE0   // 10            計161
    // この時点で LOGO_POS_Y * 8 ライン目は H表示中

    // -------- 表示します
LOGO_TRANS_DRAW_LOOP0:
    exx                                 // 4
LOGO_TRANS_DRAW_LOOP1:
      push  HL                          // 11 計11 次の HBLANK を待つ
      // 間に 52 clock 迄の処理なら崩れない
      push  DE                          // 11 計22
      ld    SP, (#ADDR_TMP_SRC)         // 20 計42
      pop   DE                          // 10 計52
      pop   HL                          // 10 計62
      ld    (#ADDR_TMP_SRC), SP         // 20 計82 HBLANK を抜ける
      ld    SP, (#ADDR_TMP_DST)         // 20      転送先アドレスを戻す
      djnz  B, LOGO_TRANS_DRAW_LOOP1    // 13/8
      ld    B, 8                        //  7
    exx                                 //  4
    ld      HL, VRAM_WIDTH              // 10      転送先アドレスを1行下へ移動
    add     HL, SP                      // 11
    ld      (#ADDR_TMP_DST), HL         // 16
    ld      SP, HL                      //  6
    djnz    B, LOGO_TRANS_DRAW_LOOP0    // 13/8

    // ---------------- 後始末
LOGO_TRANS_END:
    BANK_RAM                    // バンク切替
LOGO_TRANS_SP_RESTORE:
    ld      SP, #0x0000         // SP を復帰
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
    sysSetStepCounter(500);
    sdSetEnabled(true);

    // サウンドの初期化はここではやらない. 一瞬音がなってしまうので
    //sdMake(P2I14(0, 0x0000));
    //sdSetEnabled(true);    // これをやると一瞬音が鳴ってしまう

    sFreq = 10;
}

// ---------------------------------------------------------------- メイン
static void wait()
{
    __asm
    ld      B, #0
WAIT_LOOP:
    ex      (SP), HL    // とにかく, ちょっと時間のかかる命令をウエイトに入れておく
    ex      (SP), HL
    djnz    B, WAIT_LOOP
    __endasm;
}


void stepLogoMain(u16 stepCounter)
{
    logoTrans((100 < stepCounter) && (stepCounter < 400));

    sdMake(0x0000);

    vramSetTransDisabled();
    starsSetDisabled();
    scoreSetDisabled();

    if (stepCounter == 0) {
        sysSetStep(stepTitleDemoInit, stepTitleDemoMain);
    }
    wait();

    if ((stepCounter < 400) && (100 <= stepCounter)) {
        sdMake(sFreq);
        sFreq += 60;
    }
}
