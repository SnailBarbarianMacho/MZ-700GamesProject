/**
 * サウンド
 *
 * @author Snail Barbarian Macho (NWK)
 */

#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "sound.h"

#define ADDR_SD3_ATT_TAB 0x0000

// -------------------------------- システム
void soundInit() __z88dk_fastcall
{
__asm
    // -------- 減衰テーブルの作成 -アドレスは 0x100 単位に作ってアクセスしやすいようにします
    // 減衰テーブルのコピー
    ld      HL, _sSound3AttTab
    ld      DE, ADDR_SD3_ATT_TAB
    ld      BC, 0x40
    ldir
    // 残りは 0x01 で埋める
    ld      A,  0x01
    ld      B,  0xc0
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
    ld      A, #MIO_ETC_GATE_MASK
SOUND_SET_ENABLED:
    ld      HL, #MIO_ETC
    ld      (HL), A

    // ---------------- 後始末
    BANK_RAM                    // バンク切替
    ret
__endasm;
}
#pragma restore


// -------------------------------- 三重和音サウンド
static u16* spMml0;     // サウンド データへのポインタ
static u16* spMml1;
static u16* spMml2;
static u8   sSdLen0;    // 音長カウンタ初期値(8bit)
static u8   sSdLen1;
static u8   sSdLen2;
static u8   sWaveLen0;  // 波長初期値(8bit)
static u8   sWaveLen1;
static u8   sWaveLen2;

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
    // 初期化
    // HL' BC' DE' = 音長カウンタ = 0x0000 (マクロの外で設定)
    // B D E       = 波長カウンタ
#define SD3_INIT(pMml, waveLen, sdLen, waveLenCtReg) \
    pop     HL                      ; HL = MML\
    ; ---- 波長初期値, カウンタ\
    ld      A, (HL)                 ; A = 波長\
    ld      (waveLen), A            ; 波長初期値\
    ld      waveLenCtReg, A         ; 波長カウンタ\
    inc     HL                      \
    ; ---- 音長初期値\
    ld      A, (HL)                 ; A = 音長\
    ld      (sdLen), A              \
    inc     HL                      \
    ; ---- MML ポインタ\
    ld      (pMml), HL              ; HL = MML

    // メイン
    // HL' BC' DE' = 音長カウンタ = 0x0000 (マクロの外で設定)
    // B D E       = 波長カウンタ
    // C = 8253 出力データ(MIO_8253_CH0_MODE0 or MIO_8253_CH0_MODE3)
    // H = ADDR_SD3_ATT_TAB の上位 8 bit
    // L 破壊
#define SD3_MAIN(pMml, waveLen, sdLen, sdLenRegH, sdLenRegL, waveLenCtReg, pushHL, popHL, LABEL_SD_LEN_END, LABEL_SD_WAVE_PULSE, LABEL_SD_WAVE_LEN, LABEL_SD_END) \
    ; ---------------- sound length 音長 \
    ; ---- 音長カウンタ -- \
    exx                             ;  4\
        inc sdLenRegH##sdLenRegL    ;  6\
        ld  A, (sdLen)              ; 13\
        cp  A, sdLenRegH            ;  4\
        jp  nz, LABEL_SD_LEN_END    ; 10 if (sound length == 0) { read next MML }\
            pushHL                  ; 11 / 0\
            ld  HL, (pMml)          ; 16\
    ;---- 波長初期値\
            ld  A, (HL)             ;  7 A = 波長初期値\
            and A                   ;  4\
            jp  z, SD3_END       ; 10 if (A == 0) goto end \
            ld  (waveLen), A        ; 13\
            inc HL                  ;  4\
    ;---- 音長初期値\
            ld  A, (HL)             ;  7 A = 音長初期値\
            ld  (sdLen), A          ; 13\
            inc HL                  ;  6\
    ;---- MML\
            ld  (pMml), HL          ; 16 HL = MML\
            popHL                   ; 10\
    ;---- 音長カウンタ\
            ld sdLenRegH##sdLenRegL, 0x0000; 10\
    ;---- 波長カウンタは初期化しません\
    exx                             ;  4\
    jp      LABEL_SD_END            ; 10 / 0\
    ;\
LABEL_SD_LEN_END:\
        ld  A, sdLenRegH            ;  4 音長カウンタ 上位 8bit\
    exx                             ;  4\
    ; ---------------- wave length 波長 \
    ; ---- if (waveLenReg <= sound3AttTab[A]) { C = MIO_8253_CH0_MODE3; }\
    ld      L, A                    ;  4 HL = sound3 att table\
    ld      A, (HL);                ;  7 offset is self-modify\
    cp      A, waveLenCtReg         ;  4\
    jp      c, LABEL_SD_WAVE_PULSE  ; 10  (if A > WaveLenCtReg) .. \
        ld  C, MIO_8253_CH0_MODE3   ;  7\
;\
LABEL_SD_WAVE_PULSE:\
    ; ---- wave length Loop\
    dec     waveLenCtReg            ;  4\
    jp      nz, LABEL_SD_WAVE_LEN   ; 10\
        ld  A, (waveLen)            ; 13\
        ld  waveLenCtReg, A         ;  4\
    ;\
LABEL_SD_WAVE_LEN:\
    ;\
LABEL_SD_END:

// CPU クロックの計算                     loop start
//                                          |7
//                                        -----
//                                          |37
//                +-------------------------+
// sound len == 0 |                         |33
//                |                  +------+
//                |    wave len < 10 |7     |
//                |                  +------+
//             131|                         |14
//                |                  +------+
//                |    wave len == 0 |17    |
//                |                  +------+
//                |                         |
//                +-------------------------+
//                                          |
//                                        -----
//                                          |27
//                                       loop end
// 最短の場合の時間
// (減衰なし) 40 + (46+14+13)*3 + 27 = 286 clocks
// (減衰あり)  7 + (37+33+14)*3 + 27 = 286 clocks

__asm
    // ---------------- 初期化
    ld      (SD3_SP_RESTORE + 1), SP// SP を保存(自己書換)
    pop     HL                      // リターン アドレス(捨てる)

    SD3_INIT(_spMml0, _sWaveLen0, _sSdLen0, B)
    SD3_INIT(_spMml1, _sWaveLen1, _sSdLen1, D)
    SD3_INIT(_spMml2, _sWaveLen2, _sSdLen2, E)
    ld      SP, #ADDR_TMP_SP
    // 音長カウンタの初期化
    exx
        ld  HL, 0x0000  // HL' 音長カウンタ 0 = 0x0000
        ld  BC, HL      // BC' 音長カウンタ 1 = 0x0000
        ld  DE, HL      // DE' 音長カウンタ 2 = 0x0000
    exx

    // バンクを切り替えて, 8253 にアクセスするようにします
    ld      C, 0xe3
    out     (C), A  // 値はなんでもいい

    // ---------------- 波形合成
    ld      H, ADDR_SD3_ATT_TAB >> 8// 減衰テーブル
SD3_LOOP:
    ld      C, #MIO_8253_CH0_MODE0  // 7
    SD3_MAIN(_spMml0, _sWaveLen0, _sSdLen0, H, L, B,        ,       , SD3_SD_LEN_END0, SD3_SD_WAVE_PULSE0, SD3_SD_WAVE_LEN0, SD3_SD_END0)
    SD3_MAIN(_spMml1, _sWaveLen1, _sSdLen1, B, C, D, push HL, pop HL, SD3_SD_LEN_END1, SD3_SD_WAVE_PULSE1, SD3_SD_WAVE_LEN1, SD3_SD_END1)
    SD3_MAIN(_spMml2, _sWaveLen2, _sSdLen2, D, E, E, push HL, pop HL, SD3_SD_LEN_END2, SD3_SD_WAVE_PULSE2, SD3_SD_WAVE_LEN2, SD3_SD_END2)

SD3_LOOP_END:
    // ---------------- 波形出力
    ld      A, C                    // 4
    ld      (#MIO_8253_CTRL), A     // 13
    jp      SD3_LOOP                // 10

SD3_END:
    // ---------------- 後始末
    // 8253 を元の設定に戻します
    ld      A, MIO_8253_CH0_MODE3
    ld      (#MIO_8253_CTRL), A

    // バンクを切り替えて, 8253 を切り離します
    ld      C, 0xe1
    out     (C), A  // 値はなんでもいい

SD3_SP_RESTORE:
    ld      SP, #0x0000             // SP を復帰
    ret
__endasm;
}
#pragma restore