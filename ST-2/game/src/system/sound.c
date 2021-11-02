/**
 * サウンド
 *
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "addr.h"
#include "vram.h"
#include "math.h"
#include "../game/bgm.h"
#include "../game/se.h"
#include "sound.h"

// ---------------------------------------------------------------- 変数, マクロ
#define NR_SE_SEQUENCERS 4

u16  (*_bgmMain)(u16);                   // サウンド BGM シーケンサ
u16    _bgmCt;                           // サウンド BGM シーケンサ カウンタ
u8     _sePri;                           // サウンド SE 優先順位
void (*_seMains[NR_SE_SEQUENCERS])(u8);  // サウンド SE シーケンサ
u8     _seCts[NR_SE_SEQUENCERS];         // サウンド SE シーケンサ カウンタ

// ---------------------------------------------------------------- システム(初期化)
// -------- sd1 音程テーブル
static const u16 sSdScaleTab[] = {
    0, 0x0000,  // 停止と休符
    SD_MAKE_INTERVAL(SD_FREQ_C2),  SD_MAKE_INTERVAL(SD_FREQ_CS2), SD_MAKE_INTERVAL(SD_FREQ_D2),  SD_MAKE_INTERVAL(SD_FREQ_DS2),
    SD_MAKE_INTERVAL(SD_FREQ_E2),  SD_MAKE_INTERVAL(SD_FREQ_F2),  SD_MAKE_INTERVAL(SD_FREQ_FS2), SD_MAKE_INTERVAL(SD_FREQ_G2),
    SD_MAKE_INTERVAL(SD_FREQ_GS2), SD_MAKE_INTERVAL(SD_FREQ_A2),  SD_MAKE_INTERVAL(SD_FREQ_AS2), SD_MAKE_INTERVAL(SD_FREQ_B2),

    SD_MAKE_INTERVAL(SD_FREQ_C3),  SD_MAKE_INTERVAL(SD_FREQ_CS3), SD_MAKE_INTERVAL(SD_FREQ_D3),  SD_MAKE_INTERVAL(SD_FREQ_DS3),
    SD_MAKE_INTERVAL(SD_FREQ_E3),  SD_MAKE_INTERVAL(SD_FREQ_F3),  SD_MAKE_INTERVAL(SD_FREQ_FS3), SD_MAKE_INTERVAL(SD_FREQ_G3),
    SD_MAKE_INTERVAL(SD_FREQ_GS3), SD_MAKE_INTERVAL(SD_FREQ_A3),  SD_MAKE_INTERVAL(SD_FREQ_AS3), SD_MAKE_INTERVAL(SD_FREQ_B3),

    SD_MAKE_INTERVAL(SD_FREQ_C4),  SD_MAKE_INTERVAL(SD_FREQ_CS4), SD_MAKE_INTERVAL(SD_FREQ_D4),  SD_MAKE_INTERVAL(SD_FREQ_DS4),
    SD_MAKE_INTERVAL(SD_FREQ_E4),  SD_MAKE_INTERVAL(SD_FREQ_F4),  SD_MAKE_INTERVAL(SD_FREQ_FS4), SD_MAKE_INTERVAL(SD_FREQ_G4),
    SD_MAKE_INTERVAL(SD_FREQ_GS4), SD_MAKE_INTERVAL(SD_FREQ_A4),  SD_MAKE_INTERVAL(SD_FREQ_AS4), SD_MAKE_INTERVAL(SD_FREQ_B4),
};

// -------- sd3 減衰テーブル
static const u8 sSd3AttTab[] = {
#if 1
    4,10,15,17,  14,10,9, 8,  8, 7, 7, 6,  6, 5, 5, 5,
    4, 4, 4, 4,  3, 3, 3, 3,  3, 3, 3, 3,  2, 2, 2, 2,
    2, 2, 2, 2,  2, 2, 2, 2,  1, 1, 1, 1,  1, 1, 1, 1,
    1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,
#endif
#if 0 // 立ち上がりが遅くて「ミョンミョン」な音色
    1,3,5,7, 9,11,13,15, 16,17,17,17, 16,16,16,16,
    15,15,14,14, 13,13,12,12, 11,11,11,11, 10,10,10,10,
    9,9,9,9,8,8,8,8,7,7,7,7,6,6,6,6,
    5,5,5,5,4,4,4,4,3,3,3,3,2,2,2,2,
#endif
};

static void sdInitSub() __z88dk_fastcall
{
__asm
    // -------- 減衰テーブルの作成
    ld      HL, _sSdScaleTab
    ld      DE, #ADDR_SD_SCALE_TAB
    ld      BC, #((2 + 12 + 12 + 12) * 2)
    ldir

    // -------- 減衰テーブルの作成
    // 減衰テーブルのコピー
    ld      HL, _sSd3AttTab
    ld      DE, #ADDR_SD3_ATT_TAB
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

void sdInit()
{
    _bgmMain  = nullptr;
    _sePri    = 0xff;
    for (u8 i = 0; i < NR_SE_SEQUENCERS; i++) {
        _seMains[i] = nullptr;
    }
    sdInitSub();
}


// ---------------------------------------------------------------- システム(メイン)
void sdBgmMain()
{
    u16 (*main)(u16) = _bgmMain;
    if (main) {
        _bgmCt = main(_bgmCt);
    }
}
void sdSeMain()
{
    // SE シーケンサ
    u8 i = 0;
    for (; i < NR_SE_SEQUENCERS; i++) {
        void (*main)(u8) = _seMains[i];
        if (main) {
            u8 ct = _seCts[i];
            ct--;
            main(ct);
            if (ct == 0) {
                _seMains[i] = nullptr;
                _sePri  = 0xff;       // 鳴り終わったらシーケンサー優先度をリセットする
                sdMake(0x0000);       // 音は止める
            } else {
                // 優先順位の低いシーケンサーは削除
                _seCts[i] = ct;
                for (i++; i < NR_SE_SEQUENCERS; i++) {
                    _seMains[i] = nullptr;
                }
                break;
            }
        }
    }
}

// ---------------------------------------------------------------- システム(ON/OFF)
#pragma disable_warning 85
#pragma save
void sdSetEnabled(const bool bEnabled) __z88dk_fastcall __naked
{
    // (MIO_ETC)       = 0x00 or MIO_ETC_GATE_MASK
    // (MIO_8253_CTRL) = MIO_8253_CH0_MODE3;
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

    // ---------------- 音も止めます
    ld      A, #MIO_8253_CH0_MODE3
    dec     L               // L = MIO_8253_CTRL
    ld      (HL), A

    // ---------------- 後始末
    BANK_RAM                    // バンク切替
    ret
__endasm;
}
#pragma restore


// ---------------------------------------------------------------- サウンド シーケンサ
void sdPlayBgm(const u8 bgm) __z88dk_fastcall
{
    const BgmSequencerDesc* const pTab = bgmGetSequencerDesc(bgm);
    _bgmMain = pTab->main;
    _bgmCt   = 0;
    if (pTab->init) { pTab->init(); }
}

void sdPlaySe(const u8 se) __z88dk_fastcall
{
    const SeSequencerDesc* const pTab = seGetSequencerDesc(se);
    u8 priority = pTab->priority;
    if (priority <= _sePri) {
        _sePri = priority;
        _seMains[priority] = pTab->main;
        _seCts[priority] = pTab->ct;
    }
}

// ---------------------------------------------------------------- 音を鳴らす(任意波長)
#pragma disable_warning 85
#pragma save
void sdMake(const u16 interval) __z88dk_fastcall __naked
{
    // (MIO_8253_CH0) = L;
    // (MIO_8253_CH0) = H;
    // if (!HL) { (MIO_8253_CTRL) = MIO_8253_CH0_MODE3; }
__asm
    // ---------------- 準備
    BANK_VRAM_IO               // バンク切替

    // ---------------- 音程 0x0000 の場合の処理
    // 音程が 0 ならば音を止めて, 優先順位を最低にします
    // 実機では音程に 0 を書き込むと音が止まりますが,
    // EmuZ-700 だと音が鳴るので, 8253 モード再セットによりカウンタを停止します
    ld      A, H
    or      A, L
    jp      nz, SDM_NON_ZERO
        ld  A, #MIO_8253_CH0_MODE3
        ld  (#MIO_8253_CTRL), A
        jp  SDM_END
SDM_NON_ZERO:

    // ----------------
    // カウンタをセット
    ld      DE, #MIO_8253_CH0
    ld      A, L
    ld      (DE), A
    ld      A, H
    ld      (DE), A

    // ---------------- 後始末
SDM_END:
    BANK_RAM                    // バンク切替
    ret
__endasm;
}
#pragma restore

// ---------------------------------------------------------------- 音を鳴らす(単音)
#pragma disable_warning 85
#pragma save
void sd1Play(const u8 scale) __z88dk_fastcall __naked
{
__asm
    // ---------------- 準備
    ld      H, #(ADDR_SD_SCALE_TAB >> 8)// L = 音階(偶数)
    ld      A, (HL)
    inc     L
    ld      H, (HL)
    ld      L, A                // HL = 音程

    BANK_VRAM_IO                // バンク切替

    // ---------------- 音程 0x0000 の場合の処理
    // 音程が 0 ならば音を止めて, 優先順位を最低にします
    // 実機では音程に 0 を書き込むと音が止まりますが,
    // EmuZ-700 だと音が鳴るので, 8253 モード再セットによりカウンタを停止します
    or      A, H
    jp      nz, SD1_NON_ZERO
        ld  A, #MIO_8253_CH0_MODE3
        ld  (#MIO_8253_CTRL), A
        jp  SD1_END
SD1_NON_ZERO:

    // ----------------
    // カウンタをセット
    ld      DE, #MIO_8253_CH0
    ld      A, L
    ld      (DE), A
    ld      A, H
    ld      (DE), A

    // ---------------- 後始末
SD1_END:
    BANK_RAM                    // バンク切替
    ret
__endasm;
}
#pragma restore

// ---------------------------------------------------------------- 音を鳴らす(三重和音)
static u16* _pMml0;     // サウンド データへのポインタ
static u16* _pMml1;
static u16* _pMml2;
static u8   _sdLen0;    // 音長カウンタ初期値(8bit)
static u8   _sdLen1;
static u8   _sdLen2;
static u8   _sdWaveLen0;  // 波長初期値(8bit)
static u8   _sdWaveLen1;
static u8   _sdWaveLen2;

// 一時的に使うワークエリア
#define ADDR_TMP_SP     (VVRAM_TMP_WORK + 10)// SP の仮場所. 4段もあれば十分

#pragma disable_warning 85
#pragma save
bool sd3Play(const u8* mml0, const u8* mml1, const u8* mml2, const bool bCancelEnabled) __naked
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
    // C = MIO_8253_CH0_MODE0 or MIO_8253_CH0_MODE3
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
    ;---- \
            pushHL                  ; 11 / 0 マクロ引数です\
            ld  HL, (pMml)          ; 16 小計26\
    ;---- F1 キーでキャンセル\
            ld  A, (MIO_8255_PORTB) ; 13\
            and A, A                ;  4\
            jp  p, SD3_CANCEL_END; 10 小計27\
    ;---- 波長初期値\
            ld  A, (HL)             ;  7 A = 波長初期値\
            and A                   ;  4\
            jp  z, SD3_END       ; 10 if (A == 0) goto end, H != 0 \
            ld  (waveLen), A        ; 13\
            inc HL                  ;  4 小計38\
    ;---- 音長初期値\
            ld  A, (HL)             ;  7 A = 音長初期値\
            ld  (sdLen), A          ; 13\
            inc HL                  ;  6 小計26\
    ;---- MML\
            ld  (pMml), HL          ; 16 HL = MML\
            popHL                   ; 10 マクロ引数です 小計26\
    ;---- 音長カウンタ\
            ld sdLenRegH##sdLenRegL, 0x0000; 10 小計10\
    ;---- 波長カウンタは初期化しません\
    exx                             ;  4\
    jp      LABEL_SD_END            ; 10 小計14\
    ;\
LABEL_SD_LEN_END:\
        ld  A, sdLenRegH            ;  4 音長カウンタ 上位 8bit\
    exx                             ;  4\
    ; ---------------- wave length 波長 \
    ; ---- if (waveLenReg <= sd3AttTab[A]) { C = MIO_8253_CH0_MODE3; }\
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
    ld      (SD3_PLAY_SP_RESTORE + 1), SP// SP 保存(自己書換)
    pop     HL                  // リターン アドレス(捨てる)

    SD3_INIT(__pMml0, __sdWaveLen0, __sdLen0, B)
    SD3_INIT(__pMml1, __sdWaveLen1, __sdLen1, D)
    SD3_INIT(__pMml2, __sdWaveLen2, __sdLen2, E)
    pop     HL                  // L= キャンセル可能フラグ

    ld      SP, #ADDR_TMP_SP

    // 音長カウンタの初期化
    exx
        ld  HL, 0x0000          // HL' 音長カウンタ 0 = 0x0000
        ld  BC, HL              // BC' 音長カウンタ 1 = 0x0000
        ld  DE, HL              // DE' 音長カウンタ 2 = 0x0000
    exx

    BANK_VRAM_IO                // バンク切替

    // F1 キーでキャンセルできるように, Key Strobe を仕込みます
    ld      A, 9                // A = key strobe 9
    dec     L                   // L = キャンセル可能フラグ
    jr      z, SD3_CANCEL_ENABLE
    inc     A
SD3_CANCEL_ENABLE:
    ld      (#MIO_8255_PORTA), A

    // ---------------- 波形合成
    ld      H, ADDR_SD3_ATT_TAB >> 8// 減衰テーブル
SD3_LOOP:
    ld      C, #MIO_8253_CH0_MODE0  // 7

    SD3_MAIN(__pMml0, __sdWaveLen0, __sdLen0, H, L, B,        ,       , SD3_SD_LEN_END0, SD3_SD_WAVE_PULSE0, SD3_SD_WAVE_LEN0, SD3_SD_END0)
    SD3_MAIN(__pMml1, __sdWaveLen1, __sdLen1, B, C, D, push HL, pop HL, SD3_SD_LEN_END1, SD3_SD_WAVE_PULSE1, SD3_SD_WAVE_LEN1, SD3_SD_END1)
    SD3_MAIN(__pMml2, __sdWaveLen2, __sdLen2, D, E, E, push HL, pop HL, SD3_SD_LEN_END2, SD3_SD_WAVE_PULSE2, SD3_SD_WAVE_LEN2, SD3_SD_END2)

SD3_LOOP_END:
    // ---------------- 波形出力
    ld      A, C                // 4
    ld      (#MIO_8253_CTRL), A // 13
    jp      SD3_LOOP            // 10

    // ---------------- 後始末
SD3_CANCEL_END:
    // F1 キーが離れるまで待ちます
    ld      A, (#MIO_8255_PORTB)
    and     A, A
    jp      p, SD3_CANCEL_END
    ld      H, 0                // return false

SD3_END:
    // 8253 を元の設定に戻します
    ld      A, #MIO_8253_CH0_MODE3
    ld      (#MIO_8253_CTRL), A

    BANK_RAM                    // バンク切替
SD3_PLAY_SP_RESTORE:
    ld      SP, #0x0000         // SP 復活
    ld      L, H                // 戻値
    ret
__endasm;
}
#pragma restore
