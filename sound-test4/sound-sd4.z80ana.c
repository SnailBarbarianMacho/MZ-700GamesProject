/**
 * サウンド - PFM方式6重和音
 * @author Snail Barbarian Macho (NWK)
 */

#include "../src-common/common.h"
#include "../src-common/hard.h"
#include "../src-common/asm-macros.h"
#include "../src-common/z80ana.h"
#include "addr.h"
#include "sound.h"

// ---------------------------------------------------------------- チェック, カスタマイズ, マクロ
// -------- 周波数や8255パルス パラメータのチェック
    STATIC_ASSERT(SD4_B1 < 256 + 1, "valueOverflow"); // 値が大きすぎて鳴らせません
    STATIC_ASSERT(SD4_C6 >= 15,     "valueUnderflow");// 値が小さすぎて音が歪みます

#if 0
    STATIC_ASSERT(SD4_WL(SD_FREQ_C2) < 128.0f, "valueOverflow");    // 値が大きすぎ
    STATIC_ASSERT(SD4_WL(SD_FREQ_C5) > 15.0f,  "valueUnderflow");   // 値が小さすぎて高音が歪みます
    #define MMIO_8253_CT0_PWM   MMIO_8253_CTRL_CT0(MMIO_8253_CTRL_RL_L_MASK, MMIO_8253_CTRL_MODE0_MASK)
#endif

// -------- カスタマイズ

// Lead にて特別な命令に使われる音長. この音長の音符は使えません(4の倍数, makefile で定義). 曲にあわせて設定してください
#ifndef SD4_EXCEPT_LEN
#define SD4_EXCEPT_LEN      0
#endif

#define TEMPO               0       // [0, 128]          128にすると1.5倍速になります.
#define CHORD_VOL_SPEED_R   7       // 0, 1, 3, 7, 15, ...  Chord のエンベロープ音量変化速度
// Lead(Echo)の遅延. 次の3つから選択します
void LEAD_ECHO_DELAY(void) __z80ana_macro __naked
{
    Z80ANA_DEF_VARS;

    //cp(A, 3);     // 遅延小
    //cp(A, 2);     // 遅延中
    A--;          // 遅延大

    Z80ANA_ENDM;
}

// Lead/Chord/Base の音量比率を設定します. 最大 63 未満になるようにしてください
#define LEAD_VOL_MAX        22
#define LEAD_VOL_MIN_DW     8   // ◣の場合の最小音量
#define LEAD_VOL_MIN_UP     5   // ◢の場合の最小音量
#define CHORD_VOL_MAX       21
#define CHORD_VOL_MIN_DW    7   // ◣の場合の最小音量 (※◢の場合の開始音量は 0)
#define BASE_VOL_MAX        20
#define BASE_VOL_MIN_DW     7   // ◣の場合の最小音量 (※◢の場合の開始音量は 0)

// -------- 命令バイナリ
/*
//#define JP          0xc3
#define JR          0x18
#define JR_NZ       0x20
#define JR_Z        0x28
//#define LD_HL_NN    0x21    // ジャンプしない代わりに使用
#define LD_A_N      0x3e    // 相対ジャンプしない代わりに使用
#define OR_A_N      0xf6    // 相対ジャンプしない代わりに使用(cf = 0)
#define INC_A       0x3c
#define DEC_A       0x3d
#define INC_D       0x14
#define DEC_D       0x15
#define INC_E       0x1c
#define DEC_E       0x1d
#define RES_0_L     0x85
#define SET_0_L     0xc5
#define ADD_A_HL    0x86
#define OR_A_HL     0xb6
#define XOR_A_HL    0xae
//#define INC_H       0x24
//#define DEC_H       0x25
//#define INC_L       0x2c
//#define DEC_L       0x2d
//#define OR_A_A      0xb7    // A == 0 ならば zf = 1
//#define OR_A_H      0xb4    // 常に zf = 0
*/

// -------- その他設定
#pragma save
#pragma disable_warning 85          // 引数未使用警告抑止
#pragma disable_warning 59          // 戻値未設定警告抑止

// ---------------------------------------------------------------- 初期化
// -------- sd4 テーブル
static u8 const SD4_DATA_[] = {
    // 音量変換テーブル [0, 63] -> [56, 0]
    // テーブルの最初は 0 でないといけない
    // 合成で and/or/xor を使うな場合のテーブル [0, 63] -> [56, 0]
    0,56,55,54, 54,53,52,51,  50,49,48,47, 47,46,45,44,
    43,42,41,40, 40,39,38,37, 36,35,34,33, 33,32,31,30,
    29,28,27,26, 26,25,24,23, 22,21,20,19, 19,18,17,16,
    15,14,13,12, 12,11,10,9, 8,7,6,5, 4,3,2,1,

// Lead エンベロープ速度逆数テーブル (この値と and して 0 ならば速度変更, 音量 MAX は LEAD_VOL_MAX)
//   |   Lead スライド速度逆数テーブル (最初は未使用. 2番目以降は, この値と and して 0 ならば速度変更)
//   |     |   未使用
//   |     |    |  |
    0x03, 0x00, 0, 0,
    0x07, 0x00, 0, 0,
    0x0f, 0x01, 0, 0,
    0x1f, 0x03, 0, 0,

#include "music/drum1.h"
#include "music/drum2.h"
#include "music/drum3.h"
};


// MARK: init()
void sd4Init(void) __z80ana
{
    Z80ANA_DEF_VARS;
extern _SD4_DATA_;
extern sd4Init_dataLoop;

    // -------- データのコピー
    HL = _SD4_DATA_;
    DE = ADDR_SD4_VTAB;
    BC = SZ_SD4_VTAB + SZ_SD4_TAB;
    ldir();

    DE = ADDR_SD4_DRUM;
    BC = SZ_SD4_DRUM;
    ldir();

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
 * @param reg_wlc               レジスタ: 波長カウンタ (例:B)
 * @param reg_hwl               レジスタ: 半波長値 (例:C)
 * @param reg_wlchwl            レジスタ: wlc と hwl のペア (例:BC)
 * @param reg_vol               レジスタ: 音量 (例:H)
 * @param reg_slc               レジスタ: 音長カウンタ (例:IXH)
 * @param inc_vol               命令: 音量++ (例:INC_H)
 * @param dec_vol               命令: 音量-- (例:DEC_H)
 * @param addr_rep_1            アドレス(自己書換): リピート用スタック
 * @param addr_slide_0          アドレス(自己書換): スライド モード
 * @param addr_mod_start_1      アドレス(自己書換): 音量・波長変更分岐
 * @param addr_mod_vol          アドレス: 音量変更ル―チン
 * @param addr_mod_vol_speed_1  アドレス(自己書換): 音量変更速度
 * @param addr_mod_vol_1        アドレス(自己書換): 音量
 * @param addr_mod_vol_end_1    アドレス(自己書換): 音量end
 * @param addr_mod_vol_inc_0    アドレス(自己書換): 音量加算/減算
 * @param addr_mod_vol_1        アドレス(自己書換): 音量
 * @param addr_mod_wl           アドレス: 波長変更ル―チン
 * @param addr_mod_wl_speed_1   アドレス(自己書換): 波長変更速度
 * @param addr_mod_wl_end_1     アドレス(自己書換): 波長end
 * @param addr_mod_wl_inc_0     アドレス(自己書換): 波長加算/減算
 * @param addr_wl_1             アドレス(自己書換): Beeper 波長セット先
 * @param addr_mod_end          アドレス: 音量・波長変更しない
 * @param addr_end              アドレス: 終了ならばこのアドレスに飛びます
 */
void SD4PLAY_NOTE_LEAD(
    int reg_wlc, int reg_hwl, int reg_wlchwl, int reg_vol, int reg_slc,
    int inc_vol, int dec_vol,
    int addr_rep_1,
    int addr_slide_0,
    int addr_mod_start_1,
    int addr_mod_vol, int addr_mod_vol_speed_1, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_mod_wl,  int addr_mod_wl_speed_1,  int addr_mod_wl_end_1,  int addr_mod_wl_inc_0,
    int addr_wl_1,
    int addr_mod_end,
    int addr_end
) __z80ana_macro __naked
{
    Z80ANA_DEF_VARS;
    extern
        sd4playNoteLead_pop0, sd4playNoteLead_pop,
        sd4playNoteLead_rep_2,
        sd4playNoteLead_initDw, sd4playNoteLead_initDw_1,
        sd4playNoteLead_initEnd, sd4playNoteLead_initEndSlur;

    // -------- 音符処理: 音長カウンタが 0 になったら, 次の音符を読みます
    reg_slc--; if (z) {                                 // 8+10
    sd4playNoteLead_pop:
        pop(reg_wlchwl);                                // 10   注意! reg_hwl=音長, reg_wlc=波長

        // ---- 音長モード別の波長/音長設定
        A = reg_hwl; A += A; if (nc_r) {                // 4+4+7/12
            A += A; reg_slc = A; if (c_r) {             // 4+8+7

                // ---- リピート終了
                cp(A, SD4_EXCEPT_LEN); if (z_r) {
addr_rep_1:     HL = 0x0000; dec(mem[HL]); if (z_r) {   // リピート終了
                        L--; L--; L--; mem[addr_rep_1 + 1] = HL;
                        SP--;
                        goto sd4playNoteLead_pop0;
                    }
                    // リピート中
                    L--; A = mem[HL]; L--; L = mem[HL]; H = A; SP = HL;
                    goto sd4playNoteLead_pop0;
                }

                // ---- 終了コード
                {
                    cp(A, SD4_EXCEPT_LEN + 4); jp_z(addr_end);  // 7+10
                }

                // ---- ◢
                {
                    A = mem[VRAM_TEXT]; ex(AF, AF); mem[MMIO_8253_CT0] = A; ex(AF, AF);// /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす
                    A = reg_wlc; mem[addr_wl_1 + 1] = A;// 4+13 自己書換 波長
                    reg_wlchwl = addr_mod_vol; mem[addr_mod_start_1 + 1] = reg_wlchwl;  // 10+16 自己書換 波長音量変更ジャンプ先
                    reg_wlchwl = (LEAD_VOL_MAX << 8) | inc_vol;// 10
                    reg_vol    = LEAD_VOL_MIN_UP;       // 7    音量
                    goto sd4playNoteLead_initDw;        // 10 スラー無
                }
            }

            // ---- リピート
            cp(A, SD4_EXCEPT_LEN); if (nc) {            // A = 音長
                cp(A, SD4_EXCEPT_LEN + 4 * 7); if (c) {
                    SP--;
                    A -= SD4_EXCEPT_LEN - 8;            // A = 8, 12, ..., 32
                    rrca_n(A, 2);                       // A = 2, 3, ..., 8
                    HL = mem[addr_rep_1 + 1];
                    L++; mem[sd4playNoteLead_rep_2 + 2] = HL; sd4playNoteLead_rep_2: mem[0x0000] = SP; // 自己書 mem[HL] = SP
                    L++; L++; mem[HL] = A;
                    mem[addr_rep_1 + 1] = HL;
                    goto sd4playNoteLead_pop0;
                }
            }

            // ---- 休符 ... 音量設定不要
            {
                SP--;
                xor(A, A); mem[addr_wl_1 + 1] = A;      // 自己書換 波長 = 0(最長)
                reg_wlchwl = addr_mod_end; mem[addr_mod_start_1 + 1] = reg_wlchwl;  // 10+16 自己書換 波長音量変更ジャンプ先
                reg_hwl = A;                            // 半波長 = 0(無音)
                goto sd4playNoteLead_initEndSlur;       // スラー有
            }
        }
        A += A; reg_slc = A; if (c_r) {                 // 4+8+7/12
            cp(A, SD4_EXCEPT_LEN); if (nc) {            // 7    A = 音長
                if (z_r) {                              // 12

                    // ---- スライド無
                    A = OPCODE_JR/*スライド無*/; mem[addr_slide_0 + 0] = A;
                    SP--;
                    goto sd4playNoteLead_pop0;
                }

                cp(A, SD4_EXCEPT_LEN + 4 * 4); if (c) {     // 7+12
                    // ---- スライド(波長)速度設定          // A = 204, 208, 212
                    A -= SD4_EXCEPT_LEN - (SZ_SD4_VTAB + 1);// A = 190, 194, 198
                    reg_wlc = ADDR_SD4_VTAB / 256; reg_hwl = A; A = mem[reg_wlchwl];  // A = 0, 1, 3
                    mem[addr_mod_wl_speed_1 + 1] = A;   // 自己書換 波長速度
                    A = OPCODE_LD_A_N/*スライド有*/; mem[addr_slide_0 + 0] = A;
                    SP--;
                    goto sd4playNoteLead_pop0;
                }
            }

            A = mem[VRAM_TEXT]; ex(AF, AF); mem[MMIO_8253_CT0] = A; ex(AF, AF);// /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす
            addr_slide_0: if (z_r) {                    // 7 自己書換

                // ---- ■ スライド有 (スラー:波長カウンタ リセットや音量の設定無し)
                A = reg_wlc; mem[addr_mod_wl_end_1 + 1] = A;   // 4+13 自己書換 波長end
                // 現在の波長と比較してスライド方向を決める
                A = mem[addr_wl_1 + 1]; cp(A, reg_wlc); A = OPCODE_INC_A; if (nc_r) { A++; /* DEC A */} // 13+4+7+12
                mem[addr_mod_wl_inc_0 + 0] = A;         // 13  自己書換 A++/A--
                reg_wlchwl = addr_mod_wl; mem[addr_mod_start_1 + 1] = reg_wlchwl;  // 10+16 自己書換 波長音量変更ジャンプ先
                A = mem[addr_wl_1 + 1]; A >>= 1; reg_hwl = A; // 13+8+4 半波長
                goto sd4playNoteLead_initEndSlur;       // 10 スラー有
            }

            // ---- ■ スライド無
            {
                A = reg_wlc; mem[addr_wl_1  + 1] = A;   // 自己書換 波長
                reg_wlchwl = addr_mod_end; mem[addr_mod_start_1 + 1] = reg_wlchwl;  // 10+16 自己書換 波長音量変更ジャンプ先
                A >>= 1;   reg_hwl = A;                 // 半波長
                reg_vol  = LEAD_VOL_MAX;                // 自己書換 音量 = max
                goto sd4playNoteLead_initEnd;           // スラー無
            }
        }

        cp(A, SD4_EXCEPT_LEN); if (nc_r) {              // 7+7 A = 音長
            cp(A, SD4_EXCEPT_LEN + 4 * 4); if (c) {     // 7+10

                // ---- エンベロープ(音量)速度設定      // A = 204, 208, 212, 216
                A -= SD4_EXCEPT_LEN - SZ_SD4_VTAB;      // A = 190, 194, 198, 202
                reg_wlc = ADDR_SD4_VTAB / 256; reg_hwl = A; A = mem[reg_wlchwl];  // A = 0, 1, 3, 7
                mem[addr_mod_vol_speed_1 + 1] = A;      // 自己書換 音量速度
                SP--;
sd4playNoteLead_pop0:
                A = mem[VRAM_TEXT]; ex(AF, AF); mem[MMIO_8253_CT0] = A; ex(AF, AF);// /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす
                goto sd4playNoteLead_pop;
            }
        }

        // ---- ◣
        // chord2 処理時間のワーストケースは 262(◣).
        // 140+140くらいになるように分割してコード量削減に努めます
        {
            A = mem[VRAM_TEXT]; ex(AF, AF); mem[MMIO_8253_CT0] = A; ex(AF, AF); // /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす
            A = reg_wlc; mem[addr_wl_1 + 1] = A;        // 4+13 自己書換 波長
            reg_wlchwl = addr_mod_vol; mem[addr_mod_start_1 + 1] = reg_wlchwl;  // 10+16 自己書換 波長音量変更ジャンプ先
            reg_wlchwl = (LEAD_VOL_MIN_DW << 8) | dec_vol;// 10
            reg_vol    = LEAD_VOL_MAX;                  // 7    自己書換 音量 = max
        }
sd4playNoteLead_initDw:
        A >>= 1; mem[sd4playNoteLead_initDw_1 + 1] = A; // 8+13 半波長
        A = reg_wlc;   mem[addr_mod_vol_end_1 + 1] = A; // 7+13 自己書換 音量end = min
        A = reg_hwl;   mem[addr_mod_vol_inc_0 + 0] = A; // 7+13 自己書換 音量変化速度(減算)
sd4playNoteLead_initDw_1: reg_hwl = 0x00;               // 7    自己書換 半波長

sd4playNoteLead_initEnd:
        reg_wlc = 1;                                    // 7 スラー無

sd4playNoteLead_initEndSlur:
        A = mem[VRAM_TEXT]; ex(AF, AF); mem[MMIO_8253_CT0] = A; ex(AF, AF);// /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす
    }
    // ワースト ケース=262 (◣)
    // その他=228 (◢), 231 (■ スライド有)
    Z80ANA_ENDM;
}


// MARK: MOD_LEAD()
/** 音量・波長変更処理(Lead)
 * @param reg_hwl               レジスタ: 半波長値 (例:B)
 * @param reg_vol               レジスタ: 音量 (例:L)
 * @param reg_slc               レジスタ: 音長カウンタ
 * @param addr_mod_start_1      アドレス(自己書換): 音量・波長変更分岐
 * @param addr_mod_vol          アドレス: 音量変更ル―チン
 * @param addr_mod_vol_speed_1  アドレス(自己書換): 音量変更速度
 * @param addr_mod_vol_end_1    アドレス(自己書換): 音量end
 * @param addr_mod_vol_inc_0    アドレス(自己書換): 音量加算/減算
 * @param addr_mod_wl           アドレス: 波長変更ル―チン
 * @param addr_mod_wl_speed_1   アドレス(自己書換): 波長変更速度
 * @param addr_mod_wl_end_1     アドレス(自己書換): 波長end
 * @param addr_mod_wl_inc_0     アドレス(自己書換): 波長加算/減算
 * @param addr_wl_1             アドレス(自己書換): Beeper 波長セット先
 * @param addr_mod_end          アドレス: 音量・波長変更しない
 */
void SD4PLAY_MOD_LEAD(
    int reg_hwl, int reg_vol, int reg_slc,
    int addr_mod_start_1,
    int addr_mod_vol, int addr_mod_vol_speed_1, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_mod_wl,  int addr_mod_wl_speed_1,  int addr_mod_wl_end_1,  int addr_mod_wl_inc_0,
    int addr_wl_1,
    int addr_mod_end
) __z80ana_macro __naked
{
    Z80ANA_DEF_VARS;
addr_mod_start_1: jp(addr_mod_end/*ジャンプ先*/); // 10

    // ---- 音量変更◢◣
addr_mod_vol:
    A = reg_slc; addr_mod_vol_speed_1: A &= 0x00/*音量変更速度*/; if (z_r) {            // 8+7+7
        A = reg_vol; addr_mod_vol_end_1: cp(A, 0x00/*音量end*/);                        // 4+7
        if (nz_r) { addr_mod_vol_inc_0: reg_vol++/*音量加減*/; }                        // 7+4
    }
    goto addr_mod_end;                                  // 10

    // ---- 波長変更処理(■(スライド有))
addr_mod_wl:
    A = reg_slc; addr_mod_wl_speed_1: A &= 0x00/*波長変更速度*/; jr_nz(addr_mod_end); { // 8+7+7
        A = mem[addr_wl_1 + 1]; addr_mod_wl_end_1: cp(A, 0x00/*波長end*/);              // 13+7
        jr_z(addr_mod_end); addr_mod_wl_inc_0: A++; mem[addr_wl_1 + 1] = A;             // 7+4+13
        A >>= 1; reg_hwl = A;                           // 8+4 半波長
    }
    // ワースト ケース=88

    // ---- 何もしない
addr_mod_end:

    Z80ANA_ENDM;
}


// MARK: NOTE_BASE()
/** 音符処理(Base)
 * @param reg_wlc               レジスタ: 波長カウンタ (例:D)
 * @param reg_hwl               レジスタ: 半波長値 (例:E)
 * @param reg_wlwl              レジスタ: wlc と hwl のペア (例:DE)
 * @param reg_vol               レジスタ: 音量 (例:L)
 * @param reg_slc               レジスタ: 音長カウンタ (例:IXL)
 * @param inc_vol               命令: 音量++ (例:INC_L)
 * @param dec_vol               命令: 音量-- (例:DEC_L)
 * @param addr_mod_start_0      アドレス(自己書換): 音量変更開始
 * @param addr_mod_vol_end_1    アドレス(自己書換): 音量変更end
 * @param addr_mod_vol_inc_0    アドレス(自己書換): 音量変更加算/減算
 * @param addr_wl_1             アドレス(自己書換): Beeper 波長
 */
void SD4PLAY_NOTE_BASE(
    int reg_wlc, int reg_hwl, int reg_wlwl, int reg_vol, int reg_slc,
    int inc_vol, int dec_vol,
    int addr_mod_start_0, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_wl_1
) __z80ana_macro __naked
{
    Z80ANA_DEF_VARS;
    extern sd4playNoteBase_fl, sd4playNoteBase_initEnd;

    // -------- 音符処理: 音長カウンタが 0 になったら, 次の音譜を読みます
    reg_slc--; if (z_r) {                               // 8+7
        pop(reg_wlwl);                                  // 10   注意! reg_hwl=音長, reg_wlc=波長

        // ---- 音長モード別の波長/音長設定
        A = reg_hwl; A += A; if (nc_r) {                // 4+4+7/12
            A += A; reg_slc = A; if (c_r) {             // 4+8+4

                // ---- ◢
                A = reg_wlc; mem[addr_wl_1 + 1] = A;    // 4+13 自己書換 波長
                A >>= 1;     reg_hwl = A;               // 8+4  半波長
                reg_vol = 0;                            // 7    音量 = 0
                A = BASE_VOL_MAX; mem[addr_mod_vol_end_1 + 1] = A;  // 7+13 自己書換 音量end
                A = inc_vol;      mem[addr_mod_vol_inc_0 + 0] = A;  // 7+13 自己書換 音量変化速度(加算)
                A = OPCODE_JR_NZ;                       // 7    音量処理有
                goto sd4playNoteBase_initEnd;           // 10
            }

            // ---- 休符 ... 音量設定不要
            SP--;
            xor(A, A); mem[addr_wl_1 + 1] = A;          // 自己書換 波長 = 0(最長)
            reg_hwl = A;                                // 半波長 = 0(無音)
            A = OPCODE_JR;                              // 音量処理無
        } else {
            A += A; reg_slc = A;                        // 4+8  音長 cf:0/1 = ◣■
            A = reg_wlc; mem[addr_wl_1 + 1] = A;        // 4+13 自己書換 波長
            reg_vol = BASE_VOL_MAX;                     // 音量

            if (c_r) {                                  // 12

                // ---- ■
                A >>= 1;     reg_hwl = A;               // 半波長
                A = OPCODE_JR;                          // 音量処理無
            } else {

                // ---- ◣
                rra(A);      reg_hwl = A;               // 4+4 半波長. cf=0 なので srl でなくても OK
                A = BASE_VOL_MIN_DW; mem[addr_mod_vol_end_1 + 1] = A;  // 7+13 自己書換 音量end
                A = dec_vol;         mem[addr_mod_vol_inc_0 + 0] = A;  // 7+13 自己書換 音量変化速度(減算)
                A = OPCODE_JR_NZ;                       // 7 音量処理有
            }
        }
sd4playNoteBase_initEnd:
        mem[addr_mod_start_0 + 0] = A;                  // 13  自己書換 分岐でジャンプ/ジャンプしない
        reg_wlc = 1;                                    // 7   波長カウンタ. すぐに再設定されます
        // ワースト ケース=149(◢), 148(◣)
        A = mem[VRAM_TEXT]; ex(AF, AF); mem[MMIO_8253_CT0] = A; ex(AF, AF);// /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす
    }

    Z80ANA_ENDM;
}


// MARK: MOD_BASE()
/** 音量変更処理(Base)
 * @param reg_vol               レジスタ: 音量 (例:L)
 * @param reg_slc               レジスタ: 音長カウンタ (例:IXL)
 * @param addr_mod_start_0      アドレス(自己書換): 音量変更開始
 * @param addr_mod_vol_end_1    アドレス(自己書換): 音量変更end
 * @param addr_mod_vol_inc_0    アドレス(自己書換): 音量変更加算/減算
 */
void SD4PLAY_MOD_BASE(
    int reg_vol, int reg_slc,
    int addr_mod_start_0, int addr_mod_vol_end_1, int addr_mod_vol_inc_0
) __z80ana_macro __naked
{
    Z80ANA_DEF_VARS;
    extern sd4playModBase_end;

    // ---- 音量変更◢◣
    A = reg_slc; A &= 0x01;
    addr_mod_start_0: jr_nz(sd4playModBase_end)/*音量変更有無*/;// 8+7+7    自己書換
    A = reg_vol; addr_mod_vol_end_1: cp(A, 0x00/*音量end*/);    // 4+7      自己書換
    if (nz_r) { addr_mod_vol_inc_0: reg_vol++/*音量加減*/; }    // 7+4      自己書換
    // ワースト ケース=44, ベスト ケース=27

    // ---- 何もしない
sd4playModBase_end:

    Z80ANA_ENDM;
}


// MARK: NOTE_CHORD2()
/** 音符処理 (Chord2)
 * - コードは音長と音量が兼用です
 * @param reg_wl0               レジスタ: 波長0 / 波長カウンタ(例:B)
 * @param reg_wl1               レジスタ: 波長1 / 半波長(例:C)
 * @param reg_wl01              レジスタ: (reg_wl0 << 8) | reg_wl1(例:BC)
 * @param reg_vol               レジスタ: 音量 (例:D)
 * @param reg_slc               レジスタ: 音長カウンタ (例:IYH)
 * @param inc_vol               命令: 音量++ (例:INC_D)
 * @param dec_vol               命令: 音量-- (例:DEC_D)
 * @param addr_mod_wl_start_0   アドレス(自己書換): 波長変更開始
 * @param addr_mod_wl0_1        アドレス(自己書換): 波長0
 * @param addr_mod_wl1_1        アドレス(自己書換): 波長1
 * @param addr_mod_vol_start_0  アドレス(自己書換): 音量変更開始
 * @param addr_mod_vol_end_1    アドレス(自己書換): 音量変更end
 * @param addr_mod_vol_inc_0    アドレス(自己書換): 音量変更加算/減算
 * @param addr_beep_wl_1        アドレス(自己書換): 波長
 */
void SD4PLAY_NOTE_CHORD2(
    int reg_wl0, int reg_wl1, int reg_wl01, int reg_vol, int reg_slc,
    int inc_vol, int dec_vol,
    int addr_mod_wl_start_0,  int addr_mod_wl0_1,     int addr_mod_wl1_1,
    int addr_mod_vol_start_0, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_beep_wl_1
) __z80ana_macro __naked
{
    Z80ANA_DEF_VARS;
    extern sd4playNoteChord2_initDw, sd4playNoteChord2_initEnd;

    // -------- 音符処理: 音長カウンタが 0 になったら, 次の音譜を読みます
    reg_slc--; if (z) {                                 // 8+10
        SP--; pop(AF);                                  // 6+10 A = 音長

        // ---- 音長モード別の波長/音長設定
        A += A; if (nc_r) {                             // 4+7/12
            A += A; reg_slc = A; if (c_r) {             // 4+8+7

                // ---- ◢
                // chord2 処理時間のワーストケースは 205(◢).
                // 120+120くらいになるように分割してコード量削減に努めます
                pop(reg_wl01);                          // 10
                A = reg_wl0; mem[addr_mod_wl0_1 + 1] = A;   // 4+13 自己書換 波長0
                A = reg_wl1; mem[addr_mod_wl1_1 + 1] = A;   // 4+13 自己書換 波長1
                reg_vol = 0;                            // 4 音量 = 0
                reg_wl01 = (CHORD_VOL_MAX << 8) | inc_vol;  // 10 音量end|音量変化速度(加算)
                goto sd4playNoteChord2_initDw;          // 10
            }

            // ---- 休符 ... 音量設定不要
            xor(A, A); mem[addr_beep_wl_1 + 1] = A;     // 自己書換 波長0 = 0(最長)
            reg_wl1 = A;                                // 半波長 = 0(無音)
            A = OPCODE_JR;                              // 波長処理無(音量処理も無)
            goto sd4playNoteChord2_initEnd;
        }
        A += A; reg_slc = A;                            // 4+8  音長 cf:0/1 = ◣■
        pop(reg_wl01);                                  // 10
        A = reg_wl0; mem[addr_mod_wl0_1 + 1] = A;       // 4+13 自己書換 波長0
        A = reg_wl1; mem[addr_mod_wl1_1 + 1] = A;       // 4+13 自己書換 波長1
        reg_vol = CHORD_VOL_MAX;                        // 7    音量
        if (c) {                                        // 10

            // ---- ■
            A = OPCODE_JR; mem[addr_mod_vol_start_0 + 0] = A;  // 音量分岐無
            A = OPCODE_LD_A_N;                          // 波長分岐有
        } else {

            // ---- ◣
            reg_wl01 = (CHORD_VOL_MIN_DW << 8) | dec_vol;// 10 音量end|音量変化速度(減算)
sd4playNoteChord2_initDw:
            A = mem[VRAM_TEXT]; ex(AF, AF); mem[MMIO_8253_CT0] = A; ex(AF, AF);// /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす
            A = reg_wl0; mem[addr_mod_vol_end_1   + 1] = A; // 4+13 自己書換 音量end
            A = reg_wl1; mem[addr_mod_vol_inc_0   + 0] = A; // 4+13 自己書換 音量変化速度
            A = OPCODE_JR_NZ;   mem[addr_mod_vol_start_0 + 0] = A; // 7+13 音量処理有
            A = OPCODE_LD_A_N;                              // 7    波長処理有
        }
sd4playNoteChord2_initEnd:
        mem[addr_mod_wl_start_0 + 0] = A;               // 13 自己書換 波長分岐でジャンプ/ジャンプしない
        reg_wl0 = 1;                                    // 7  波長カウンタ. すぐに再設定されます
        // ワーストケース=205(◢),210(◣)
        A = mem[VRAM_TEXT]; ex(AF, AF); mem[MMIO_8253_CT0] = A; ex(AF, AF);// /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす
    }

    Z80ANA_ENDM;
}


// MARK: MOD_CHORD2()
/** 音量・波長変更処理(Chord2)
 * - 2つの波長を交互にアルペジオ風に流します. 波長カウンタのリセットはしません
 * @param reg_hwl               レジスタ: 半波長値 (例:C)
 * @param reg_vol               レジスタ: 音量 (例:D)
 * @param reg_slc               レジスタ: 音長カウンタ (例:IYH)
 * @param addr_mod_wl_start_0   アドレス(自己書換): 波長変更開始
 * @param addr_mod_wl0_1        アドレス(自己書換): 波長0
 * @param addr_mod_wl1_1        アドレス(自己書換): 波長1
 * @param addr_mod_vol_start_0  アドレス(自己書換): 音量変更開始
 * @param addr_mod_vol_end_1    アドレス(自己書換): 音量変更end
 * @param addr_mod_vol_inc_0    アドレス(自己書換): 音量変更加算/減算
 * @param addr_beep_wl_1        アドレス(自己書換): 波長
 */
void SD4PLAY_MOD_CHORD2(
    int reg_hwl, int reg_vol, int reg_slc,
    int addr_mod_wl_start_0,  int addr_mod_wl0_1,     int addr_mod_wl1_1,
    int addr_mod_vol_start_0, int addr_mod_vol_end_1, int addr_mod_vol_inc_0,
    int addr_beep_wl_1
) __z80ana_macro __naked
{
    Z80ANA_DEF_VARS;
    extern sd4playModChord_end;

    // ---- 無音ならば処理は不要
    addr_mod_wl_start_0: jr(sd4playModChord_end)/*発音有無*/;           // 7            自己書換

    // ---- 波長変更(アルペジオ)◢■◣
    A = reg_slc; A &= 2; addr_mod_wl0_1: A = 0x00/*波長0*/;             // 8+7+7 cf=0   自己書換
    if (z_r) {           addr_mod_wl1_1: A = 0x00/*波長1*/;}            // 7+7          自己書換
    mem[addr_beep_wl_1 + 1] = A; rra(A); reg_hwl = A;                   // 13+4+4 rra時cf=0

    // ---- 音量変更◢◣
    A = reg_slc; A &= 1;
    addr_mod_vol_start_0: jr_nz(sd4playModChord_end)/*音量変更有無*/;   // 8+7+7 自己書換
    A = reg_vol; addr_mod_vol_end_1: cp(A, 0x00/*音量end*/);            // 4+7   自己書換
    if (nz_r) { addr_mod_vol_inc_0: reg_vol++/*音量加減*/; }            // 7+4   自己書換
    // ワースト ケース=108, ベスト ケース=39

sd4playModChord_end:

    Z80ANA_ENDM;
}


// MARK: NOTE_DRUM()
/** 音譜処理(Drum)
 * @param reg_sl            レジスタ: 音長カウンタ
 * @param reg_tmp           レジスタ: テンポラリ
 * @param addr_drum_ct_1    アドレス: 自己書換 カウンタ変更命令
 * @param addr_drum_nr_1    アドレス: 自己書換 ドラム番号
 * @param addr_drum_add_0   アドレス: 自己書換 ドラム音を鳴らす
 */
void SD4PLAY_NOTE_DRUM(int reg_sl, int reg_tmp,
    int addr_drum_ct_1, int addr_drum_nr_1, int addr_drum_add_0) __z80ana_macro __naked
{
    Z80ANA_DEF_VARS;
    extern sd4PlayModDrum_ct_1, sd4PlayModDrum_end;

    reg_sl--; if (z) {                                  // 8+10
        SP--; pop(AF);                                  // 6+10
        reg_tmp = A; A &= 0xfc; reg_sl = A;             // 4+7+8 音長
        A ^= reg_tmp;                                   // 4     reg_tmp と A を xor すると, ドラム番号(0-3)だけ残る
        if (nz_r) {                                     // 7     ドラム 0 は無音
            A += (ADDR_SD4_DRUM / 256 - 1); mem[addr_drum_nr_1 + 1] = A;// 7+13 自己書換 ドラム番号1～3
            A = mem[VRAM_TEXT]; ex(AF, AF); mem[MMIO_8253_CT0] = A; ex(AF, AF);// /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす
            A = OPCODE_RES_0_L; mem[addr_drum_ct_1  + 1] = A;       // 7+13 自己書換 ドラム カウンタ調整
            A = OPCODE_OR_HL;   mem[addr_drum_add_0 + 0] = A;       // 7+13 自己書換 ドラム音を鳴らす
            A = 2;              mem[sd4PlayModDrum_ct_1 + 1] = A;   // 7+13 自己書換 ドラム カウンタ
        }
        // ワーストケース=74+70=154
        goto sd4PlayModDrum_end;
    }

sd4PlayModDrum_ct_1: A = 0x00/* ドラム カウンタ */; A |= A; if (nz_r) { // 7+4+7
        A--; mem[sd4PlayModDrum_ct_1 + 1] = A;          // 4+13 自己書換 ドラムカウンタ
        if (nz_r) {
            A = OPCODE_SET_0_L; mem[addr_drum_ct_1  + 1] = A;   // 7+7+13+10 自己書換 ドラム カウンタ調整
        } else {
            xor(A, A);          mem[addr_drum_add_0 + 0] = A;   // 12+7+13 自己書換 ドラム音を鳴らす
        }
        // ワーストケース=72
    }
sd4PlayModDrum_end:

    Z80ANA_ENDM;
}


// -------------------------------- 波長処理 マクロ
// MARK: BEEPER_WAVE_R()
/** 波形生成と合成処理(Lead, Chord, Base). 波長カウンタに併せて, 8255パルスを立てる. 波長カウンタが 0 になったら再設定します
 * @param reg_wlc       レジスタ: 波長カウンタ (例:D)
 * @param reg_hwl       レジスタ: 半波長値 (例:E)
 * @param reg_vol       レジスタ: 音量 (例:L)
 * @param addr_wl_1     アドレス(自己書換): 波長をセットする
 */
void SD4PLAY_BEEPER_WAVE_R(int reg_wlc, int reg_hwl, int reg_vol, int addr_wl_1) __z80ana_macro __naked
{
    Z80ANA_DEF_VARS;

    // 波長処理
    reg_wlc--; if (z) {                                 // 4+10
        addr_wl_1: reg_wlc = 0x00;/* 波長を設定したらその場は音量処理は不要 */
    } else {
        // 音量処理
        A = reg_wlc; cp(A, reg_hwl);                    // 4+4
        if (c_r) { ex(AF, AF); A += reg_vol; ex(AF, AF); }// 7+4+4+4
    }
    // ワースト ケース=41

    Z80ANA_ENDM;
}


// MARK: BEEPER_WAVE_BR()
/** 波長カウンタが B ならば, 少し速くなるぞ!
 * @param reg_wlc       レジスタ: 波長カウンタ (Bで固定)
 * @param reg_hwl       レジスタ: 半波長値 (例:C)
 * @param reg_vol       レジスタ: 音量 (例:H)
 * @param addr_wl_1     アドレス(自己書換): 波長をセットする
 */
void SD4PLAY_BEEPER_WAVE_BR(int reg_wl, int reg_hwl, int reg_vol, int addr_wl_1) __z80ana_macro __naked
{
    Z80ANA_DEF_VARS;
    Z80ANA_LOCAL(sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd);
    extern sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd;

    // 波長処理
    djnz(B, sd4play_beeperWaveBr); {                    // 13
        addr_wl_1: reg_wl = 0x00; jr(sd4play_beeperWaveBrEnd);/* 波長を設定したらその場は音量処理は不要 */
    }
    sd4play_beeperWaveBr: {
        // 音量処理. vol < 2 ならば, 波長カウンタの値に依らず cf が立たない
        A = reg_wl; cp(A, reg_hwl);                     // 4+4
        if (c_r) { ex(AF, AF); A += reg_vol; ex(AF, AF); }// 7+4+4+4
    }
    sd4play_beeperWaveBrEnd:
    // ワースト ケース=40

    Z80ANA_ENDM;
}


// MARK: BEEPER_WAVE_B()
/** 音量レジスタが無い場合
 * @param reg_wlc       レジスタ: 波長カウンタ (Bで固定)
 * @param reg_hwl       レジスタ: 半波長値 (例:C)
 * @param addr_vol_1    アドレス(自己書換): 音量
 * @param addr_wl_1     アドレス(自己書換): 波長
 */
void SD4PLAY_BEEPER_WAVE_B(int reg_wl, int reg_hwl, int addr_vol_1, int addr_wl_1) __z80ana_macro __naked
{
    Z80ANA_DEF_VARS;
    Z80ANA_LOCAL(sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd);
    extern sd4play_beeperWaveBr, sd4play_beeperWaveBrEnd;

    // 波長処理
    djnz(B, sd4play_beeperWaveBr); {                    // 13
        addr_wl_1: reg_wl = 0x00; jr(sd4play_beeperWaveBrEnd);/* 波長を設定したらその場は音量処理は不要 */
    }
    sd4play_beeperWaveBr: {
        // 音量処理. vol < 2 ならば, 波長カウンタの値に依らず cf が立たない
        A = reg_wl; cp(A, reg_hwl);                     // 4+4
        if (c_r) { ex(AF, AF); addr_vol_1:A += 0x00; ex(AF, AF); }// 7+4+7+4
    }
    sd4play_beeperWaveBrEnd:
    // ワースト ケース=43

    Z80ANA_ENDM;
}


// MARK: BEEPER_DRUM()
/** 波形生成と合成処理(Drum). カウンタに併せて, 8255パルスを立てます
 * @param reg_wav_h         レジスタ: ドラム波形テーブル(例: H)
 * @param reg_wav_l         レジスタ: ドラム波形テーブル(例: L)
 * @param reg_wav_hl        レジスタ: ドラム波形テーブル(例: HL)
 * @param reg_ct            レジスタ: カウンタ(例: E)
 * @param addr_drum_ct_1    アドレス: 自己書換 カウンタ変更命令
 * @param addr_drum_nr_1    アドレス: 自己書換 ドラム番号
 * @param addr_drum_add_0   アドレス: 自己書換 ドラム音を鳴らす
 */
void SD4PLAY_BEEPER_DRUM(
    int reg_wav_h, int reg_wav_l, int reg_wav_hl, int reg_ct,
    int addr_drum_ct_1, int addr_drum_nr_1, int addr_drum_add_0) __z80ana_macro __naked
{
    Z80ANA_DEF_VARS;
    reg_wav_l = reg_ct; addr_drum_ct_1: set(0, reg_wav_l);              // 4+8  自己書換
    addr_drum_nr_1: reg_wav_h = (ADDR_SD4_DRUM - 1) / 256 + 1/*ドラム番号*/; // 7    自己書換
    ex(AF, AF); addr_drum_add_0:nop()/*ドラム音の合成*/;                // 4+4  自己書換
    // AF レジスタは裏のまま
    // ワーストケース=27

    Z80ANA_ENDM;
}


// MARK: play()
u8 sd4play(u32 param) __z80ana __z88dk_fastcall
{                                                       // HL = data, E = cancellable
    Z80ANA_DEF_VARS;
extern sd4play_waitUntilKeyOff1;
extern sd4play_waitUntilKeyOff2;
extern sd4play_vblk0, sd4play_vblk1;
extern sd4play_setVols, sd4play_loop, sd4play_beeperLoop;
extern sd4play_rep_1, sd4play_leadSlide_0;
extern sd4play_modLead_start;
extern sd4play_modLead_vol, sd4play_modLead_volSpeed_1, sd4play_modLead_vol_1, sd4play_modLead_volEnd_1, sd4play_modLead_volInc_0;
extern sd4play_modLead_wl,  sd4play_modLead_wlSpeed_1, sd4play_modLead_wlEnd_1, sd4play_modLead_wlInc_0;
extern sd4play_modLead_end;
extern sd4play_modBase_start, sd4play_modBase_volEnd_1, sd4play_modBase_volInc_0;
extern sd4play_modChord_wlStart,  sd4play_modChord_wl0,    sd4play_modChord_wl1;
extern sd4play_modChord_volStart, sd4play_modChord_volEnd, sd4play_modChord_volInc;
extern sd4play_beeper0_wl_1;
extern sd4play_beeper1_wl_1;
extern sd4play_beeper2_wl_1;
extern sd4play_beeper3_drumCt, sd4play_beeper3_drumNr, sd4play_beeper3_drumAdd;
extern sd4play_end;
extern sd4play_restoreSP;
extern tmp;

    // ---------------- 初期化
    push(IX);                                           // z88dk は IX を保存しないようなので念のため
    BANKH_VRAM_MMIO(C);                                 // バンク切替

    // ---- F2, F4 キーが離れるまで待ちます
    A = 0xf9; mem[MMIO_8255_PORTA] = A;                 // A = key strobe 9
sd4play_waitUntilKeyOff1:
    A = mem[MMIO_8255_PORTB]; not(A); A &= KEY9_F2_MASK | KEY9_F4_MASK; jr_nz(sd4play_waitUntilKeyOff1);

    // ---- E == false ならば, キャンセルできないように, 無効な Key Strobe を仕込みます
    E--; if (nz_r) {                                    // E = キャンセル可能フラグ
        A = 0xfa; mem[MMIO_8255_PORTA] = A;
    }

    // ---- 8253 初期化
    A = MMIO_8253_CTRL_CT0(MMIO_8253_CTRL_RL_L_MASK, MMIO_8253_CTRL_MODE0_MASK);
    mem[MMIO_8253_CTRL] = A;

#if 0   // PWM パルス TEST
    L00: A = mem[VRAM_TEXT];
    A = 56/2; mem[MMIO_8253_CT0] = A;
    ex(mem[SP], HL); ex(mem[SP], HL); ex(mem[SP], HL); ex(mem[SP], HL);
    jr(L00);
#endif

    // ----- レジスタ, ワーク初期化
    // - 使用レジスタ:
    //   SP             サウンド データ
    //   H  L  D, IXH   Beeper0 Lead  波長カウンタ, 半波長, 音量, 音長カウンタ
    //   B  C  E, IXL   Beeper1 Base  波長カウンタ, 半波長, 音量, 音長カウンタ
    //   B' C' D' IYH   Beeper2 Chord 波長カウンタ, 半波長, 音量, 音長カウンタ
    //            IYL   Beeper3 Drum                              音長カウンタ
    //   A'             音量
    //   E'             Beeper ループカウンタ
    //   HL             ドラム テーブルへのポインタ, 音量変換テーブルへのポインタ
    //   I              未使用
    // ---- スタックポインタ設定
    mem[sd4play_restoreSP + 1] = SP; SP = HL;           // SP 保存
    xor(A, A);
    mem[sd4PlayModDrum_ct_1        + 1] = A;            // ドラム カウンタ   = 0
    mem[sd4play_beeper3_drumAdd    + 0] = A;            // ドラム音を鳴らす  = 0(nop)
    mem[sd4play_modLead_volSpeed_1 + 1] = A;            // Lead 音量変化速度 = 0

    A++;
    IXH = A; IXL = A; IYL = A; IYH = A;                 // Lead, Base, Chord, Drum 音長カウンタ = 1
    HL = ADDR_SD4_REP + 0xff; mem[sd4play_rep_1 + 1] = HL; // Lead リピート用スタック
    A = OPCODE_JR; mem[sd4play_leadSlide_0 + 0] = A;    // Lead スライド無

    // ---- 同期取ります
    xor(A, A); HL = MMIO_8255_PORTC;
sd4play_vblk1:  or(A, mem[HL]); jp_p(sd4play_vblk1);    // /VBLK=H (表示中) になるまで待つ
    A = mem[VRAM_TEXT];                                 // /VBLK=L (ブランク中) になるまで待つ
    xor(A, A); ex(AF, AF);                              // 最初は無音

    // ---------------- サウンド ループ
    // チェック無しで音長処理へ
sd4play_loop:
    A = mem[VRAM_TEXT]; ex(AF, AF); mem[MMIO_8253_CT0] = A; ex(AF, AF);// /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす

    // ---------------- 音符処理 + 波長変更・音量変更処理
    //                wl hwl   vol sl   vol++  vol--
    SD4PLAY_NOTE_LEAD(H, L, HL, D, IXH, OPCODE_INC_D, OPCODE_DEC_D, sd4play_rep_1, sd4play_leadSlide_0,
        sd4play_modLead_start,
        sd4play_modLead_vol, sd4play_modLead_volSpeed_1, sd4play_modLead_volEnd_1, sd4play_modLead_volInc_0,
        sd4play_modLead_wl,  sd4play_modLead_wlSpeed_1,  sd4play_modLead_wlEnd_1,  sd4play_modLead_wlInc_0,
        sd4play_beeper0_wl_1,
        sd4play_modLead_end,
        sd4play_end);
    SD4PLAY_NOTE_BASE(B, C, BC, E, IXL, OPCODE_INC_E, OPCODE_DEC_E,
        sd4play_modBase_start, sd4play_modBase_volEnd_1, sd4play_modBase_volInc_0,
        sd4play_beeper1_wl_1);
    SD4PLAY_MOD_LEAD(    L,     D, IXH,
        sd4play_modLead_start,
        sd4play_modLead_vol, sd4play_modLead_volSpeed_1, sd4play_modLead_volEnd_1, sd4play_modLead_volInc_0,
        sd4play_modLead_wl,  sd4play_modLead_wlSpeed_1,  sd4play_modLead_wlEnd_1,  sd4play_modLead_wlInc_0,
        sd4play_beeper0_wl_1,
        sd4play_modLead_end);
    SD4PLAY_MOD_BASE(           E, IXL,
        sd4play_modBase_start, sd4play_modBase_volEnd_1, sd4play_modBase_volInc_0);
    A = mem[VRAM_TEXT]; ex(AF, AF); mem[MMIO_8253_CT0] = A; ex(AF, AF);// /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす
    exx();             //  wl0 wl1 wl01 vol sl   vol++  vol--
        SD4PLAY_NOTE_CHORD2(B,  C,  BC,  D,  IYH, OPCODE_INC_D, OPCODE_DEC_D,
            sd4play_modChord_wlStart,  sd4play_modChord_wl0, sd4play_modChord_wl1,
            sd4play_modChord_volStart, sd4play_modChord_volEnd, sd4play_modChord_volInc,
            sd4play_beeper2_wl_1);
        SD4PLAY_MOD_CHORD2(    C,     D, IYH,
            sd4play_modChord_wlStart,  sd4play_modChord_wl0, sd4play_modChord_wl1,
            sd4play_modChord_volStart, sd4play_modChord_volEnd, sd4play_modChord_volInc,
            sd4play_beeper2_wl_1);
        SD4PLAY_NOTE_DRUM(             IYL,    L,
            sd4play_beeper3_drumCt, sd4play_beeper3_drumNr, sd4play_beeper3_drumAdd);
        E = TEMPO;
        A = mem[VRAM_TEXT]; ex(AF, AF); mem[MMIO_8253_CT0] = A;// /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす

    // ---------------- Beeper ループ
sd4play_beeperLoop:
        {
            xor(A, A); ex(AF, AF);
            exx();
        //                       wcl hwl  vol
        SD4PLAY_BEEPER_WAVE_R(     H,  L,  D, sd4play_beeper0_wl_1);// Lead
        SD4PLAY_BEEPER_WAVE_BR(    B,  C,  E, sd4play_beeper1_wl_1);// Base
        exx();
            SD4PLAY_BEEPER_WAVE_BR(B,  C,  D, sd4play_beeper2_wl_1);// Chord2
            SD4PLAY_BEEPER_DRUM(H, L, HL, E,
                sd4play_beeper3_drumCt, sd4play_beeper3_drumNr, sd4play_beeper3_drumAdd);// Drum
                // AF レジスタは裏のまま

            // ---- 波形出力, Beeperループ終了
            H = ADDR_SD4_VTAB / 256; L = A;             // A = 波形
            A = mem[VRAM_TEXT]; A = mem[HL]; mem[MMIO_8253_CT0] = A;// 7+13 /VBLK=L (ブランク中) になるまで待ち, 音を鳴らす
            E++; jp_nz(sd4play_beeperLoop);             // 4+10
        }
    // ---------------- サウンド ループ終了
        exx();                                          // 4
    ex(AF, AF);                                         // 4
    // F2, F4 キーでキャンセル
    A = mem[MMIO_8255_PORTB]; not(A); A &= KEY9_F2_MASK | KEY9_F4_MASK; jp_z(sd4play_loop);// 13+8+7+10 計88
    L = A;                                              // 押下したキーが入ってます
    jr(sd4play_waitUntilKeyOff2);

    // ---------------- 後始末
sd4play_end:
    L = 0x00;
    // F2, F4 キーが離れるまで待ちます
sd4play_waitUntilKeyOff2:
    A = mem[MMIO_8255_PORTB]; not(A); A &= KEY9_F2_MASK | KEY9_F4_MASK ; jr_nz(sd4play_waitUntilKeyOff2);

    // 8253 を元の設定に戻します
    A = MMIO_8253_CT0_MODE3; mem[MMIO_8253_CTRL] = A;

    BANKH_RAM(C);                                       // バンク切替

sd4play_restoreSP: SP = 0x0000;                         // SP 復活
    pop(IX);

    // L = 押下したキー
}
#pragma restore
