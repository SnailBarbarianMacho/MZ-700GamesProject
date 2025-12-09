/**
 * サウンド - PFM方式6重和音
 * @author Snail Barbarian Macho (NWK)
 */

#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "../src-common/asm-macros.h"
#include "../src-common/aal80.h"
#include "addr.h"
#include "sound.h"

// ---------------------------------------------------------------- チェック, カスタマイズ, マクロ
// -------- 周波数や8255パルス パラメータのチェック
    STATIC_ASSERT(SD6_B1 < 256 + 1, "valueOverflow"); // 値が大きすぎて鳴らせません
    STATIC_ASSERT(SD6_C6 >= 15,     "valueUnderflow");// 値が小さすぎて音が歪みます

#if 0
    STATIC_ASSERT(SD6_WL(SD_FREQ_C2) < 128.0f, "valueOverflow");    // 値が大きすぎ
    STATIC_ASSERT(SD6_WL(SD_FREQ_C5) > 15.0f,  "valueUnderflow");   // 値が小さすぎて高音が歪みます
    #define MMIO_8253_CT0_PWM   MMIO_8253_CTRL_CT0(MMIO_8253_CTRL_RL_L_MASK, MMIO_8253_CTRL_MODE0_MASK)
#endif

// -------- カスタマイズ
#define BEEPER1_WL_CT_VAL   8       // [0, 255]          Beeper1 の波長カウンタ初期値. 偽音が発生するときに調整します
#define BEEPER1_SYNC        true    // true/false        Beeper1 の波長カウンタのリセットを Beeper0 と同期します. 偽音の発生を軽減します
#define TEMPO               0       // [0, 128]          128にすると1.5倍速になります.
#define BASE_VOL_SPEED_R    7       // 1, 3, 7, 15, ...  Base   のエンベロープ音量変化速度
#define CHORD2_VOL_SPEED_R  7       // 1, 3, 7, 15, ...  Chord2 のエンベロープ音量変化速度

// Lead(Echo)の遅延. 次の3つから選択します
void LEAD_ECHO_DELAY(void) __aal_macro __naked
{
    AAL_DEF_VARS;

    //cp(A, 3);     // 遅延小
    //cp(A, 2);     // 遅延中
    A--;          // 遅延大

    AAL_ENDM;
}

// Lead にて特別な命令に使われる音長. この音長の音符は使えません(4の倍数, makefile で定義). 曲にあわせて設定してください
#ifndef SD6_EXCEPT_LEN
#define SD6_EXCEPT_LEN      0
#endif

// -------- パルス
#define PULSE_L MMIO_8253_CT0_MODE0
#define PULSE_H MMIO_8253_CT0_MODE3

// -------- その他設定
#pragma save
#pragma disable_warning 85          // 引数未使用警告抑止
#pragma disable_warning 59          // 戻値未設定警告抑止

// ---------------------------------------------------------------- 初期化
// -------- sd6 テーブル
static u8 const SD6_DATA_[] = {
#include "music/sd6-data.h"
};

// MARK: init()
/**
 * サウンド初期化
 */
void sd6Init(void) __aal
{
    AAL_DEF_VARS;
extern _SD6_DATA_;
extern sd6Init_dataLoop;

    // -------- データを, ドラム データと, パルス幅変換テーブル に分解してコピー
    HL = _SD6_DATA_;
    DE = ADDR_SD6_DRUM;
    BC = SZ_SD6_DRUM;
    ldir();

    HL = ADDR_SD6_DRUM;                                 // DE = ADDR_SD6_DRUM + 256
    B  = SZ_SD6_TAB & 0xff;
sd6Init_dataLoop: {
        A = mem[HL]; A &= 0x1f; mem[DE] = A;            // パルス幅変換テーブルを抽出コピー
        A ^= mem[HL]; mem[HL] = A;                      // ドラム データのみに修正 (bit5～7) します. bit4 は常に0
        L++; E++;
    } djnz(B, sd6Init_dataLoop);

    // -------- 8253 の初期設定
    BANKH_VRAM_MMIO(C);                                 // バンク切替
    HL = MMIO_ETC;
    mem[HL] = MMIO_ETC_GATE_MASK;                       // GATE ON
    L--;                                                // L = MMIO_8253_CTRL
    mem[HL] = MMIO_8253_CT0_MODE3;
    BANKH_RAM(C);                                       // バンク切替
}


// ---------------------------------------------------------------- プレイ
// MARK: NOTE_LEAD()
/** 音符処理 + 波長・音量変更処理(Lead)
 * @param reg_wlc               レジスタ: 波長カウンタ
 * @param reg_slc               レジスタ: 音長カウンタ
 * @param reg_tmp_h             レジスタ: テンポラリとして H を使用します
 * @param reg_tmp_l             レジスタ: テンポラリとして L を使用します
 * @param reg_tmp_hl            レジスタ: テンポラリとして HL を使用します
 * @param addr_rep_1            アドレス(自己書換): リピート用スタック
 * @param addr_vol_1            アドレス(自己書換): Beeper0/1 音量
 * @param addr_wl_1             アドレス(自己書換): Beeper0/1 波長
 * @param addr_slide_0          アドレス(自己書換): スライド モード
 * @param addr_mod_vol_speed_1  アドレス(自己書換): 音量変化速度
 * @param addr_beeper1_sync_0   アドレス(自己書換): Beeper1 の同期
 * @param addr_end              アドレス: 終了ならばこのアドレスに飛びます
 */
void SD6PLAY_NOTE_LEAD(
    int reg_wlc, int reg_slc,
    int reg_tmp_h, int reg_tmp_l, int reg_tmp_hl,
    int addr_rep_1,
    int addr_vol_1, int addr_wl_1, int addr_slide_0, int addr_mod_vol_speed_1,
    int addr_beeper1_sync_0,
    int addr_end
) __aal_macro __naked
{
    AAL_DEF_VARS;
    extern
        sd6playNoteLead_pop,
        sd6playNoteLead_rep_2,
        sd6playNoteLead_initEnd, sd6playNoteLead_initEndSlur,
        sd6playNoteLead_modBranch,
        sd6playNoteLead_modWl,  sd6playNoteLead_modWlEnd_1, sd6playNoteLead_modWlSpeed_1, sd6playNoteLead_modWlAdd_0,
        sd6playNoteLead_modVol, sd6playNoteLead_modVolVal_1, sd6playNoteLead_modVolEnd_1, sd6playNoteLead_modVolInc_0,
        sd6playNoteLead_modEnd;

    // -------- 音符処理: 音長カウンタが 0 になったら, 次の音符を読みます
    reg_slc--; if (z) {                                 // z になる割合は 1/4～1/256
    sd6playNoteLead_pop:
        pop(reg_tmp_hl);                                // H = 波長, L = 音長

        // ---- 音長モード別の波長/音長設定
        A = reg_tmp_l; A += A; if (nc) {
            A += A; reg_slc = A; if (nc_jr) {
                // ---- リピート
                cp(A, SD6_EXCEPT_LEN); if (nc) {        // A = 音長
                    cp(A, SD6_EXCEPT_LEN + 4 * 7); if (c) {
                        SP--;
                        A -= SD6_EXCEPT_LEN - 8;        // A = 8, 12, ..., 32
                        rrca_n(A, 2);                   // A = 2, 3, ..., 8
                        HL = mem[addr_rep_1 + 1];
                        L++; mem[sd6playNoteLead_rep_2 + 2] = HL; sd6playNoteLead_rep_2: mem[0x0000] = SP; // 自己書換 mem[HL] = SP
                        L++; L++; mem[HL] = A;
                        mem[addr_rep_1 + 1] = HL;
                        goto sd6playNoteLead_pop;
                    }
                }

                // ---- 休符
                SP--;
                xor(A, A); mem[addr_wl_1  + 1] = A;     // 自己書換 波長 = 0(最長)
                A++;       mem[addr_vol_1 + 1] = A;     // 自己書換 音量 = 1(無音) エコーでも使うので0ではなく1
                HL = sd6playNoteLead_modEnd;            // 休符は波長も音量も変更しない
                goto sd6playNoteLead_initEndSlur;       // スラー有
            }

            // ---- リピート終了
            cp(A, SD6_EXCEPT_LEN); if (z_jr) {
addr_rep_1:     HL = 0x0000; dec(mem[HL]); if (z_jr) { // リピート終了
                    L--; L--; L--; mem[addr_rep_1 + 1] = HL;
                    SP--;
                    goto sd6playNoteLead_pop;
                }
                // リピート中
                L--; A = mem[HL]; L--; L = mem[HL]; H = A; SP = HL;
                goto sd6playNoteLead_pop;
            }

            // ---- 終了コード
            {
                cp(A, SD6_EXCEPT_LEN + 4); jp_z(addr_end);
            }

            // ---- ◢
            {
                A = reg_tmp_h; mem[addr_wl_1 + 1] = A;              // 自己書換 波長
                reg_tmp_h = ADDR_SD6_TAB / 256; reg_tmp_l = A; A = mem[reg_tmp_hl]; // 音量 = 波長->パルス幅変換テーブル
                mem[           sd6playNoteLead_modVolEnd_1 + 1] = A;// 自己書換 音量end
                A = 2;     mem[sd6playNoteLead_modVolVal_1 + 1] = A;// 自己書換 音量 = 2(最小)
                mem[addr_vol_1 + 1] = A;                            // 自己書換 音量
                A = OPCODE_INC_A; mem[sd6playNoteLead_modVolInc_0] = A; // 自己書換 音量変化速度(加算)
                HL = sd6playNoteLead_modVol;
                goto sd6playNoteLead_initEnd;           // スラー無
            }
        }
        A += A; reg_slc = A; if (nc_jr) {
            // ---- エンベロープ(音量)速度設定
            cp(A, SD6_EXCEPT_LEN); if (nc) {            // A = 音長
                cp(A, SD6_EXCEPT_LEN + 4 * 4); if (c) {
                    A -= SD6_EXCEPT_LEN - 1;            // A = 1, 5, 9, 13
                    reg_tmp_h = ADDR_SD6_TAB / 256; reg_tmp_l = A; A = mem[reg_tmp_hl]; // 音量 = 波長->パルス幅変換テーブル
                    mem[addr_mod_vol_speed_1 + 1] = A;  // 自己書換 音量速度
                    SP--;
                    goto sd6playNoteLead_pop;
                }
            }

            // ---- ◣
            {
                A = reg_tmp_h; mem[addr_wl_1 + 1] = A;  // 自己書換 波長
                reg_tmp_h = ADDR_SD6_TAB / 256; reg_tmp_l = A; A = mem[reg_tmp_hl]; // 音量 = 波長->パルス幅変換テーブル
                mem[           sd6playNoteLead_modVolVal_1 + 1] = A;// 自己書換 音量
                mem[addr_vol_1                             + 1] = A;// 自己書換 音量
                A = 2;            mem[sd6playNoteLead_modVolEnd_1 + 1] = A; // 自己書換 音量end = 2
                A = OPCODE_DEC_A; mem[sd6playNoteLead_modVolInc_0    ] = A; // 自己書換 音量変化速度(減算)
                HL = sd6playNoteLead_modVol;
                goto sd6playNoteLead_initEnd;           // スラー無
            }
        }

        cp(A, SD6_EXCEPT_LEN); if (nc) {                // A = 音長
            // ---- スライド無
            if (z_jr) {
                A = OPCODE_JR/*スライド無*/; mem[addr_slide_0] = A;
                SP--;
                goto sd6playNoteLead_pop;
            }

            // ---- スライド(波長)速度設定
            cp(A, SD6_EXCEPT_LEN + 4 * 4); if (c) {
                A -= SD6_EXCEPT_LEN - 2;                // A = 6, 10, 14
                reg_tmp_h = ADDR_SD6_TAB / 256; reg_tmp_l = A; A = mem[reg_tmp_hl]; // 音量 = 波長->パルス幅変換テーブル
                mem[sd6playNoteLead_modWlSpeed_1 + 1] = A;  // 自己書換 波長速度
                A = OPCODE_LD_A_N/*スライド有*/; mem[addr_slide_0] = A;
                SP--;
                goto sd6playNoteLead_pop;
            }
        }

        // ---- ■ スライド有 (スラー:波長カウンタ リセットや音量の設定無し)
addr_slide_0: if (z_jr) {                               // 自己書換 jr/ld A,n = スライド無/有
            A = reg_tmp_h; mem[sd6playNoteLead_modWlEnd_1 + 1] = A; // 自己書換 波長end
            // 現在の波長と比較してスライド方向を決める
            A = mem[addr_wl_1 + 1]; cp(A, reg_tmp_h); A = OPCODE_INC_A; if (nc_jr) { A++; /* DEC A */}
            mem[sd6playNoteLead_modWlAdd_0] = A;        // 自己書換 A++/A--
            HL = sd6playNoteLead_modWl;
            goto sd6playNoteLead_initEndSlur;           // スラー有
        }

        // ---- ■ スライド無
        {
            A = reg_tmp_h; mem[addr_wl_1 + 1] = A;      // 自己書換 波長
            reg_tmp_h = ADDR_SD6_TAB / 256; reg_tmp_l = A; A = mem[reg_tmp_hl]; // 音量 = 波長->パルス幅変換テーブル
            mem[addr_vol_1               + 1] = A;      // 自己書換 音量
            HL = sd6playNoteLead_modEnd;
            //goto sd6playNoteLead_initEnd;             // スラー無
        }

sd6playNoteLead_initEnd:
        reg_wlc = 1;                                    // スラー無 (波長カウンタ リセット)
#if BEEPER1_SYNC
        A = OPCODE_LD_A_N; mem[addr_beeper1_sync_0] = A;// Beeper1 同期
#endif

sd6playNoteLead_initEndSlur:
        mem[sd6playNoteLead_modBranch + 1] = HL;        // 自己書換 波長音量変更ジャンプ先
    }

    // -------- 波長・音量変更処理
sd6playNoteLead_modBranch: jp(sd6playNoteLead_modEnd/*ジャンプ先*/);

    // ---- 波長変更処理(■(スライド有))
sd6playNoteLead_modWl:
    A = reg_slc; sd6playNoteLead_modWlSpeed_1: A &= 0x00/*波長変更速度*/; jr_nz(sd6playNoteLead_modEnd); {
        A = mem[addr_wl_1 + 1]; sd6playNoteLead_modWlEnd_1: cp(A, 0x00/*波長end*/);
        jr_z(sd6playNoteLead_modEnd); sd6playNoteLead_modWlAdd_0: A++; mem[addr_wl_1 + 1] = A;
        reg_tmp_h = ADDR_SD6_TAB / 256; reg_tmp_l = A; A = mem[reg_tmp_hl]; // 音量 = 波長->パルス幅変換テーブル
        mem[addr_vol_1 + 1] = A;                        // 自己書換 音量
        goto sd6playNoteLead_modEnd;
    }

    // ---- 音量変更◢◣
sd6playNoteLead_modVol:
    A = reg_slc; addr_mod_vol_speed_1: A &= 0x00/*音量変更速度*/; if (z) {
        sd6playNoteLead_modVolVal_1: A = 0x00/*音量*/; sd6playNoteLead_modVolEnd_1: cp(A, 0x00/*音量end*/);
        if (nz_jr) { sd6playNoteLead_modVolInc_0: A++/*音量加減*/; mem[sd6playNoteLead_modVolVal_1 + 1] = A; }
        mem[addr_vol_1 + 1] = A;                        // 自己書換 音量
    }

    // ---- 何もしない
sd6playNoteLead_modEnd:

    AAL_ENDM;
}


// MARK: NOTE_BASE()
/** 音符処理 + 音量変更処理(Base)
 * @param reg_wlc               レジスタ: 波長カウンタ
 * @param reg_slc               レジスタ: 音長カウンタ
 * @param reg_tmp_h             レジスタ: テンポラリとして H を使用します
 * @param reg_tmp_l             レジスタ: テンポラリとして L を使用します
 * @param reg_tmp_hl            レジスタ: テンポラリとして HL を使用します
 * @param addr_vol_1            アドレス(自己書換): Beeper 音量セット先
 * @param addr_wl_1             アドレス(自己書換): Beeper 波長セット先
 */
void SD6PLAY_NOTE_BASE(
    int reg_wlc, int reg_slc,
    int reg_tmp_h, int reg_tmp_l, int reg_tmp_hl,
    int addr_vol_1, int addr_wl_1
) __aal_macro __naked
{
    AAL_DEF_VARS;
    extern
        sd6playNoteBase_fl, sd6playNoteBase_initEnd,
        sd6playNoteBase_modStart,
        sd6playNoteBase_modVol,
        sd6playNoteBase_modVolEnd,
        sd6playNoteBase_modVolInc,
        sd6playNoteBase_modVolConv,
        sd6playNoteBase_modEnd;

    // -------- 音符処理: 音長カウンタが 0 になったら, 次の音譜を読みます
    reg_slc--; if (z) {                                 // z になる割合は 1/4～1/256
        pop(reg_tmp_hl);                                // H=波長, L=音長

        // ---- 音長モード別の波長/音長設定
        A = reg_tmp_l; A += A; if (nc) {
            A += A; reg_slc = A; if (nc_jr) {
                // ---- 休符
                SP--;
                xor(A, A); mem[addr_wl_1  + 1] = A;     // 自己書換 波長 = 0(最長)
                A++; mem[      addr_vol_1 + 1] = A;     // 自己書換 音量 = 1(無音)
                A = OPCODE_JR;                          // 分岐でジャンプ
                goto sd6playNoteBase_initEnd;           // 音量 = 0
            }
            {
                // ---- ◢
                A = reg_tmp_h; mem[addr_wl_1         + 1] = A;  // 自己書換 波長
                reg_tmp_h = ADDR_SD6_TAB / 256; reg_tmp_l = A; A = mem[reg_tmp_hl]; // 音量 = 波長->パルス幅変換テーブル
                mem[       sd6playNoteBase_modVolEnd + 1] = A;  // 自己書換 音量end
                A = 2; mem[sd6playNoteBase_modVol    + 1] = A;  // 自己書換 音量 = 2(最小)
                mem[       addr_vol_1                + 1] = A;  // 自己書換 音量 = 2(最小)
                A = OPCODE_INC_A; mem[sd6playNoteBase_modVolInc] = A;   // 自己書換 音量変化速度(加算)
                A = OPCODE_LD_A_N;                      // 分岐でジャンプしない
            }
        } else {
            A += A; reg_slc = A;                        // 音長 cf:0/1 = ◣■
            A = reg_tmp_h; mem[addr_wl_1 + 1] = A;      // 自己書換 波長
            reg_tmp_h = ADDR_SD6_TAB / 256; reg_tmp_l = A; A = mem[reg_tmp_hl]; // 音量 = 波長->パルス幅変換テーブル
            if (nc_jr) {
                // ---- ◣
                mem[       sd6playNoteBase_modVol    + 1] = A;  // 自己書換 音量
                mem[       addr_vol_1                + 1] = A;  // 自己書換 音量
                A = 2; mem[sd6playNoteBase_modVolEnd + 1] = A;  // 自己書換 音量end
                A = OPCODE_DEC_A; mem[sd6playNoteBase_modVolInc] = A;   // 自己書換 音量変化速度(減算)
                A = OPCODE_LD_A_N;                      // 分岐でジャンプしない
            } else {
                // ---- ■
                mem[addr_vol_1 + 1] = A;                // 自己書換 音量
                A = OPCODE_JR;                          // 分岐でジャンプ
            }
        }
sd6playNoteBase_initEnd:
        mem[sd6playNoteBase_modStart] = A;              // 自己書換 分岐でジャンプ/ジャンプしない
        reg_wlc = 1;                                    // 波長カウンタ. すぐに再設定されます
    }

    // -------- 音量変更処理
sd6playNoteBase_modStart:
    jr(sd6playNoteBase_modEnd/*ジャンプ先*/);

    // ---- 音量変更◢◣
    A = reg_slc; A &= BASE_VOL_SPEED_R; if (z) {
sd6playNoteBase_modVol: A = 0x00;                       // 音量
sd6playNoteBase_modVolEnd: cp(A, 0x00);                 // 音量end
        if (nz_jr) {
sd6playNoteBase_modVolInc: A ++;                        // 音量加減
            mem[sd6playNoteBase_modVol + 1] = A;
        }
        mem[addr_vol_1 + 1] = A;
    }

    // ---- 何もしない
sd6playNoteBase_modEnd:

    AAL_ENDM;
}


// MARK: NOTE_CHORD2()
/** 音符処理 + 音量変更処理(Chord)x2
 * - コードは音長と音量が兼用です
 * @param reg_wlc0              レジスタ: 波長カウンタ
 * @param reg_wlc1              レジスタ: 波長カウンタ
 * @param reg_wlc10             レジスタ: (reg_wl0 << 8) | reg_wl1
 * @param reg_slc               レジスタ: 音長カウンタ
 * @param reg_tmp_h             レジスタ: テンポラリとして H を使用します
 * @param reg_tmp_l             レジスタ: テンポラリとして L を使用します
 * @param reg_tmp_hl            レジスタ: テンポラリとして HL を使用します
 * @param reg_vol01             レジスタ:           Beeper 音量0/1兼用
 * @param addr_wl0_1            アドレス(自己書換): Beeper 音長セット先0
 * @param addr_wl1_1            アドレス(自己書換): Beeper 音長セット先1
 */
void SD6PLAY_NOTE_CHORD2(
    int reg_wlc0, int reg_wlc1, int reg_wlc10, int reg_slc,
    int reg_tmp_h, int reg_tmp_l, int reg_tmp_hl,
    int reg_vol01, int addr_wl0_1, int addr_wl1_1
) __aal_macro __naked
{
    AAL_DEF_VARS;
    extern
        //sd6playNoteChord2_fl0, sd6playNoteChord2_fl1,
        sd6playNoteChord2_initEnd,
        sd6playNoteChord2_modStart_0,
        sd6playNoteChord2_modVol_1,
        sd6playNoteChord2_modVolEnd_1,
        sd6playNoteChord2_modVolInc_0,
        sd6playNoteChord2_modEnd;

    // -------- 音符処理: 音長カウンタが 0 になったら, 次の音譜を読みます
    reg_slc--; if (z) {                                 // z になる割合は 1/4～1/256
        SP--; pop(AF);                                  // A = 音長
        // ---- 音長モード別の波長/音長設定
        A += A; if (nc) {
            A += A; reg_slc = A; if (nc_jr) {
                // ---- 休符
                xor(A, A); mem[addr_wl0_1 + 1] = A;     // 自己書換 波長0 = 0(最長)
                mem[           addr_wl1_1 + 1] = A;     // 自己書換 波長1 = 0(最長)

                reg_vol01 = A;                          //          音量0/1兼用 = 0(無音)

                A = OPCODE_JR;                          // 分岐でジャンプ
                goto sd6playNoteChord2_initEnd;
            }
            {
                pop(reg_wlc10);

                // ---- ◢
                A = reg_wlc1; mem[addr_wl1_1 + 1] = A;  // 自己書換 波長1
                A = reg_wlc0; mem[addr_wl0_1 + 1] = A;  // 自己書換 波長0

                reg_tmp_h = ADDR_SD6_TAB / 256; reg_tmp_l = A; A = mem[reg_tmp_hl]; // 音量 = 波長->パルス幅変換テーブル
                mem[sd6playNoteChord2_modVolEnd_1 + 1] = A; // 自己書換 音量変更end0/1兼用

                A = 2;
                mem[sd6playNoteChord2_modVol_1 + 1] = A;// 自己書換 音量変更0/1兼用
                reg_vol01 = A;                          //          音量0/1兼用

                A = OPCODE_INC_A;
                mem[sd6playNoteChord2_modVolInc_0] = A; // 自己書換 音量変化速度(加算)0/1兼用

                A = OPCODE_LD_A_N;                      // 分岐でジャンプしない
            }
        } else {
            A += A; reg_slc = A;                        // 音長 cf:0/1 = ◣■
            pop(reg_wlc10);

            A = reg_wlc0; mem[addr_wl0_1 + 1] = A;      // 自己書換 波長0
            reg_tmp_h = ADDR_SD6_TAB / 256; reg_tmp_l = A; A = mem[reg_tmp_hl]; // 音量 = 波長->パルス幅変換テーブル
            reg_vol01 = A;                              //          音量0/1兼用

            if (nc_jr) {
                // ---- ◣
                mem[sd6playNoteChord2_modVol_1 + 1] = A;// 自己書換 音量変更0/1兼用

                A = reg_wlc1; mem[addr_wl1_1 + 1] = A;  // 自己書換 波長1

                A = 2;
                mem[sd6playNoteChord2_modVolEnd_1 + 1] = A; // 自己書換 音量変化end0/1兼用

                A = OPCODE_DEC_A;
                mem[sd6playNoteChord2_modVolInc_0] = A; // 自己書換 音量変化速度(減算)0/1兼用

                A = OPCODE_LD_A_N;                      // 分岐でジャンプしない
            } else {
                // ---- ■
                A = reg_wlc1; mem[addr_wl1_1 + 1] = A;  // 自己書換 波長1

                A = OPCODE_JR;                          // 分岐でジャンプ
            }
        }
sd6playNoteChord2_initEnd:
        mem[sd6playNoteChord2_modStart_0] = A;          // 自己書換 分岐でジャンプ/ジャンプしない
        reg_wlc0 = 1;                                   // 波長カウンタ. すぐに再設定されます
        reg_wlc1 = reg_wlc0;
    }

    // -------- 音量変更処理
sd6playNoteChord2_modStart_0: jr(sd6playNoteChord2_modEnd/*ジャンプ先*/);

    A = reg_slc; A &= CHORD2_VOL_SPEED_R; if (z) {
        // ---- 音量変更◢◣0/1兼用
sd6playNoteChord2_modVol_1: A = 0x00;                   // 音量
sd6playNoteChord2_modVolEnd_1: cp(A, 0x00);             // 音量end
        if (nz_jr) {
sd6playNoteChord2_modVolInc_0: A++;                     // 音量加減
            mem[sd6playNoteChord2_modVol_1 + 1] = A;
        }
        reg_vol01 = A;
    }

    // ---- 何もしない
sd6playNoteChord2_modEnd:

    AAL_ENDM;
}


// MARK: NOTE_DRUM()
/** 音譜処理(Drum)
 * @param reg_sl            レジスタ: 音長カウンタ
 * @param reg_tmp           レジスタ: テンポラリ
 * @param addr_drum_on      アドレス: 自己書換 ドラム処理フラグ
 * @param addr_drum_nr_1    アドレス: 自己書換 ドラム番号
 */
void SD6PLAY_NOTE_DRUM(int reg_sl, int reg_tmp, int addr_drum_nr_1) __aal_macro __naked
{
    AAL_DEF_VARS;

    A = 0x46;                                           // ドラム OFF
    reg_sl--; if (z) {                                  // z になる割合は 1/8～1/256
        SP--; pop(AF);
        reg_tmp = A; A &= 0xfc; reg_sl = A;             // 音長
        A ^= reg_tmp;                                   // reg_tmp と A を xor すると, ドラム番号(0-3)だけ残る
        rlca_n(A, 3); A += 0x66;                        // 0,1,2,3→0x66,0x6e,0x76,0x7e それぞれ bit(4～7,mem[HL])
    }
    mem[addr_drum_nr_1 + 1] = A;                        // 自己書換 bit(n, mem[HL])の一部

    AAL_ENDM;
}


// -------------------------------- 波長処理 マクロ
// MARK: BEEPER_WAVE()
/** 波形生成と合成処理(Lead, Chord, Base). 波長カウンタに併せて, 8255パルスを立てる. 波長カウンタが 0 になったら wl_set に飛ぶ
 * @param reg_wl        レジスタ: 波長カウンタ
 * @param reg_pulse     レジスタ: 8255パルス
 * @param addr_vol_1    アドレス: 音量をセットする(8255パルス幅を決める)
 * @param addr_wl_1     アドレス: 波長をセットする
 */
void SD6PLAY_BEEPER_WAVE(int reg_wl, int reg_pulse, int addr_vol, int addr_wl_1) __aal_macro __naked
{
    AAL_DEF_VARS;

    // 波長処理
    reg_wl--; if (z) {
addr_wl_1: reg_wl = 0x00;                               // 波長を設定したらその場は音量処理は不要
    } else {
        // 音量処理 vol < 2 ならば, 波長カウンタの値に依らず cf が立たない
        A = reg_wl;
addr_vol_1: cp(A, 0x02/*音量*/); if (c_jr) { reg_pulse = PULSE_H; }
    }
    // 計 最短 4+12+7+4+12 = 39(T-states)

    AAL_ENDM;
}


// MARK: BEEPER_WAVE_R()
/** 8bit レジスタが1つ余ってたら SD6PLAY_BEEPER_WAVE の代わりにこちらを使おう
 * @param reg_wl        レジスタ: 波長カウンタ
 * @param reg_pulse     レジスタ: 8255パルス値
 * @param reg_vol       レジスタ: 音量(8255パルス幅)
 * @param addr_wl_1     アドレス: 波長をセットする
 */
void SD6PLAY_BEEPER_WAVE_R(int reg_wl, int reg_pulse, int reg_vol, int addr_wl_1) __aal_macro __naked
{
    AAL_DEF_VARS;

    // 波長処理
    reg_wl--; if (z) {
addr_wl_1: reg_wl = 0x00;/* 波長を設定したらその場は音量処理は不要 */
    } else {
        // 音量処理. vol < 2 ならば, 波長カウンタの値に依らず cf が立たない
        A = reg_wl; cp(A, reg_vol); if (c_jr) { reg_pulse = PULSE_H; }
    }
    // 計 最短 4+12+4+4+12 = 36(T-states)

    AAL_ENDM;
}


// MARK: BEEPER_WAVE_BR()
/** 波長カウンタが B ならば, 少し速くなるぞ!
 * @param reg_wl        レジスタ: 波長カウンタ=B
 * @param reg_pulse     レジスタ: 8255パルス値
 * @param reg_vol       レジスタ: 音量(8255パルス幅)
 * @param addr_wl_1     アドレス: 波長をセットする
 */
void SD6PLAY_BEEPER_WAVE_BR(int reg_wl, int reg_pulse, int reg_vol, int addr_wl_1) __aal_macro __naked
{
    AAL_DEF_VARS;
    AAL_LOCAL(sd6play_beeperWaveBr, sd6play_beeperWaveBrEnd);
    extern sd6play_beeperWaveBr, sd6play_beeperWaveBrEnd;

    // 波長処理
    djnz(B, sd6play_beeperWaveBr); {
addr_wl_1: reg_wl = 0x00; jr(sd6play_beeperWaveBrEnd);/* 波長を設定したらその場は音量処理は不要 */
    }
sd6play_beeperWaveBr: {
        // 音量処理. vol < 2 ならば, 波長カウンタの値に依らず cf が立たない
        A = reg_wl; cp(A, reg_vol); if (c_jr) { reg_pulse = PULSE_H; }
    }
sd6play_beeperWaveBrEnd:
    // 計 最短 13+4+4+12 = 33(T-states)

    AAL_ENDM;
}


// MARK: BEEPER_DRUM()
/** 波形生成と合成処理(Drum). カウンタに併せて, 8255パルスを立てます
 * @param reg_wav           レジスタ: ドラム波形テーブル
 * @param reg_pulse         レジスタ: 8255パルス値
 * @param addr_drum_on      アドレス: ドラム処理を飛ばす命令
 * @param addr_drum_nr_1    アドレス: ドラム波形テーブルのビット命令
 */
void SD6PLAY_BEEPER_DRUM(int reg_wav, int reg_pulse, int addr_drum_nr_1) __aal_macro __naked
{
    AAL_DEF_VARS;

addr_drum_nr_1: bit(0/*ドラム波形テーブル ビット位置*/, mem[reg_wav]); // 自己書換
    if (nz_jr) { reg_pulse = PULSE_H; }// 12+7+7 or 12+12
    // 計 最短 12+12 = 24(T-states)

    AAL_ENDM;
}


// MARK: play()
u8 sd6play(u32 param) __aal __z88dk_fastcall
{                                                       // HL = data, E = cancellable
    AAL_DEF_VARS;
extern sd6play_waitUntilKeyOff1;
extern sd6play_waitUntilKeyOff2;
extern sd6Play_rep_1;
extern sd6play_setVols, sd6play_loop, sd6play_beeperLoop;
extern sd6play_beeper0_wl, sd6play_slide0, sd6play_modVolSpeed0;
extern sd6play_beeper1_wl;
extern sd6play_beeper2_wl;
extern sd6play_beeper3_wl;
extern sd6play_beeper4_wl;
extern sd6play_beeper5_drumNr;
extern sd6play_beeper1_sync, sd6play_beeper1_syncEnd;
extern sd6play_end;
extern sd6play_restoreSP;
extern tmp;

    // ---------------- 初期化
    push(IX);                                           // z88dk は IX を保存しないようなので念のため
    BANKH_VRAM_MMIO(C);                                 // バンク切替

    // ---- F2, F4 キーが離れるまで待ちます
    A = 0xf9; mem[MMIO_8255_PORTA] = A;                 // A = key strobe 9
sd6play_waitUntilKeyOff1:
    A = mem[MMIO_8255_PORTB]; not(A); A &= KEY9_F2_MASK | KEY9_F4_MASK; jr_nz(sd6play_waitUntilKeyOff1);

    // ---- E == false ならば, キャンセルできないように, 無効な Key Strobe を仕込みます
    E--; if (nz_jr) {                                   // E = キャンセル可能フラグ
        A = 0xfa; mem[MMIO_8255_PORTA] = A;
    }

    // ----- レジスタ, ワーク初期化
    // - 使用レジスタ:
    //   SP             サウンド データ
    //   B  H  IXH      Beeper0 Lead       波長カウンタ0, 音量, 音長カウンタ
    //   C              Beeper1 Lead(echo) 波長カウンタ1
    //   D  L  IXL      Beeper2 Base       波長カウンタ,  音量, 音長カウンタ
    //   B' D' IYH      Beeper3 Chord2     波長カウンタ0, 音量, 音長カウンタ
    //   C'             Beeper4 Chord2     波長カウンタ1
    //         IYL      Beeper5 Drum       音長カウンタ
    //   E  E'          8255 パルス値 PULSE_L or PULSE_H
    //   H'L'           ドラム テーブルへのポインタ兼 Beeper ループカウンタ
    //   A' I           未使用
    mem[sd6play_restoreSP + 1] = SP; SP = HL;           // SP 保存
    xor(A, A);
    C = BEEPER1_WL_CT_VAL; mem[sd6play_beeper1_wl + 1] = A;// Beeper1 の初期化 音長 = 0
    A++;
    IXH = A; IXL = A; IYL = A; IYH = A;                 // Lead, Base, Chord2, Drum 音長カウンタ = 1
    A = 7;         mem[sd6play_modVolSpeed0 + 1] = A;   // Lead 音量変化速度
    A = OPCODE_JR; mem[sd6play_slide0          ] = A;   // Lead スライド無
    HL = ADDR_SD6_REP + 0xff; mem[sd6Play_rep_1 + 1] = HL; // Lead リピート用スタック
#if BEEPER1_SYNC
    mem[sd6play_beeper1_sync            ] = A;          // Beeper1 同期
#endif

    // ---------------- サウンド ループ
    // チェック無しで音長処理へ
sd6play_loop:
    // ---------------- 音符処理 + 波長変更・音量変更処理
    //                wlc slc  tmp-----
    SD6PLAY_NOTE_LEAD(B,  IXH, H, L, HL,
        sd6Play_rep_1,
        sd6play_setVols + 1, sd6play_beeper0_wl, sd6play_slide0, sd6play_modVolSpeed0, sd6play_beeper1_sync, sd6play_end);
    SD6PLAY_NOTE_BASE(D,  IXL, H, L, HL,
        sd6play_setVols,     sd6play_beeper2_wl);
sd6play_setVols: HL = 0x0000;                           // 自己書換 音量
    exx();              //  wlc0 wlc1 wlc01 slc  tmp-----  vol01
        SD6PLAY_NOTE_CHORD2(B,   C,   BC,   IYH, H, L, HL, D,    sd6play_beeper3_wl, sd6play_beeper4_wl);
        SD6PLAY_NOTE_DRUM(                  IYL,    L,     sd6play_beeper5_drumNr);
        HL = ADDR_SD6_DRUM | TEMPO;

        // ---------------- Beeper ループ
sd6play_beeperLoop:
        {
            exx();
        // Lead  は 2 つの beeper を使いますが, 高速化の為, 音量は共通です
        // Chord は 2 つの beeper を使いますが, 高速化の為, 音量は共通です
        E = PULSE_L;        //     wlc pulse vol wl
        SD6PLAY_BEEPER_WAVE_BR(    B,  E,    H,  sd6play_beeper0_wl);// 33 Lead
        SD6PLAY_BEEPER_WAVE_R(     C,  E,    H,  sd6play_beeper1_wl);// 36 Lead(echo)
        SD6PLAY_BEEPER_WAVE_R(     D,  E,    L,  sd6play_beeper2_wl);// 36 Base
        A = E; exx(); E = A;
            SD6PLAY_BEEPER_WAVE_BR(B,  E,    D,  sd6play_beeper3_wl);// 33 Chord0
            SD6PLAY_BEEPER_WAVE_R( C,  E,    D,  sd6play_beeper4_wl);// 36 Chord1
            SD6PLAY_BEEPER_DRUM(HL,    E,    sd6play_beeper5_drumNr);// 24 Drum
            // ---- 波形出力, Beeperループ終了
            A = E; mem[MMIO_8253_CTRL] = A;             // sdSetEnabled(false) で GATE を禁止しても, 音は鳴ります
            L++; jp_nz(sd6play_beeperLoop);             // zf = 1 になる割合は 1/256
        } // ループ計 最短 4+6+33+36+36+12+33+36+24+17+14 = 251(T-states) = 14261(Hz)
        // ---------------- サウンド ループ終了
        exx();

    // Lead(echo): beeper0 の設定を, 数ループ遅延して beeper1 へコピー
    A = IXH; A &= 3; LEAD_ECHO_DELAY(); if (z_jr) {     // 指定ループ遅延してからコピー

#if BEEPER1_SYNC                                        // Beeper1 同期
sd6play_beeper1_sync: jr(sd6play_beeper1_syncEnd); {
        C = BEEPER1_WL_CT_VAL;
        A = OPCODE_JR; mem[sd6play_beeper1_sync] = A;
    }
sd6play_beeper1_syncEnd:
#endif
        A = mem[sd6play_beeper0_wl + 1]; mem[sd6play_beeper1_wl + 1] = A;
    }
    // F2, F4 キーでキャンセル
    A = mem[MMIO_8255_PORTB]; not(A); A &= KEY9_F2_MASK | KEY9_F4_MASK; jp_z(sd6play_loop);
    L = A;                                              // 押下したキーが入ってます
    jr(sd6play_waitUntilKeyOff2);

    // ---------------- 後始末
sd6play_end:
    L = 0x00;
    // F2, F4 キーが離れるまで待ちます
sd6play_waitUntilKeyOff2:
    A = mem[MMIO_8255_PORTB]; not(A); A &= KEY9_F2_MASK | KEY9_F4_MASK ; jr_nz(sd6play_waitUntilKeyOff2);

    // 8253 を元の設定に戻します
    A = MMIO_8253_CT0_MODE3; mem[MMIO_8253_CTRL] = A;

    BANKH_RAM(C);                                       // バンク切替

sd6play_restoreSP: SP = 0x0000;                         // SP 復活
    pop(IX);

    // L = 押下したキー
}
#pragma restore
