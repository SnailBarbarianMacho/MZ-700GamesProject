/**
 * MZ-700 ハードウェア
 *
 * - 名前空間 MMIO_, IO_
 * - c 側でアドレスを使う場合は (u8*) キャストを付けてください.
 * - アセンブラで即値に使う場合, カッコから始まる式は, メモリアクセスになるので, #, +, - 等から始まる様にします
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef HARD_H_INCLUDED
#define HARD_H_INCLUDED
#define CPU_CLOCK                   3579545f

// ---------------------------------------------------------------- 色
#define BLACK   0
#define BLUE    1
#define RED     2
#define MAGENTA 3
#define GREEN   4
#define CYAN    5
#define YELLOW  6
#define WHITE   7

// ---------------------------------------------------------------- VRAM
#define VRAM_TEXT                   0xd000  /// テキストVRAM
#define VRAM_ATB                    0xd800  /// アトリビュート VRAM(text から atb への移動は, set 3, H でもOK)
#define VRAM_WIDTH                  40      /// VRAM の幅(文字)
#define VRAM_HEIGHT                 25      /// VRAM の高さ(文字)
#define VRAM_ATB_ATB_SHIFT          7
#define VRAM_ATB_FG_COL_SHIFT       4
#define VRAM_ATB_BG_COL_SHIFT       0
#define VRAM_TEXT_ADDR(x, y)        (VRAM_TEXT + VRAM_WIDTH * (y) + (x))   // VRAM TEXT のアドレス
#define VRAM_ATB_ADDR(x, y)         (VRAM_ATB  + VRAM_WIDTH * (y) + (x))   // VRAM ATB  のアドレス
/** atb を生成するマクロ(u8) */
#define VATB(fg_col, bg_col, b_atb) (       \
    ((fg_col) << VRAM_ATB_FG_COL_SHIFT) |   \
    ((bg_col) << VRAM_ATB_BG_COL_SHIFT) |   \
    ((b_atb) << VRAM_ATB_ATB_SHIFT))
/** text+atb を生成するマクロ(u16) */
#define VATB_CODE(fg_col, bg_col, b_atb, display_code) (    \
    ((fg_col) << (VRAM_ATB_FG_COL_SHIFT + 8)) |             \
    ((bg_col) << (VRAM_ATB_BG_COL_SHIFT + 8)) |             \
    ((b_atb)  << (VRAM_ATB_ATB_SHIFT    + 8)) |             \
    (display_code))


// ---------------------------------------------------------------- メモリマップド I/O
#define MMIO_8255_PORTA                 0xe000  /// Key strobe
#define MMIO_KEY_STROBE_SHIFT           0       /// Key Strobe
#define MMIO_KEY_STROBE_MASK            0x0f    /// Key Strobe
#define MMIO_556RST_SHIFT               7       /// カーソル点滅用タイマのリセット 556RST (0 = リセット)
#define MMIO_556RST_MASK                0x80    /// カーソル点滅用タイマのリセット 556RST (0 = リセット)
#define MMIO_8255_PORTB                 0xe001  /// Key read
#define MMIO_8255_PORTC                 0xe002  /// Sound mask, CMT Data, CMT Motor, V-blank, etc.
#define MMIO_8255_PORTC_VBLK_SHIFT      7       /// 垂直ブランキング信号 /VBLK (0 = ブランク中)
#define MMIO_8255_PORTC_VBLK_MASK       0x80    /// 垂直ブランキング信号 /VBLK (0 = ブランク中)
#define MMIO_8255_PORTC_556OUT_SHIFT    6       /// カーソル点滅用タイマ出力 556OUT
#define MMIO_8255_PORTC_556OUT_MASK     0x40    /// カーソル点滅用タイマ出力 556OUT
#define MMIO_8255_PORTC_RDATA_SHIFT     5       /// データ レコーダー入力 RDATA
#define MMIO_8255_PORTC_RDATA_MASK      0x20    /// データ レコーダー入力 RDATA
#define MMIO_8255_PORTC_MOTOR_SHIFT     4       /// データ レコーダーのモーターの状態 MOTOR (0 = OFF)
#define MMIO_8255_PORTC_MOTOR_MASK      0x10    /// データ レコーダーのモーターの状態 MOTOR (0 = OFF)
#define MMIO_8255_PORTC_MON_SHIFT       3       /// データ レコーダーのモーター制御 (0→1 で反転)
#define MMIO_8255_PORTC_MON_MASK        0x08    /// データ レコーダーのモーター制御 (0→1 で反転)
#define MMIO_8255_PORTC_INTMSK_SHIFT    2       /// 8253 CH2 からの割込マスク INTMSK (0 = マスク)
#define MMIO_8255_PORTC_INTMSK_MASK     0x04    /// 8253 CH2 からの割込マスク INTMSK (0 = マスク)
#define MMIO_8255_PORTC_WDATA_SHIFT     1       /// データ レコーダー出力 WDATA
#define MMIO_8255_PORTC_WDATA_MASK      0x02    /// データ レコーダー出力 WDATA
#define MMIO_8255_CTRL                  0xe003
#define MMIO_8255_CTRL_PORTC_BSR_SET(bit)   (((bit) << 1) | 1)      // BSR モードによるポートCのセット
#define MMIO_8255_CTRL_PORTC_BSR_RES(bit)   (((bit) << 1) | 0)      // BSR モードによるポートCのリセット

#define MMIO_8253_CT0                   0xe004  /// 8253 カウンタ 0. 出力は, サウンドに繋がってます
#define MMIO_8253_CT1                   0xe005  /// 8253 カウンタ 1. 出力は, カウンタ 2 に繋がってます
#define MMIO_8253_CT2                   0xe006  /// 8253 カウンタ 2. 出力は, Z80 の割込に繋がってます
#define MMIO_8253_CTRL                  0xe007  /// 8253 制御
#define MMIO_8253_CTRL_CT0_MASK         0x00    /// カウンタ 0 SC1～0
#define MMIO_8253_CTRL_CT1_MASK         0x40    /// カウンタ 1 SC1～0
#define MMIO_8253_CTRL_CT2_MASK         0x80    /// カウンタ 2 SC1～0
#define MMIO_8253_CTRL_RL_LATCH_MASK    0x00    /// カウンタR/W時のビット数. カウンタ ラッチ操作 RL1～0
#define MMIO_8253_CTRL_RL_L_MASK        0x10    /// カウンタR/W時のビット数. 下 8bit のみ        RL1～0
#define MMIO_8253_CTRL_RL_H_MASK        0x20    /// カウンタR/W時のビット数. 上 8bit のみ        RL1～0
#define MMIO_8253_CTRL_RL_LH_MASK       0x30    /// カウンタR/W時のビット数. 下→上 でアクセス   RL1～0
#define MMIO_8253_CTRL_MODE0_MASK       0x00    /// モード0 M2～0
#define MMIO_8253_CTRL_MODE1_MASK       0x02    /// モード1 M2～0
#define MMIO_8253_CTRL_MODE2_MASK       0x04    /// モード2 M2～0
#define MMIO_8253_CTRL_MODE3_MASK       0x06    /// モード3 M2～0
#define MMIO_8253_CTRL_MODE4_MASK       0x08    /// モード4 M2～0
#define MMIO_8253_CTRL_MODE5_MASK       0x0a    /// モード5 M2～0
#define MMIO_8253_CTRL_BCD_MASK         0x01    /// BCD カウンタモード. 通常 0 で使用 BCD
#define MMIO_8253_CTRL_CT0(rl_mask, mode_mask)   ((MMIO_8253_CTRL_CT0_MASK) | (rl_mask) | (mode_mask))
#define MMIO_8253_CTRL_CT1(rl_mask, mode_mask)   ((MMIO_8253_CTRL_CT1_MASK) | (rl_mask) | (mode_mask))
#define MMIO_8253_CTRL_CT2(rl_mask, mode_mask)   ((MMIO_8253_CTRL_CT2_MASK) | (rl_mask) | (mode_mask))
#define MMIO_8253_CT0_MODE0             MMIO_8253_CTRL_CT0(MMIO_8253_CTRL_RL_LH_MASK, MMIO_8253_CTRL_MODE0_MASK)
#define MMIO_8253_CT0_MODE3             MMIO_8253_CTRL_CT0(MMIO_8253_CTRL_RL_LH_MASK, MMIO_8253_CTRL_MODE3_MASK)    /// サウンドは, 通常このモードで使います. 0x36

#define MMIO_ETC                        0xe008  /// H-blank, Joystick, 8253 Ch 0 Gate, etc.
#define MMIO_ETC_HBLK_SHIFT             7       /// 水平ブランキング信号 /HBLK (0 = ブランク中)
#define MMIO_ETC_HBLK_MASK              0x80    /// 水平ブランキング信号 /HBLK (0 = ブランク中)
#define MMIO_ETC_JB_SHIFT               3       /// Joystick B 入力 JB2～1
#define MMIO_ETC_JB_MASK                0x18    /// Joystick B 入力 JB2～1
#define MMIO_ETC_JA1_MASK               0x02    /// Joystick A 入力 JA1
#define MMIO_ETC_JA2_MASK               0x04    /// Joystick A 入力 JA2
#define MMIO_ETC_JA_SHIFT               1       /// Joystick A 入力 JA2～1
#define MMIO_ETC_JA_MASK                0x06    /// Joystick A 入力 JA2～1
#define MMIO_ETC_GATE_SHIFT             0       /// 8253 Ch 0 Gate GATE (0 = サウンド停止)
#define MMIO_ETC_GATE_MASK              0x01    /// 8253 Ch 0 Gate GATE (0 = サウンド停止)

// ---------------------------------------------------------------- キー
#define KEY0_KANA_MASK          0x80
#define KEY0_GRAPH_MASK         0x40
#define KEY0_EQUAL_MASK         0x20    /// =
#define KEY0_EJ_MASK            0x10    /// 英数
#define KEY0_SEMICOLON_MASK     0x04
#define KEY0_COLON_MASK         0x02
#define KEY0_CR_MASK            0x01

#define KEY1_Y_MASK             0x80
#define KEY1_Z_MASK             0x40
#define KEY1_AT_MASK            0x20    /// @
#define KEY1_L_BRACKET_MASK     0x10    /// (
#define KEY1_R_BRACKET_MASK     0x08    /// )

#define KEY2_Q_MASK             0x80
#define KEY2_R_MASK             0x40
#define KEY2_S_MASK             0x20
#define KEY2_T_MASK             0x10
#define KEY2_U_MASK             0x08
#define KEY2_V_MASK             0x04
#define KEY2_W_MASK             0x02
#define KEY2_X_MASK             0x01

#define KEY3_I_MASK             0x80
#define KEY3_J_MASK             0x40
#define KEY3_K_MASK             0x20
#define KEY3_L_MASK             0x10
#define KEY3_M_MASK             0x08
#define KEY3_N_MASK             0x04
#define KEY3_O_MASK             0x02
#define KEY3_P_MASK             0x01

#define KEY4_A_MASK             0x80
#define KEY4_B_MASK             0x40
#define KEY4_C_MASK             0x20
#define KEY4_D_MASK             0x10
#define KEY4_E_MASK             0x08
#define KEY4_F_MASK             0x04
#define KEY4_G_MASK             0x02
#define KEY4_H_MASK             0x01

#define KEY5_1_MASK             0x80
#define KEY5_2_MASK             0x40
#define KEY5_3_MASK             0x20
#define KEY5_4_MASK             0x10
#define KEY5_5_MASK             0x08
#define KEY5_6_MASK             0x04
#define KEY5_7_MASK             0x02
#define KEY5_8_MASK             0x01

#define KEY6_ASTERISK_MASK      0x80    /// *
#define KEY6_PLUS_MASK          0x40    /// +
#define KEY6_MINUS_MASK         0x20    /// -
#define KEY6_SPACE_MASK         0x10
#define KEY6_0_MASK             0x08
#define KEY6_9_MASK             0x04
#define KEY6_COMMA_MASK         0x02    /// ,
#define KEY6_PERIOD_MASK        0x01    /// .

#define KEY7_INST_MASK          0x80
#define KEY7_DEL_MASK           0x40
#define KEY7_UP_MASK            0x20
#define KEY7_DOWN_MASK          0x10
#define KEY7_RIGHT_MASK         0x08
#define KEY7_LEFT_MASK          0x04
#define KEY7_QUESTION_MASK      0x02    /// ?
#define KEY7_EXCLAMATION_MASK   0x01    /// !

#define KEY8_BREAK_MASK         0x80
#define KEY8_CTRL_MASK          0x40
#define KEY8_SHIFT_MASK         0x01

#define KEY9_F1_MASK            0x80
#define KEY9_F2_MASK            0x40
#define KEY9_F3_MASK            0x20
#define KEY9_F4_MASK            0x10
#define KEY9_F5_MASK            0x08


// ---------------------------------------------------------------- I/O
#define IO_BANKL_RAM                    0xe0    /// 0x0000#0x1000 = RAM (書込値はなんでも OK)
#define IO_BANKH_RAM                    0xe1    /// 0xd000#0x3000 = RAM (書込値はなんでも OK)
#define IO_BANKL_ROM                    0xe2    /// 0x0000#0x1000 = MONITOR ROM (書込値はなんでも OK)
#define IO_BANKH_VRAM_MMIO              0xe3    /// 0xd000#0x1000 = VRAM, I/O (書込値はなんでも OK)
#define IO_BANKL_ROM_BANKH_VRAM_MMIO    0xe4    /// 0x0000#0x1000 = MONITOR ROM, 0xd000#0x1000 = VRAM, I/O (書込値はなんでも OK)

// ---------------------------------------------------------------- ハード アクセス インライン アセンブラ マクロ
static void hard_asm_macros_(void) __naked
{
__asm
    // -------------------------------- バンク切替

    /** メモリ上位を VRAM/MMIO に切り替えます
     * - メモリ下位は変化しません
     * - C レジスタを破壊するので, 引数で明示します
     */
    macro   BANKH_VRAM_MMIO C
    ld      C,  IO_BANKH_VRAM_MMIO
    out     (C), A              // 値はなんでもいい
    endm

    /** メモリ上位を RAM に切り替えます
     * - メモリ下位は変化しません
     * - C レジスタを破壊するので, 引数で明示します
     */
    macro   BANKH_RAM   C
    ld      C,  IO_BANKH_RAM
    out     (C), A              // 値はなんでもいい
    endm

    /** メモリ下位を RAM に切り替えます
     * - メモリ上位は変化しません
     * - C レジスタを破壊するので, 引数で明示します
     */
    macro   BANKL_RAM   C
    ld      C,  IO_BANKL_RAM
    out     (C), A              // 値はなんでもいい
    endm

    /** メモリ下位を ROM に切り替えます
     * - メモリ上位は変化しません
     * - C レジスタを破壊するので, 引数で明示します
     */
    macro   BANKL_ROM   C
    ld      C,  IO_BANKL_ROM
    out     (C), A              // 値はなんでもいい
    endm

    /** メモリ下位をモニタROM, 上位を VRAM/MMIO に切り替えます
     * - C レジスタを破壊するので, 引数で明示します
     */
    macro   BANKL_ROM_BANKH_VRAM_MMIO   C
    ld      C,  IO_BANKL_ROM_BANKH_VRAM_MMIO
    out     (C), A              // 値はなんでもいい
    endm

__endasm;
}


#endif // HARD_H_INCLUDED