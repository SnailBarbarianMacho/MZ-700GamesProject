/**
 * サウンド
 *
 * @author Snail Barbarian Macho (NWK)
 */

#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "sound.h"

#define ADDR_SD3_ATT_TAB 0x0000
#define SZ_SD3_ATT_TAB   0x0040

// -------------------------------- システム
void soundInit() __z88dk_fastcall
{
    STATIC_ASSERT(SD3_C2 <= 255.0, SOUND_LENGTH_OVERFLOW); // 低音のループが 255 を越える
    STATIC_ASSERT(SD3_G4 >  30.0,  SOUND_LENGTH_UNDERLOW); // 高音のループが 30  以下

__asm
    // -------- 減衰テーブルの作成 -アドレスは 0x100 単位に作ってアクセスしやすいようにします
    // 減衰テーブルのコピー
    ld      HL, _sSound3AttTab
    ld      DE, #ADDR_SD3_ATT_TAB
    ld      BC, #SZ_SD3_ATT_TAB
    ldir
    // 残りはテーブル最後の値で埋める
    ld      E, #SZ_SD3_ATT_TAB - 1
    ld      A, (DE)
    inc     E
    ld      B, #(0x100 - SZ_SD3_ATT_TAB)
SOUND_INIT_ATT_TAB:
    ld      (DE), A
    inc     E
    djnz    B, SOUND_INIT_ATT_TAB
__endasm;
}


#pragma disable_warning 85
#pragma save
void sdSetEnabled(const bool bEnabled) __z88dk_fastcall __naked
{
    // (0xe008) = 0x00 or 0x01
__asm
    // ---------------- 準備
    BANK_VRAM_IO               // バンク切替

    // ---------------- 制御
    ld      A, L
    or      A
    jp      z, SOUND_SET_ENABLED
    ld      A, #MMIO_ETC_GATE_MASK
SOUND_SET_ENABLED:
    ld      HL, #MMIO_ETC
    ld      (HL), A

    // ---------------- 後始末
    BANK_RAM                    // バンク切替
    ret
__endasm;
}
#pragma restore


// -------------------------------- 三重和音サウンド
// 一時的に使うワークエリア
#define ADDR_TMP_SP     0x100 + 10  // スタック ポインタの場所

static const u8 sSound3AttTab[] = {
    5,15,13,12,  11,10,9, 8,  8, 7, 7, 6,  6, 5, 5, 5,
    4, 4, 4, 4,  3, 3, 3, 3,  3, 3, 3, 3,  2, 2, 2, 2,
    2, 2, 2, 2,  2, 2, 2, 2,  1, 1, 1, 1,  1, 1, 1, 1,
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
};

#pragma disable_warning 85
#pragma save
void sd3Play(const u8* mml0, const u8* mml1, const u8* mml2) __naked
{
    //                        <------->減衰 att
    // --+                    +-------+                    +-------+
    //   |                    |       |                    |       |
    //   +--------------------+       +--------------------+       +----- ... -----
    //   <----------------------------> 波長 wave_len (8bit)
    //   <-------------------- 音長 sd_len (16bit) ---------------------- ... ----->

    // -------------------------------- 初期化マクロ
    // スタックより MML を読みだす
    // @param sd_ct_reg_h, sd_ct_reg_l: HL'  BC'  DE' 音長カウンタ(減衰テーブル検索の為に, インクリメント カウンタになってます)
    // @param wave_ct_reg:              B    D    E   波長カウンタ(デクリメント カウンタ)
    // @param LABEL_SD_SD_LEN                         音長の上位 8bit
    // @param LABEL_SD_MML                            MML ポインタ(自己書換)
    // @param LABEL_SD_WAVE_CT                        波長カウンタ(自己書換)
#define SD3_INIT(sd_ct_reg_h, sd_ct_reg_l, sd_ct_reg_hl, wave_ct_reg, \
    LABEL_SD_SD_LEN, LABEL_SD_MML, LABEL_SD_WAVE_CT) \
    pop     HL                          /* MML ポインタ             */ \
    ; ---- 波長初期値, カウンタ         /*                          */ \
    ld      A, (HL)                     /*                          */ \
    ld      (LABEL_SD_WAVE_CT + 1), A   /* 波長カウンタ    自己書換 */ \
    ld      wave_ct_reg, A              /* 波長カウンタ             */ \
    inc     HL                          /*                          */ \
    ; ---- 音長初期値                   /*                          */ \
    ld      A, (HL)                     /*                          */ \
    ld      (LABEL_SD_SD_LEN + 1), A    /* 音長の上位 8bit 自己書換 */ \
    inc     HL                          /*                          */ \
    ld      (LABEL_SD_MML    + 1), HL   /* MML ポインタ    自己書換 */ \
    exx                                 /*                          */ \
        ld  sd_ct_reg_hl, 0x0000        /* 音長カウンタ初期化       */ \
    exx                                 /*                          */

    // -------------------------------- 波形生成と合成マクロ
    // @param sd_ct_reg_h, sd_ct_reg_l  HL'  BC'  DE'   音長カウンタ(減衰テーブル検索の為に, インクリメント カウンタになってます)
    // @param wave_ct_reg               B    D    E     波長カウンタ(デクリメント カウンタ)
    // @param LABEL_SD_SD_LEN                           音長の上位 8bit
    // @param LABEL_SD_MML                              MML ポインタ(自己書換)
    // @param LABEL_SD_WAVE_CT                          波長カウンタ(自己書換)
    // 暗黙:                            C               パルス値 MMIO_8253_CH0_MODE0 or MMIO_8253_CH0_MODE3
    // 暗黙:                            H               ADDR_SD3_ATT_TAB の上位 8 bit
    // 破壊:                            A, L
    // 未使用:                          IX, IY, I, A'
#define SD3_MAIN(sd_ct_reg_h, sd_ct_reg_l, sd_ct_reg_hl, wave_ct_reg, \
    LABEL_SD_SD_LEN, LABEL_SD_MML, LABEL_SD_LEN_END, LABEL_SD_PULSE_END, LABEL_SD_WAVE_CT, LABEL_SD_END) \
    /* ---------------- 次の MML を読む */                                             \
    exx                                         /*  4 |                             */ \
        inc sd_ct_reg_hl                        /*  6 |  音長カウンタ               */ \
        ld  A,  sd_ct_reg_h                     /*  4 |  音長カウンタ上位 8bit      */ \
LABEL_SD_SD_LEN:                                                                       \
        cp  A,  0x00                            /*  7 |  音長の上位 8bit 自己書換   */ \
        jp  nz, LABEL_SD_LEN_END                /* 10 +-計31                        */ \
LABEL_SD_MML:                                                                          \
            ld  sd_ct_reg_hl, 0x0000            /* 10    MML ポインタ       自己書換*/ \
            /* ---- 波長                                                            */ \
            ld  A, (sd_ct_reg_hl)               /*  7    波長初期値                 */ \
            and A                               /*  4                               */ \
            jp  z, SD3_END                      /* 10    if (A == 0) goto end       */ \
            ld (LABEL_SD_WAVE_CT + 1), A        /* 13    波長カウンタ初期値 自己書換*/ \
            inc sd_ct_reg_hl                    /*  6                               */ \
            /* ---- 音長                                                            */ \
            ld  A, (sd_ct_reg_hl)               /*  7    音長の上位 8bit            */ \
            inc sd_ct_reg_hl                    /*  6                               */ \
            ld  (LABEL_SD_MML + 1), sd_ct_reg_hl/* 16/20 MML ポインタ       自己書換*/ \
            ld  (LABEL_SD_SD_LEN + 1), A        /* 13    音長の上位 8bit    自己書換*/ \
            ld  sd_ct_reg_hl, 0x0000            /*  8                               */ \
                                                /* 波長カウンタ(B,D,E)は初期化しなくてもOK */ \
            ld  A, sd_ct_reg_h                  /*  4    音長カウンタ上位 8bit      */ \
LABEL_SD_LEN_END:                                                                      \
    /* ---------------- パルス生成                                                  */ \
    exx                                         /*  4 |                             */ \
    /* ---- if (sound3AttTab[A] - wave_ct_reg >= 0) { C = 波形 '1'; }               */ \
    ld      L, A                                /*  4 | HL = sound3 att table       */ \
    ld      A, (HL)                             /*  7 |                             */ \
    cp      A, wave_ct_reg                      /*  4 |                             */ \
    jp      c, LABEL_SD_PULSE_END               /* 10 +-計29                        */ \
        ld  C, MMIO_8253_CH0_MODE3               /*  7                               */ \
LABEL_SD_PULSE_END:                                                                    \
    /* ---------------- 波長ループの終わり                                          */ \
    dec     wave_ct_reg                         /*  4 |                             */ \
    jp      nz, LABEL_SD_END                    /* 10 +- 計14                       */ \
LABEL_SD_WAVE_CT:                                                                      \
        ld  wave_ct_reg, 0x00                   /*  7   波長カウンタ        自己書換*/ \
LABEL_SD_END:


__asm
    // -------------------------------- 初期化
    ld      (SD3_SP_RESTORE + 1), SP// SP を保存(自己書換)
    pop     HL                      // リターン アドレス(捨てる)

    SD3_INIT(H, L, HL, B, SD3_SD_LEN0, SD3_MML0, SD3_WAVE_CT0)
    SD3_INIT(B, C, BC, D, SD3_SD_LEN1, SD3_MML1, SD3_WAVE_CT1)
    SD3_INIT(D, E, DE, E, SD3_SD_LEN2, SD3_MML2, SD3_WAVE_CT2)

    // バンクを切り替えて, 8253 にアクセスするようにします
    ld      C, 0xe3
    out     (C), A  // 値はなんでもいい

    ld      H, ADDR_SD3_ATT_TAB >> 8 // 減衰テーブル
    // -------------------------------- 波形合成&出力ループ
SD3_LOOP:
    ld      C, #MMIO_8253_CH0_MODE0  // 7
    SD3_MAIN(H, L, HL, B, SD3_SD_LEN0, SD3_MML0, SD3_LEN_END0, SD3_PULSE_END0, SD3_WAVE_CT0, SD3_END0)
    SD3_MAIN(B, C, BC, D, SD3_SD_LEN1, SD3_MML1, SD3_LEN_END1, SD3_PULSE_END1, SD3_WAVE_CT1, SD3_END1)
    SD3_MAIN(D, E, DE, E, SD3_SD_LEN2, SD3_MML2, SD3_LEN_END2, SD3_PULSE_END2, SD3_WAVE_CT2, SD3_END2)
SD3_LOOP_END:
    // ---------------- 波形出力
    ld      A, C                    //  4 |
    ld      (#MMIO_8253_CTRL), A     // 13 |
    jp      SD3_LOOP                // 10 +- 計 17

    // ループ内の Tステート数(周波数)
    // SD3_MAIN 内最短: 7 + (31+29+14)*3 + 17 = 246 (14500Hz)

SD3_END:
    // -------------------------------- 後始末
    // 8253 を元の設定に戻します
    ld      A, MMIO_8253_CH0_MODE3
    ld      (#MMIO_8253_CTRL), A

    // バンクを切り替えて, 8253 を切り離します
    ld      C, 0xe1
    out     (C), A  // 値はなんでもいい

SD3_SP_RESTORE:
    ld      SP, #0x0000             // SP を復帰
    ret
__endasm;
}



#pragma restore