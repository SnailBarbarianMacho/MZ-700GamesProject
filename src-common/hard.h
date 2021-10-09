/**
 * ハードウェア
 *
 * - 名前空間 MIO_, IO_, BANK_
 * - c 側でアドレスを使う場合は (u8*) キャストを付けてください.
 * - アセンブラで即値に使う場合は #値 または #(値), メモリ アクセスする場合は (値) 又は (#値) と書いてください
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef HARD_H_INCLUDED
#define HARD_H_INCLUDED
// ---------------------------------------------------------------- VRAM
#define VRAM_TEXT                   0xd000  /// テキストVRAM
#define VRAM_ATB                    0xd800  /// アトリビュート VRAM
// VRAM_WIDTH, VRAM_HEIGHT, VATB() VATB_CODE() マクロは vram.h にあります

// ---------------------------------------------------------------- メモリマップド I/O
#define MIO_8255_PORTA              0xe000  /// Key strobe
#define MIO_KEY_STROBE_SHIFT        0       /// Key Strobe
#define MIO_KEY_STROBE_MASK         0x0f    /// Key Strobe
#define MIO_556RST_SHIFT            7       /// カーソル点滅用タイマのリセット 556RST (0 = リセット)
#define MIO_556RST_MASK             0x80    /// カーソル点滅用タイマのリセット 556RST (0 = リセット)
#define MIO_8255_PORTB              0xe001  /// Key read
#define MIO_8255_PORTC              0xe002  /// Sound mask, CMT Data, CMT Motor, V-blank, etc.
#define MIO_8255_PORTC_VBLK_SHIFT   7       /// 垂直ブランキング信号 VBLK (0 = ブランク中)
#define MIO_8255_PORTC_VBLK_MASK    0x80    /// 垂直ブランキング信号 VBLK (0 = ブランク中)
#define MIO_8255_PORTC_556OUT_SHIFT 6       /// カーソル点滅用タイマ出力 556OUT
#define MIO_8255_PORTC_556OUT_MASK  0x40    /// カーソル点滅用タイマ出力 556OUT
#define MIO_8255_PORTC_RDATA_SHIFT  5       /// データ レコーダー入力 RDATA
#define MIO_8255_PORTC_RDATA_MASK   0x20    /// データ レコーダー入力 RDATA
#define MIO_8255_PORTC_MOTOR_SHIFT  4       /// データ レコーダーのモーターの状態 MOTOR (0 = OFF)
#define MIO_8255_PORTC_MOTOR_MASK   0x10    /// データ レコーダーのモーターの状態 MOTOR (0 = OFF)
#define MIO_8255_PORTC_MON_SHIFT    3       /// データ レコーダーのモーター制御 (0→1 で反転)
#define MIO_8255_PORTC_MON_MASK     0x08    /// データ レコーダーのモーター制御 (0→1 で反転)
#define MIO_8255_PORTC_INTMSK_SHIFT 2       /// 8253 CH2 からの割込マスク INTMSK (0 = マスク)
#define MIO_8255_PORTC_INTMSK_MASK  0x04    /// 8253 CH2 からの割込マスク INTMSK (0 = マスク)
#define MIO_8255_PORTC_WDATA_SHIFT  1       /// データ レコーダー出力 WDATA
#define MIO_8255_PORTC_WDATA_MASK   0x02    /// データ レコーダー出力 WDATA
#define MIO_8255_CTRL               0xe003

#define MIO_8253_CH0                0xe004  /// モード 3 or 0 で使用. サウンドで使用します
#define MIO_8253_CH1                0xe005  /// モード 4 で使用. デバッグ時の処理時間計測に使用
#define MIO_8253_CH2                0xe006  /// 未使用. 出力は, Z80 の割込に繋がってます
#define MIO_8253_CTRL               0xe007
#define MIO_8253_CTRL_CH0_MASK      0x00    /// カウンタ CH0 選択 SC1～0
#define MIO_8253_CTRL_CH1_MASK      0x40    /// カウンタ CH1 選択 SC1～0
#define MIO_8253_CTRL_CH2_MASK      0x80    /// カウンタ CH2 選択 SC1～0
#define MIO_8253_CTRL_RL_MASK       0x30    /// 読み書き. 通常 3 で使用 RL1～0
#define MIO_8253_CTRL_M0_MASK       0x00    /// モード0 M2～0
#define MIO_8253_CTRL_M1_MASK       0x02    /// モード1 M2～0
#define MIO_8253_CTRL_M2_MASK       0x04    /// モード2 M2～0
#define MIO_8253_CTRL_M3_MASK       0x06    /// モード3 M2～0
#define MIO_8253_CTRL_M4_MASK       0x08    /// モード4 M2～0
#define MIO_8253_CTRL_M5_MASK       0x0a    /// モード5 M2～0
#define MIO_8253_CTRL_BCD_MASK      0x01    /// BCD カウンタモード. 通常 0 で使用 BCD
#define MIO_8253_CH0_MODE0          MIO_8253_CTRL_CH0_MASK | MIO_8253_CTRL_RL_MASK | MIO_8253_CTRL_M0_MASK  /// サウンドをモード3 に設定する. アセンブラの都合でカッコを入れてないので注意. 0x30
#define MIO_8253_CH0_MODE3          MIO_8253_CTRL_CH0_MASK | MIO_8253_CTRL_RL_MASK | MIO_8253_CTRL_M3_MASK  /// サウンドをモード3 に設定する. アセンブラの都合でカッコを入れてないので注意. 0x36

#define MIO_ETC                     0xe008  /// H-blank, Joystick, 8253 Ch 0 Gate, etc.
#define MIO_ETC_HBLK_SHIFT          7       /// 水平ブランキング信号 HBLK (0 = ブランク中)
#define MIO_ETC_HBLK_MASK           0x80    /// 水平ブランキング信号 HBLK (0 = ブランク中)
#define MIO_ETC_JB_SHIFT            3       /// Joystick B 入力 JB2～1
#define MIO_ETC_JB_MASK             0x18    /// Joystick B 入力 JB2～1
#define MIO_ETC_JA1_MASK            0x02    /// Joystick A 入力 JA1
#define MIO_ETC_JA2_MASK            0x04    /// Joystick A 入力 JA2
#define MIO_ETC_JA_SHIFT            1       /// Joystick A 入力 JA2～1
#define MIO_ETC_JA_MASK             0x06    /// Joystick A 入力 JA2～1
#define MIO_ETC_GATE_SHIFT          0       /// 8253 Ch 0 Gate GATE (0 = サウンド停止)
#define MIO_ETC_GATE_MASK           0x01    /// 8253 Ch 0 Gate GATE (0 = サウンド停止)

// ---------------------------------------------------------------- I/O
#define IO_BANKL_RAM                0xe0    /// 0x0000#0x1000 = RAM (書込値はなんでも OK)
#define IO_BANKH_RAM                0xe1    /// 0xd000#0x3000 = RAM (書込値はなんでも OK)
#define IO_BANKL_ROM                0xe2    /// 0x0000#0x1000 = MONITOR ROM (書込値はなんでも OK)
#define IO_BANKH_VRAM_IO            0xe3    /// 0xd000#0x1000 = VRAM, I/O (書込値はなんでも OK)
#define IO_BANKL_ROM_BANKH_VRAM_IO  0xe4    /// 0x0000#0x1000 = MONITOR ROM, 0xd000#0x1000 = VRAM, I/O (書込値はなんでも OK)

// ---------------------------------------------------------------- マクロ
/// バンクを切り替えて, VRAM にアクセスするようにします. C レジスタ破壊
#define BANK_VRAM_IO            \
    ld      C, #IO_BANKH_VRAM_IO\
    out     (C), A              // 値はなんでもいい

/// バンクを切り替えて, VRAM を切り離します. C レジスタ破壊
#define BANK_RAM                \
    ld      C, #IO_BANKH_RAM    \
    out     (C), A              // 値はなんでもいい

#endif // HARD_H_INCLUDED