/**
 * 仮想 VRAM 管理
 * - 名前空間 VRAM_, vram, VVRAM_, vVram
 *   仮想 VRAM はそのまま VRAM で, 実 VRAM は RVRAM です
 * - VRAM の管理
 * - VRAM から RVRAM への転送
 * - VRAM のクリア
 * - RVRAM のクリア
 *
 * - 仮想 VRAM は次のようになってます:
 *                                        VVRAM_WIDTH = 0x100 固定
 *         <----------------------------------------------------------------------------------------->
 *     VVRAM_GAPX  VRAM_WIDTH  VVRAM_GAPX
 *           |        |         |
 *         <--><------ ------><--><------ ------>
 * 0xfc00 +----------- ------------------ ------------------ ------------------ ---------------- ----+ ^
 * 0xfd00 |                                                                                          | |
 * 0xfe00 |                                                                                          | |  4 VVRAM_GAPY
 * 0xff00 |                                                                                          | v
 * 0x0000 |    +------ ------+    +------ ------+                                                    | ^
 * 0x0100 |    |             |    |             |                                                    | |
 *        :    :    TEXT     :    :     ATB     :                                                    : : 25 (0x19)VRAM_HEIGHT
 * 0x1700 |    |             |    |             |                                                    | |
 * 0x1800 |    +------ ------+    +------ ------+                                                    | v
 * 0x1900 |                                                                                          | ^
 * 0x1a00 |                                                                                          | |  4 VVRAM_GAPY
 * 0x1b00 |                                           VVRAM_TMP_WORK                                 | |
 * 0x1c00 +----------- ------------------ ------------------ ------------------ ---------------- ----+ v
 * 0x1d00
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef VRAM_H_INCLUDED
#define VRAM_H_INCLUDED

#include "../../../../src-common/common.h"
#include "addr.h"

// ---------------------------------------------------------------- 変数
extern u8 b_vram_trans_enabled_;
#if DEBUG
extern u8  vram_8253_ct2_;
extern u16 vram_trans_v_counter_;
#endif

// ---------------------------------------------------------------- マクロ(実 VRAM)
#define VRAM_WIDTH              40      /// VRAM の幅(文字)
#define VRAM_HEIGHT             25      /// VRAM の高さ(文字)
#define VRAM_TEXT_ADDR(x, y)    (VRAM_TEXT + VRAM_WIDTH * (y) + (x))   // 実 VRAM TEXT のアドレス. 要 hard.h
#define VRAM_ATB_ADDR(x, y)     (VRAM_ATB  + VRAM_WIDTH * (y) + (x))   // 実 VRAM ATB  のアドレス. 要 hard.h

// ---------------------------------------------------------------- マクロ(仮想 VRAM)
#define VVRAM_WIDTH             0x100       // 仮想 VRAM の幅
#define VVRAM_GAPY              4           // 仮想 VRAM の隙間
#define VVRAM_GAPX              6           // 仮想 VRAM の隙間
#define VVRAM_TEXT_ADDR(x, y)   (ADDR_VVRAM + VVRAM_GAPX + (x) + (VVRAM_WIDTH * (VVRAM_GAPY + (y))))// 仮想 VRAM TEXT のアドレス
#define VVRAM_ATB_ADDR(x, y)    (VVRAM_TEXT_ADDR((x), (y)) + VRAM_WIDTH + VVRAM_GAPX)               // 仮想 VRAM ATB  のアドレス

#define VVRAM_TMP_WORK          VVRAM_ATB_ADDR(VRAM_WIDTH + VVRAM_GAPX, VRAM_HEIGHT + VVRAM_GAPY - 1) // スタックが使えない時の一時的ワーク. 仮想画面外に設置

// ---------------------------------------------------------------- システム
/** VRAM システムの初期化 */
void vramInit(void) __z88dk_fastcall __naked;

/**
 * 仮想 VRAM から VRAM に転送します
 * そのあと, 仮想 VRAM をクリアします
 * この API は, 約 1/30 ～ 1/60 sec かかります
 */
void vramTrans(void) __z88dk_fastcall __naked;

/** VRAM の転送を停止します. 次のフレームでは許可されます */
inline void vramSetTransDisabled(void) { b_vram_trans_enabled_ = false; }

// ---------------------------------------------------------------- デバッグ
#if DEBUG
/** 前回の呼び出しからの処理時間 ms を返します.
 * - 30 FPS での場合, 33msec を越えると処理落ちです
 * - 20 FPS での場合, 50msec を越えると処理落ちです
 */
u16 vramDebugGetProcessTime(void);

/** 8253 ch2 の値を返します. デバッグ用. */
inline u8  vramGet8253Ct2(void)      { return vram_8253_ct2_;        }
inline u16 vramTransGetCounter(void) { return vram_trans_v_counter_; }
#endif

// ---------------------------------------------------------------- クリア
/** 仮想 VRAM をクリアします */
void vvramClear(void) __z88dk_fastcall __naked;

// ---------------------------------------------------------------- 塗りつぶし(fill)
/**
 * 仮想 VRAM の矩形領域を塗りつぶします
 * @param draw_addr 画面左上のアドレス.  VVRAM_TEXT_ADDR() マクロを使うと便利です
 * @param wh ((W << 8) | H).   common.h の W8H8 マクロを使うと便利です
 * @param code ((ATB << 8) | ディスプレイコード). VATB_CODE() マクロを使うと便利です
 */
void vVramFillRect(const u8* const draw_addr, const u16 wh, const u16 code) __naked;

/**
 * 実 VRAM の全画面を塗りつぶします. ウエイトがかかって遅いので注意です
 * @param code (ATB << 8) | ディスプレイコード
 */
void vramFill(const u16 code) __z88dk_fastcall __naked;

// ---------------------------------------------------------------- 描画(draw)m x n
/**
 * 仮想 VRAM の矩形領域を描画します
 * @param draw_addr 画面左上のアドレス.  VVRAM_TEXT_ADDR() マクロを使うと便利です
 * @param src_addr ソース アドレス. TEXT が (w * h) 個分あって, 続いて ATB が (w * h) 個分あります
 * @param wh ((W << 8) | H).   common.h の W8H8 マクロを使うと便利です
 */
void vVramDrawRect(const u8* const draw_addr, const u8* const src_addr, const u16 wh) __naked;

/**
 * 仮想 VRAM の矩形領域を描画します. TEXT の値が 0x00 ならば描画しないでスキップします.
 * @param draw_addr 画面左上のアドレス.  VVRAM_TEXT_ADDR() マクロを使うと便利です
 * @param src_addr ソース アドレス.「TEXT と ATB」 が (w * h) 個分あります. 但し TEXT = 0x00 ならば ATB はありません
 * @param wh ((W << 8) | H).   common.h の W8H8 マクロを使うと便利です
 */
void vVramDrawRectTransparent(const u8* const draw_addr, const u8* const src_addr, const u16 wh) __naked;

/**
 * 実 VRAM の矩形領域を描画します. ウエイトがかかって遅いので注意です
 * @param draw_addr 画面左上のアドレス. VRAM_TEXT_ADDR() マクロを使うと便利です
 * @param src_addr ソース アドレス. TEXT が (w * h) 個分あって, 続いて ATB が (w * h) 個分あります
 * @param wh ((W << 8) | H).   common.h の W8H8 マクロを使うと便利です
 */
void vramDrawRect(const u8* const draw_addr, const u8* const src_addr, const u16 wh) __naked;


// ---------------------------------------------------------------- 描画(draw)1 x n
/**
 * 仮想 VRAM の 1x1 領域を描画します
 * @param draw_addr 画面左上のアドレス.  VVRAM_TEXT_ADDR() マクロを使うと便利です
 * @param code ((ATB << 8) | ディスプレイコード). VATB_CODE() マクロを使うと便利です
 */
void vVramDraw1x1(const u8* const draw_addr, const u16 code) __naked;
/**
 * 仮想 VRAM の 1x3 領域を描画します
 * @param draw_addr 画面左上のアドレス.  VVRAM_TEXT_ADDR() マクロを使うと便利です
 * @param stc_addr ソース アドレス. TEXT が (1 * 3) 個分あって, 続いて ATB が (1 * 3) 個分あります
 */
void vVramDraw1x3(const u8* const draw_addr, const u8* const stc_addr) __naked;

// ---------------------------------------------------------------- 描画(draw)3 x 3
/**
 * 仮想 VRAM の 3x3 領域を描画します
 * @param draw_addr 画面左上のアドレス.  VVRAM_TEXT_ADDR() マクロを使うと便利です
 * @param stc_addr ソース アドレス. TEXT が (3 * 3) 個分あって, 続いて ATB が (3 * 3) 個分あります
 */
void vVramDraw3x3(const u8* const draw_addr, const u8* const stc_addr) __naked;
/**
 * 仮想 VRAM の 3x3 領域を描画します. TEXT の値が 0x00 ならば描画しないでスキップします.
 * @param draw_addr 画面左上のアドレス.  VVRAM_TEXT_ADDR() マクロを使うと便利です
 * @param stc_addr ソース アドレス.「TEXT と ATB」 が (w * h) 個分あります. 但し TEXT = 0x00 ならば ATB はありません
 */
void vVramDraw3x3Transparent(const u8* const draw_addr, const u8* const stc_addr) __naked;

// ---------------------------------------------------------------- 描画(draw)4 x 4, 5 x 5
/**
 * 仮想 VRAM の 4x4 領域を描画します
 * @param draw_addr 画面左上のアドレス.  VVRAM_TEXT_ADDR() マクロを使うと便利です
 * @param stc_addr ソース アドレス. TEXT が (4 * 4) 個分あって, 続いて ATB が (4 * 4) 個分あります
 */
void vVramDraw4x4(const u8* const draw_addr, const u8* const stc_addr) __naked;

/**
 * 仮想 VRAM の 5x5 領域を描画します
 * @param draw_addr 画面左上のアドレス.  VVRAM_TEXT_ADDR() マクロを使うと便利です
 * @param stc_addr ソース アドレス. TEXT が (4 * 4) 個分あって, 続いて ATB が (4 * 4) 個分あります
 */
void vVramDraw5x5(const u8* const draw_addr, const u8* const stc_addr) __naked;

#if 0 //今回は使ってない
/**
 * 仮想 VRAM の 5x5 領域を描画します. TEXT の値が 0x00 ならば描画しないでスキップします.
 * @param draw_addr 画面左上のアドレス.  VVRAM_TEXT_ADDR() マクロを使うと便利です
 * @param stc_addr ソース アドレス.「TEXT と ATB」 が (w * h) 個分あります. 但し TEXT = 0x00 ならば ATB はありません
 */
void vVramDraw5x5Transparent(const u8* const draw_addr, const u8* const stc_addr) __naked;
#endif

#endif
