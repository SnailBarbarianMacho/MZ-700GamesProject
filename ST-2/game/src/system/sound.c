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

u16  (*bgm_main_)(u16);                   // サウンド BGM シーケンサ
u16    bgm_ct_;                           // サウンド BGM シーケンサ カウンタ
u8     se_pri_;                           // サウンド SE 優先順位
void (*se_mains_[NR_SE_SEQUENCERS])(u8);  // サウンド SE シーケンサ
u8     se_cts_[NR_SE_SEQUENCERS];         // サウンド SE シーケンサ カウンタ

// ---------------------------------------------------------------- システム(初期化)
// -------- sd1 音程テーブル
static const u16 SD_SCALE_TAB_[] = {
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
static const u8 SD_ATT_TAB_[] = {
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
    ld      HL, _SD_SCALE_TAB_
    ld      DE, #ADDR_SD_SCALE_TAB
    ld      BC, #((2 + 12 + 12 + 12) * 2)
    ldir

    // -------- 減衰テーブルの作成
    // 減衰テーブルのコピー
    ld      HL, _SD_ATT_TAB_
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
    bgm_main_  = nullptr;
    se_pri_    = 0xff;
    for (u8 i = 0; i < NR_SE_SEQUENCERS; i++) {
        se_mains_[i] = nullptr;
    }
    sdInitSub();
}


// ---------------------------------------------------------------- システム(メイン)
void sdBgmMain()
{
    u16 (*main_func)(u16) = bgm_main_;
    if (main_func) {
        bgm_ct_ = main_func(bgm_ct_);
    }
}
void sdSeMain()
{
    // SE シーケンサ
    u8 i = 0;
    for (; i < NR_SE_SEQUENCERS; i++) {
        void (*main_func)(u8) = se_mains_[i];
        if (main_func) {
            u8 ct = se_cts_[i];
            ct--;
            main_func(ct);
            if (ct == 0) {
                se_mains_[i] = nullptr;
                se_pri_  = 0xff;       // 鳴り終わったらシーケンサー優先度をリセットする
                sdMake(0x0000);       // 音は止める
            } else {
                // 優先順位の低いシーケンサーは削除
                se_cts_[i] = ct;
                for (i++; i < NR_SE_SEQUENCERS; i++) {
                    se_mains_[i] = nullptr;
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
    // (MMIO_ETC)       = 0x00 or MMIO_ETC_GATE_MASK
    // (MMIO_8253_CTRL) = MMIO_8253_CH0_MODE3;
__asm
    // ---------------- 準備
    BANK_VRAM_MMIO(C)          // バンク切替

    // ---------------- 制御
    ld      A, L
    or      A
    jp      z, SOUND_SET_ENABLED
    ld      A, #MMIO_ETC_GATE_MASK
SOUND_SET_ENABLED:
    ld      HL, #MMIO_ETC
    ld      (HL), A

    // ---------------- 音も止めます
    // モードを再セットするとカウンタは止まります
    ld      A, #MMIO_8253_CH0_MODE3
    dec     L                       // L = MMIO_8253_CTRL
    ld      (HL), A

    // ---------------- 後始末
    BANK_RAM(C)                    // バンク切替
    ret
__endasm;
}
#pragma restore


// ---------------------------------------------------------------- サウンド シーケンサ
void sdPlayBgm(const u8 bgm) __z88dk_fastcall
{
    const BgmSequencerDesc* const p_tab = bgmGetSequencerDesc(bgm);
    bgm_main_ = p_tab->main_func;
    bgm_ct_   = 0;
    if (p_tab->init_func) { p_tab->init_func(); }
}

void sdPlaySe(const u8 se) __z88dk_fastcall
{
    const SeSequencerDesc* const p_tab = seGetSequencerDesc(se);
    u8 priority = p_tab->priority;
    if (priority <= se_pri_) {
        se_pri_ = priority;
        se_mains_[priority] = p_tab->main_func;
        se_cts_[priority] = p_tab->ct;
    }
}

// ---------------------------------------------------------------- 音を鳴らす(任意波長)
#pragma disable_warning 85
#pragma save
void sdMake(const u16 interval) __z88dk_fastcall __naked
{
    // (MMIO_8253_CH0) = L;
    // (MMIO_8253_CH0) = H;
    // if (!HL) { (MMIO_8253_CTRL) = MMIO_8253_CH0_MODE3; }
__asm
    // ---------------- 準備
    BANK_VRAM_MMIO(C)          // バンク切替

    // ---------------- 音程 0x0000 の場合の処理
    // 音程が 0 ならば音を止めて, 優先順位を最低にします
    // 実機では音程に 0 を書き込むと音が止まりますが,
    // EmuZ-700 だと音が鳴るので, 8253 モード再セットによりカウンタを停止します
    ld      A, H
    or      A, L
    jp      nz, SDM_NON_ZERO
        // モードを再セットするとカウンタは止まります
        ld  A, #MMIO_8253_CH0_MODE3
        ld  (#MMIO_8253_CTRL), A
        jp  SDM_END
SDM_NON_ZERO:

    // ----------------
    // カウンタをセット
    ld      DE, HL
    ld      HL, #MMIO_8253_CH0
    ld      (HL), E
    ld      (HL), D

    // ---------------- 後始末
SDM_END:
    BANK_RAM(C)                  // バンク切替
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

    BANK_VRAM_MMIO(C)           // バンク切替

    // ---------------- 音程 0x0000 の場合の処理
    // 音程が 0 ならば音を止めて, 優先順位を最低にします
    // 実機では音程に 0 を書き込むと音が止まりますが,
    // EmuZ-700 だと音が鳴るので, 8253 モード再セットによりカウンタを停止します
    or      A, H
    jp      nz, SD1_NON_ZERO
        ld  A, #MMIO_8253_CH0_MODE3
        ld  (#MMIO_8253_CTRL), A
        jp  SD1_END
SD1_NON_ZERO:

    // ----------------
    // カウンタをセット
    ld      DE, #MMIO_8253_CH0
    ld      A, L
    ld      (DE), A
    ld      A, H
    ld      (DE), A

    // ---------------- 後始末
SD1_END:
    BANK_RAM(C)                 // バンク切替
    ret
__endasm;
}
#pragma restore

// ---------------------------------------------------------------- 音を鳴らす(三重和音)
static u16* p_mml0_;     // サウンド データへのポインタ
static u16* p_mml1_;
static u16* p_mml2_;
static u8   sd_len0_;    // 音長カウンタ初期値(8bit)
static u8   sd_len1_;
static u8   sd_len2_;
static u8   sd_wave_len0_;  // 波長初期値(8bit)
static u8   sd_wave_len1_;
static u8   sd_wave_len2_;

// 一時的に使うワークエリア
#define ADDR_TMP_SP     (VVRAM_TMP_WORK + 10)// SP の仮場所. 4段もあれば十分

#pragma disable_warning 85
#pragma save
bool sd3Play(const u8* mml0, const u8* mml1, const u8* mml2, const bool b_cancel_enabled) __naked
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
    // C = MMIO_8253_CH0_MODE0 or MMIO_8253_CH0_MODE3
    // H = ADDR_SD3_ATT_TAB の上位 8 bit
    // L 破壊
#define SD3_MAIN(p_mml, wave_len, sd_len, sd_len_reg_h, sd_len_reg_l, wave_len_ct_reg, push_hl, pop_hl, LABEL_SD_LEN_END, LABEL_SD_WAVE_PULSE, LABEL_SD_WAVE_LEN, LABEL_SD_END) \
    ; ---------------- sound length 音長 \
    ; ---- 音長カウンタ -- \
    exx                             ;  4\
        inc sd_len_reg_h##sd_len_reg_l; 6\
        ld  A, (sd_len)             ; 13\
        cp  A, sd_len_reg_h         ;  4\
        jp  nz, LABEL_SD_LEN_END    ; 10 if (sound length == 0) { read next MML }\
    ;---- \
            push_hl                 ; 11 / 0 マクロ引数です\
            ld  HL, (p_mml)         ; 16 小計26\
    ;---- F1 キーでキャンセル\
            ld  A, (MMIO_8255_PORTB); 13\
            and A, A                ;  4\
            jp  p, SD3_CANCEL_END; 10 小計27\
    ;---- 波長初期値\
            ld  A, (HL)             ;  7 A = 波長初期値\
            and A                   ;  4\
            jp  z, SD3_END          ; 10 if (A == 0) goto end, H != 0 \
            ld  (wave_len), A       ; 13\
            inc HL                  ;  4 小計38\
    ;---- 音長初期値\
            ld  A, (HL)             ;  7 A = 音長初期値\
            ld  (sd_len), A         ; 13\
            inc HL                  ;  6 小計26\
    ;---- MML\
            ld  (p_mml), HL         ; 16 HL = MML\
            pop_hl                  ; 10 マクロ引数です 小計26\
    ;---- 音長カウンタ\
            ld sd_len_reg_h##sd_len_reg_l, 0x0000; 10 小計10\
    ;---- 波長カウンタは初期化しません\
    exx                             ;  4\
    jp      LABEL_SD_END            ; 10 小計14\
    ;\
LABEL_SD_LEN_END:\
        ld  A, sd_len_reg_h         ;  4 音長カウンタ 上位 8bit\
    exx                             ;  4\
    ; ---------------- wave length 波長 \
    ; ---- if (wave_len_reg <= ATT_TAB_[A]) { C = MMIO_8253_CH0_MODE3; }\
    ld      L, A                    ;  4 HL = sound3 att table\
    ld      A, (HL);                ;  7 offset is self-modify\
    cp      A, wave_len_ct_reg      ;  4\
    jp      c, LABEL_SD_WAVE_PULSE  ; 10  (if A > Wave_len_ct_reg) .. \
        ld  C, MMIO_8253_CH0_MODE3  ;  7\
;\
LABEL_SD_WAVE_PULSE:\
    ; ---- wave length Loop\
    dec     wave_len_ct_reg         ;  4\
    jp      nz, LABEL_SD_WAVE_LEN   ; 10\
        ld  A, (wave_len)           ; 13\
        ld  wave_len_ct_reg, A      ;  4\
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

    SD3_INIT(_p_mml0_, _sd_wave_len0_, _sd_len0_, B)
    SD3_INIT(_p_mml1_, _sd_wave_len1_, _sd_len1_, D)
    SD3_INIT(_p_mml2_, _sd_wave_len2_, _sd_len2_, E)
    pop     HL                  // L= キャンセル可能フラグ

    ld      SP, #ADDR_TMP_SP    // バンク切替による 臨時 SP
    BANK_VRAM_MMIO(C)           // バンク切替

    // 音長カウンタの初期化
    exx
        ld  HL, 0x0000          // HL' 音長カウンタ 0 = 0x0000
        ld  BC, HL              // BC' 音長カウンタ 1 = 0x0000
        ld  DE, HL              // DE' 音長カウンタ 2 = 0x0000
    exx

    // F1 キーでキャンセルできるように, Key Strobe を仕込みます
    ld      A, 0xf9             // A = key strobe 9
    dec     L                   // L = キャンセル可能フラグ
    jr      z, SD3_CANCEL_ENABLE
    inc     A
SD3_CANCEL_ENABLE:
    ld      (#MMIO_8255_PORTA), A

    // ---------------- 波形合成
    ld      H, ADDR_SD3_ATT_TAB >> 8// 減衰テーブル
SD3_LOOP:
    ld      C, #MMIO_8253_CH0_MODE0 // 7

    SD3_MAIN(_p_mml0_, _sd_wave_len0_, _sd_len0_, H, L, B,        ,       , SD3_SD_LEN_END0, SD3_SD_WAVE_PULSE0, SD3_SD_WAVE_LEN0, SD3_SD_END0)
    SD3_MAIN(_p_mml1_, _sd_wave_len1_, _sd_len1_, B, C, D, push HL, pop HL, SD3_SD_LEN_END1, SD3_SD_WAVE_PULSE1, SD3_SD_WAVE_LEN1, SD3_SD_END1)
    SD3_MAIN(_p_mml2_, _sd_wave_len2_, _sd_len2_, D, E, E, push HL, pop HL, SD3_SD_LEN_END2, SD3_SD_WAVE_PULSE2, SD3_SD_WAVE_LEN2, SD3_SD_END2)

SD3_LOOP_END:
    // ---------------- 波形出力
    ld      A, C                // 4
    ld      (#MMIO_8253_CTRL), A// 13
    jp      SD3_LOOP            // 10

    // ---------------- 後始末
SD3_CANCEL_END:
    // F1 キーが離れるまで待ちます
    ld      A, (#MMIO_8255_PORTB)
    and     A, A
    jp      p, SD3_CANCEL_END
    ld      H, 0                // return false

SD3_END:
    // 8253 を元の設定に戻します
    ld      A, #MMIO_8253_CH0_MODE3
    ld      (#MMIO_8253_CTRL), A

    BANK_RAM(C)                 // バンク切替
SD3_PLAY_SP_RESTORE:
    ld      SP, #0x0000         // SP 復活
    ld      L, H                // 戻値
    ret
__endasm;
}
#pragma restore
