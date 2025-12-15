/**
 * 入力
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "../../../../src-common/aal80.h"
#include "../../../../src-common/asm-macros.h"
#include "assert.h"
#include "input.h"

// ---------------------------------------------------------------- 変数
u8          input_;
static u8   input_old_;              // 1フレーム前の input_
u8          input_trg_;
u8          input_joy_;
u8          input_joy_mode_;
u8          input_mz1x03_sensitivity_;

// ---------------------------------------------------------------- システム
void inputInit(void) __z88dk_fastcall
{
    input_     = 0;
    input_old_ = 0;
    input_trg_ = 0;
    input_joy_mode_ = INPUT_JOY_MODE_AM7J_DETECTING;
    input_mz1x03_sensitivity_ = 0;
}


static const u8 INPUT_TAB_[] = {
    // strobe, count, mask0, input0, mask1, input1, ...
    0xf1,      1, /* Z */0x40, INPUT_MASK_A,
    0xf2,      5, /* S */0x20, INPUT_MASK_D, /* U */0x08, INPUT_MASK_U, /* V */0x04, INPUT_MASK_A, /* W */0x02, INPUT_MASK_U, /* X */0x01, INPUT_MASK_B,
    0xf3,      3, /* J */0x40, INPUT_MASK_B, /* K */0x20, INPUT_MASK_R, /* M */0x08, INPUT_MASK_D,
    0xf4,      4, /* A */0x80, INPUT_MASK_L, /* B */0x40, INPUT_MASK_B, /* D */0x10, INPUT_MASK_R, /* H */0x01, INPUT_MASK_L,
    0xf6,      1, /* Space */0x10, INPUT_MASK_A,
    0xf7,      4, /* ↑ */0x20, INPUT_MASK_U, /* ↓ */0x10, INPUT_MASK_D, /* → */0x08, INPUT_MASK_R, /* ← */0x04, INPUT_MASK_L,
    0xf9,      4, /* F1 */0x80, INPUT_MASK_START, /* F2 */0x40, INPUT_MASK_CANCEL, /* F3 */0x20, INPUT_MASK_START, /* F4 */0x10, INPUT_MASK_CANCEL,
    0,
};


/** キー入力をして, _input_ に保存します */
static void inputKey_(void) __z88dk_fastcall __naked
{
__asm
    BANKH_VRAM_MMIO C           // バンク切替

    // -------- 古いデータを保存, 準備
    ld      A, (_input_)
    ld      (_input_old_), A

    ld      HL, 0 + _INPUT_TAB_
    ld      DE, 0 + MMIO_8255_PORTA
    ld      C,  0x00           // input_

    // -------- Write strobe
STROBE_LOOP:
    ld      A,  (HL)             // strobe
    or      A
    jr      z,  STROBE_LOOP_END
    inc     HL
    ld      (DE), A             // strobe
    inc     E                   // DE = MMIO_8255_PORTB
    ld      A,  (DE)            // key data
    ld      E,  A

    ld      B,  (HL)            // count
    inc     HL

    // -------- Read key
KEY_LOOP:
    ld      A,  (HL)            // 該当ビットが 0 になる
    inc     HL
    and     A,  E
    jp      nz, KEY_LOOP_END
    ld      A,  (HL)
    or      A,  C               // input_
    ld      C,  A               // input_
KEY_LOOP_END:
    inc     HL
    djnz    B,  KEY_LOOP
    ld      E,  0 + MMIO_8255_PORTA & 0xff
    jp      STROBE_LOOP

STROBE_LOOP_END:
    ld      A,  C
    ld      (_input_), A
    BANKH_RAM C                 // バンク切替
    ret
__endasm;
}


/** AM7J 入力し, input_joy_ に返します */
static void inputAM7J_(void) __z88dk_fastcall __naked
{
__asm
    // broken: HL,DE,AF     BCや裏レジスタは保存されます
    BANKH_VRAM_MMIO C           // バンク切替

    // -------- JA2 立下がり検出
    ld      HL, 0 + MMIO_ETC
    ld      DE, 0x0e00 | MMIO_ETC_JA2_MASK

    // H になるまで待つ. 90 T state 以上待って L のままならば無視
    ld      A,  E               //          E = MMIO_ETC_JA2_MASK
    and     A,  (HL)            // 7        if (MMIO_ETC & JA2 == 0) { loop }
    jp      nz, AM7J_H_DETECTED // 10/10
    ld      A,  E               // 4
    and     A,  (HL)            // 7
    jp      nz, AM7J_H_DETECTED // 10/10
    ld      A,  E               // 4
    and     A,  (HL)            // 7
    jp      nz, AM7J_H_DETECTED // 10/10
    ld      A,  E               // 4
    and     A,  (HL)            // 7
    jp      nz, AM7J_H_DETECTED // 10/10
    ld      A,  E               // 4
    and     A,  (HL)            // 7
    jp      nz, AM7J_H_DETECTED // 10/10
    ld      A,  E               // 4
    and     A,  (HL)            // 7
    // 小計 7+10+ 4+7+10+ 4+7+10+ 4+7+10+ 4+7+10+ 4+4+7 = 116
    jr      z, AM7J_NOT_DETECTED // 12/7

AM7J_H_DETECTED:
    // L になるまで待つ. 98 T state 以上待って H のままならば無視
    and     A, (HL)             // 7        if (MMIO_ETC & JA2 != 0) { loop }
    jr      z, AM7J_L_DETECTED  // 12/7
    and     A, (HL)             // 7
    jr      z, AM7J_L_DETECTED  // 12/7
    and     A, (HL)             // 7
    jr      z, AM7J_L_DETECTED  // 12/7
    and     A, (HL)             // 7
    jr      z, AM7J_L_DETECTED  // 12/7
    and     A, (HL)             // 7
    jr      z, AM7J_L_DETECTED  // 12/7
    and     A, (HL)             // 7
    jr      z, AM7J_L_DETECTED  // 12/7
    and     A, (HL)             // 7
    // 小計 10+ 7+7+ 7+7+ 7+7+ 7+7+ 7+7+ 7+7+ 7 = 101
    jr      z, AM7J_L_DETECTED  // 12/7

    // -------- AM7J 正しく読み取れなかった
AM7J_NOT_DETECTED: // 非認識
    ld      A,  0 + INPUT_MASK_NC
    jp      AM7J_END

    // -------- JA1(buttonA), JA2(0) 1つ読み取り
AM7J_L_DETECTED:
    // この時点で, JA2 の立ち下がりから最短 12(jr), 最悪 7(jp)+7(and)+12(jr) = 26 T state 遅れてます
    ld      A,  (HL)            // 7    ****_*0A*
    //ld A, 0x02 // TEST
    rrca                        // 4    ****_**0A
    rrca                        // 4    ****_***0   carry = buttonA
    rl      D                   // 8    0001_110A

    ld      A,  0 + MMIO_ETC_JA_MASK// 7   0000_0110
    //                          計 7+4+4+8+7 = 30 cycles

    // -------- JA1(buttonB), JA2(Y1) 1つ読み取り
    and     A,  (HL)            // 7    0000_0YB0   Y = Y1
    //ld A, 0x06 // TEST
    add     A,  D               // 4    00ww_wwBA   wwww = (Y1 == 0) ? 0111 : 1000
    and     A,  0x33            // 7    00yy_00BA   yy = Y1 + 1
    rrca                        // 4    A00y_y00B
    rrca                        // 4    BA00_yy00
    ld      D,  A               // 4    BA00_yy00
    //                          計 7+4+7+4+4+4 = 30 cycles

    // -------- JA1(R), JA2(Y0) 1つ読み取り
    ld      A,  (HL)            // 7    ****_*YR*    Y = Y0
    //ld A, 0x06 // TEST
    and     A,  0 + MMIO_ETC_JA_MASK// 7   0000_0YR0
    add     A,  D               // 4    BA00_yyR0    yy == Y1 + Y0 + 1 == UD
    rlca                        // 4    A00U_DR0B
    rlca                        // 4    00UD_R0BA
    ld      D,  A               // 4    00UD_R0BA
    //                          計 7+7+4+4+4+4 = 30 cycles

    // -------- JA1(L), JA2(1) 1つ読み取り
    ld      A,  (HL)            //      ****_*1L*
    //ld A, 0x06 // TEST
    rlca                        //      ****_1L**
    and     A,  E               //      0000_0L00
    or      A,  D               //      00UD_RLBA(負論理)

    // -------- Select/Start の場合
#if 0    // V1.01 左右同時押しでポーズがかかりやすいので廃止
    or      A,  0xc0            //      11UD_RLBA
    ld      E,  A
    and     A,  0x0c            //      0000_RL00
    ld      A,  E               //      11UD_RLBA
    jp      nz, JEND2           // 左右同時押し無し
                                //  Play       Select     Play + Select
                                //  1111_0011  1110_0011  1101_0011
    and     A,  0x30            //  0011_0000  0010_0000  0001_0000
    dec     A                   //  0010_1111  0001_1111  0000_1111
    add     A,  A               //  0101_1110  0011_1110  0001_1110
    add     A,  A               //  1011_1100  0111_1100  0011_1100
    or      A,  D               //  1011_11BA  0111_11BA  0011_11BA(負論理)
JEND2:
    cpl     A                   //      SPUD_RLBA(正論理)
#else
    xor     A,  0x3f            //      00UD_RLBA(正論理)
#endif
AM7J_END:
    ld      (_input_joy_), A
    BANKH_RAM C                 // バンク切替
    ret

#if 0
    // -------- 2-8) 正しく読み取れた

    // 検出済ならば, カウントアップして, 閾値越えたら非検出判定へ
AM7J_ND_STAT_DETECTED:
    ld      A, (_input_am7j_ct_)
    inc     A
    cmp     A, 0 + AM7J_CT
    ld      (_input_am7j_ct_), A
    ld      A, 0
    jp      nz, AM7J_END        // A = 0x00
    ld      (_input_am7j_ct_), A
    ld      (_input_am7j_detected_), A
    jp      AM7J_END            // A = 0x00



    // 既に検出状態ならば, 特に何もしない
    ld      D, A                // A 保存
      ld    A, (_input_am7j_detected_)
      or    A, A
    ld      A, D                // A 復帰
    jp      nz, AM7J_END
    // 未検出状態ならば, 検出カウンタ++して, 閾値越えたら検出状態へ
AM7J_D_STAT_NOT_DETECTED:
    ld      A, (_input_am7j_ct_)
    inc     A
    cmp     A, 0 + AM7J_CT
    ld      (_input_am7j_ct_), A
    ld      A, 0x00             // まだ AM7J の状態は 0x00 で
    jp      nz, AM7J_END
    dec     A                   // A = 0xff
    ld      (_input_am7j_detected_), A
    inc     A
    ld      (_input_am7j_ct_), A
AM7J_END:

    // ---------------- 3) キー入力と AM7J を合成, トリガ検出
    or      A, C
    ld      (_input_), A
    ld      E, A

    // input_trg_ = input_ & ~input_old_;
    ld      A, (_input_old_)
    cpl     A                   // ビット反転
    and     A, E
    ld      (_input_trg_), A
#endif
__endasm;
}


extern int _input_joy_;
extern int _input_;
extern int _input_old_;
extern int _input_trg_;

/** ジョイスティックとキーの入力をマージして, トリガ検出 */
static void inputMain2_(void) __aal
{
    AAL_DEF_VARS;

    // -------- if (input_joy != INPUT_MASK_NC) { input_ |= input_joy_; }
    A = mem[_input_joy_]; A &= (~INPUT_MASK_NC) & 0xff; B = A;

    A = mem[_input_]; A |= B; mem[_input_] = A; B = A;

    // -------- input_trg_ = input_ & ~input_old_;
    A = mem[_input_old_]; not(A); A &= B; mem[_input_trg_] = A;
}


void inputMain(void) __z88dk_fastcall
{
    //for (int i = 0; i < 30000; i++);
    inputKey_();
    if (input_joy_mode_ < INPUT_JOY_MODE_MZ1X03_DETECTING) {
        // -------- AM7J モード
        inputAM7J_();
        if (input_joy_mode_ < INPUT_JOY_MODE_AM7J_DETECTED) {
            // AM7J 検出中
            if (input_joy_ != INPUT_MASK_NC) {
                // 規定回数連続認識したら接続済みとする
                input_joy_mode_++;
            } else {
                // そうでなければ即 AM7J モードを抜ける
                input_joy_mode_ = INPUT_JOY_MODE_MZ1X03_DETECTING;
            }
        } else {
            // AM7J 検出済み
            if (input_joy_ == INPUT_MASK_NC) {
                // 規定回数連続認識できなかったら AM7J モードを抜ける
                input_joy_mode_++;
            } else {
                // そうでなければ AM7J モードのまま
                input_joy_mode_ = INPUT_JOY_MODE_AM7J_DETECTED;
            }
        }
    } else {
        // -------- MZ-1X03 モード
        if (input_joy_mode_ < INPUT_JOY_MODE_MZ1X03_DETECTED) {
            // MZ-1X03 検出中
            if (input_joy_ != INPUT_MASK_NC) {
                // 規定回数連続認識したら接続済みとする
                input_joy_mode_++;
            } else {
                // そうでなければ蘇武 MZ-1X03 モードを抜ける
                input_joy_mode_ = INPUT_JOY_MODE_AM7J_DETECTING;
            }
        } else {
            // MZ-1X03 モード
            if (input_joy_ == INPUT_MASK_NC) {
                // 規定回数連続認識できなかったら MZ-1X03 モードを抜ける
                input_joy_mode_++;
                if (INPUT_JOY_MODE_MAX < input_joy_mode_) {
                    input_joy_mode_ = INPUT_JOY_MODE_AM7J_DETECTING;
                }
            } else {
                // 異常検出してなければ MZ-1X03 モードのまま
                input_joy_mode_ = INPUT_JOY_MODE_MZ1X03_DETECTED;
            }
        }
    }

    inputMain2_();
}

// ---------------------------------------------------------------- 入力

// ---------------------------------------------------------------- AM7J

// ---------------------------------------------------------------- MZ-1X03
#pragma save
#pragma disable_warning 85          // 引数未使用警告抑止

void inputSetMZ1X03Enabled(bool const enabled) __aal __z88dk_fastcall
{
    AAL_DEF_VARS;                                    // L = enabled
    extern  inputMZ1X03_enabled;
    extern  _input_joy_mode_;
    extern  _input_joy_;

    // if (enabled) {
    //    mem[inputMZ1X03_enabled] == OPCODE_JP
    // } else {
    //    mem[inputMZ1X03_enabled] == OPCODE_JP_NZ
    // }
    L--;
    HL = inputMZ1X03_enabled;
    mem[HL] = OPCODE_JP_NZ;                             // 強制 MZ1X03 モード:無効
    if (z_jr) {
        mem[HL] = OPCODE_LD_BC_NN;                      // 強制 MZ1X03 モード:有効
    }

    // input_joy_mode_ = INPUT_JOY_MODE_MZ1X03_DETECTED;
    // input_joy_ = 0x00;
    A = INPUT_JOY_MODE_MZ1X03_DETECTED;
    mem[_input_joy_mode_] = A;

    A ^= A;
    mem[_input_joy_] = A;
}
#pragma restore


#pragma save
#pragma disable_warning 59          // 戻値未設定警告抑止

bool inputIsMZ1X03Enabled(void) __aal
{
    AAL_DEF_VARS;
    extern  inputMZ1X03_enabled;

    // return mem[inputMZ1X03_enabled] == OPCODE_JP
    A = mem[inputMZ1X03_enabled]; cp(A, OPCODE_JP_NZ);  // jp nc, xxxx ならば無効
    L = false; ret_z();                                 // 強制 MZ1X03 モード:無効
    L++;                                                // 強制 MZ1X03 モード:有効
}
#pragma restore


#pragma save
#pragma disable_warning 85          // 引数未使用警告抑止

void inputMZ1X03ButtonVSyncAxis1(u8 const mz1x03_sensitivity) __aal __z88dk_fastcall
{
    // MZ-1X03 の軸の読み方 (数字は T-states)
    //          __<-----------14316------------>______
    // /VBLK     |\____________________________/
    //          __       ______________________
    // JA1,JA2   |\_____//////////////////     \____
    //           |<--><--->  |<----->|
    //           0 100 209   a  b-a  b
    //
    // s 値(感度. 値が大きいほど感度が高い)と, ab の時間(/VBLK からの時間, T-states)
    // ※HSYNC 単位で動くので 912=228*4, 1824=228*8 と 228 T-states の倍数になってます
    // s  a                   b-a
    // --------------------------------------------
    // 0  100+209+912*0=309   1824*4=7296
    // 1  100+209+912*1=1221  1824*3=5472
    // 2  100+209+912*2=2133  1824*2=3648
    // 3  100+209+912*3=3045  1824*1=1824
    //
    // - (1) MZ-1X03 ボタンABのリード
    // - (2) /VBLK が H→L になるまで待つ
    // - (3) 100 T-states 待つ
    // - (4) ボタンABが 'L' でなければ非接続検出 (強制 MZ-1X03 モードでは無視)
    // - (5) (3)から, (209 + s * 912) T-states 待つ
    //       ※912 = 4 * 4 * 3 * 19 = 48 * (13+6) に分解できるので,
    //       ループ回数は 0, 48, 96, 144 に 11 を足した値. ループは djnz + inc HL
    // - (6) 軸を読み取り, a とする
    // - (7) (4 - s) 行の転送 (= 7296 - s * 1824 T-states) だけ待つ
    // - (8) 軸を読み取り, b とする
    // - (9) a, b の値から左右を判定する
    AAL_DEF_VARS;                                    // L = mz1x03_sensitivity(0～3)
    extern  inputMZ1X03_button_1;
    extern  inputMZ1X03_vSyncLoop;
    extern  inputMZ1X03_wait1Loop;
    extern  inputMZ1X03_wait2Loop;
    extern  inputMZ1X03_notDetected;
    extern  _input_joy_;

    BANKH_VRAM_MMIO(C);                                 // バンク切替

    E = L;                                              // E = mz1x03_sensitivity(0～3)
    // 現在 /VBLK = 'H' な筈なので, そのままボタンが読める筈!
#if DEBUG
    A = mem[MMIO_8255_PORTC]; A &= A;
    //ASSERT(m, ASSERT_CODE_VBLK);                        // Vブランク中ならエラー
#endif
    // -------- (1) MZ-1X03 ボタンABのリード
    D = MMIO_ETC_JA1_MASK | MMIO_ETC_JA2_MASK;
    HL = MMIO_ETC;
    A = mem[HL];                                        // ****_*BA*
    not(A);                                             // 正論理に変換
    A &= D;                                             // 0000_0BA0
    // /VBLK に突入してしまってボタンが読めなかったら assert(ちなみに両方おして)

    rrca(A);                                            // 0000_00BA
    mem[inputMZ1X03_button_1 + 1] = A;                  // 自己書換

    // -------- (2) /VBLK が H→L になるまで待つ
    A = H;                                              // 最上位 bit が '1' であればなんでもいい
    L = MMIO_8255_PORTC & 0xff;
    // 現在 /VBLK = 'H' な筈なので, チェックはしないでいい筈!
inputMZ1X03_vSyncLoop: {
        A &= mem[HL];                                   // /VBLK = L になるまで待つ
    } jp_m(inputMZ1X03_vSyncLoop);                      // 10   '1' ならループ
inputMZ1X03_vSync:

    // -------- (3) 100 T-states 待つ
    L = MMIO_ETC & 0xff;                                // 7
    B = 6;                                              // 7
inputMZ1X03_wait1Loop: {
    } djnz(B, inputMZ1X03_wait1Loop);                   // 13 * n - 5
    // 小計 10+7+7+(13*6-5) = 97

    // -------- (4) ボタンABが 'L' でなければ非接続検出 (強制 MZ-1X03 モードでは無視)
    A = mem[HL];                                        // 7    ****_*YX*
    A &= D;                                             // 4    0000_0YX0
inputMZ1X03_enabled:                                    // 自己書換 jp/jr
    jp_nz(inputMZ1X03_notDetected);                     // 10   接続されてるなら '0' が読める筈
    // 小計 7+4+10 = 21

    // -------- (5) (209 + s * 912) T-states 待つ
    A = E;                                              // 4    A = 0/1/2/3
    A += A; A += E;                                     // 8    A = 0/3/6/9
    A += A; A += A; A += A; A += A;                     // 16   A = 0/48/96/144
    A += 9;                                             // 7    本来は11. 実際は処理時間を考慮して少し引く(1引くと19 T-states)
    exx(); {                                            // 4
        B = A;                                          // 4
inputMZ1X03_wait2Loop: {
            HL++;
        } djnz(B, inputMZ1X03_wait2Loop);// 19 * n - 5
    } exx();                                            // 4
    // 小計:4+8+16+7+4+4+(19*(s*48-2))-5 = 912*s + 4

    // -------- (6) 軸読み取り a する
inputMZ1X03_readA:
    A = mem[HL];                                        // 7    ****_*YX*
    // 小計 7+4+4+4+7+4+(13*(64*0+6)-5)+7 = 110  理想 100
    // 小計 7+4+4+4+7+4+(13*(64*3+5)-5)+7 = 2606 理想 2602
    A &= D;                                             //      0000_0YX0
    rrca(A);                                            //      0000_00YX
    mem[_input_joy_] = A;

    BANKH_RAM(C);                                       // バンク切替
    // --------- (7296 - s * 1824) T-states だけ待つ
    return;

inputMZ1X03_notDetected:
    A = INPUT_MASK_NC; mem[_input_joy_] = A;
    BANKH_RAM(C);
}
#pragma restore


void inputMZ1X03Axis2(void) __aal __naked
{
    AAL_DEF_VARS;                                    // L = enabled
    extern  _input_joy_;
    extern  INPUT_MZ1X03_TAB;

    // --------- (8) 軸読み取り b する
    A = mem[_input_joy_];
    cp(A, INPUT_MASK_NC); ret_z();                      // 非検出なので処理なし
    E = A;                                              //      0000_00YX

    BANKH_VRAM_MMIO(C);                                 // バンク切替
inputMZ1X03_readB:
    A = mem[MMIO_ETC];                                  //      ****_*yx*

    // --------- (9) a, b の値から左右を判定する
    A &= MMIO_ETC_JA1_MASK | MMIO_ETC_JA2_MASK;         //      0000_0yx0
    A += A;                                             //      0000_yx00
    A |= E;                                             //      0000_yxYX
    HL = INPUT_MZ1X03_TAB;
    D = 0x00; E = A; HL += DE;
    A = mem[HL];                                        //      00UD_RL00
inputMZ1X03_button_1: A |= 0x00;                        //      00UD_RLBA 自己書換
    BANKH_RAM(C);                                       // バンク切替
    mem[_input_joy_] = A;
    return;

INPUT_MZ1X03_TAB:
    AAL_DB(INPUT_MASK_R | INPUT_MASK_D, INPUT_MASK_NC              , INPUT_MASK_NC              , INPUT_MASK_NC              );  // 0000, 0001, 0010, 0011
    AAL_DB(               INPUT_MASK_D, INPUT_MASK_L | INPUT_MASK_D, INPUT_MASK_NC              , INPUT_MASK_NC              );  // 0100, 0101, 0110, 0111
    AAL_DB(INPUT_MASK_R               , INPUT_MASK_NC              , INPUT_MASK_R | INPUT_MASK_U, INPUT_MASK_NC              );  // 1000, 1001, 1010, 1011
    AAL_DB(0                          , INPUT_MASK_L               , INPUT_MASK_U               , INPUT_MASK_L | INPUT_MASK_U);  // 1100, 1101, 1110, 1111
    AAL_NO_RETURN;
}
