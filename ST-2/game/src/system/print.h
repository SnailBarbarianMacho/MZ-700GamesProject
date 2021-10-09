/**
 * 文字の表示
 * - 名前空間 PRINT_ CHAR_ ATB または print
 * - 文字, 数値の表示
 * - 画像の表示
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef PRINT_H_INCLUDED
#define PRINT_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../../../../src-common/char.h"

// ---------------------------------------------------------------- マクロ

// ---------------------------------------------------------------- 設定
/**
 * 属性を設定します
 * @param atb 属性. VATB() マクロが便利です
 *   7   6    5   4   3   2   1   0
 * +---+---+---+---+---+---+---+---+
 * |ATB|  Fg Color |   | Bg Color  |
 * +---+---+---+---+---+---+---+---+
 */
void printSetAtb(const u8 atb) __z88dk_fastcall;
/**
 * 表示先アドレスを設定します
 * @param addr アドレス
 * @example
 * printSetAddr((u8*)VVRAM_TEXT_ADDR(x, y));
 */
void printSetAddr(u8* const addr) __z88dk_fastcall;
/**
 * 表示先アドレスを相対移動します
 * 1文字進めたりするのに便利です
 */
void printAddAddr(u16 offset) __z88dk_fastcall;

// ---------------------------------------------------------------- 表示(文字)
/** 1文字出力します. 制御コードは効きません */
void printPutc(const u8 c) __z88dk_fastcall __naked;

// ---------------------------------------------------------------- 表示(文字列)
/**
 * printString() と同じですが, 最大文字列の長さを指定できます.
 * - 制御コード(CHAR_CAPS 等)を除きます
 * @param len 制御コードを除いた文字数. 負の値も指定できます. 0 未満の場合, 表示されません
 */
void printStringWithLength(const u8* const str, const s16 len) __naked;
/**
 * 文字列を表示します
 * - 0 を検出すると, そこまで表示して終了します
 * - CHAR_CAPS で小文字やひらがなに変更しても, 終了したら戻ります
 */
inline void printString(const u8* const str) {
    printStringWithLength(str, 0x7fff);
}

// ---------------------------------------------------------------- 表示(8 bit 数値)
/**
 * 8 bit 値を 3 桁右詰めで表示します
 */
void printU8Right(const u8 value) __z88dk_fastcall __naked;
/**
 * 8 bit 値を左詰めで表示します
 */
void printU8Left(const u8 value) __z88dk_fastcall __naked;

// ---------------------------------------------------------------- 表示(16 bit 数値)
/**
 * 16 bit 値を 5 桁右詰めで表示します
 */
void printU16Right(const u16 value) __z88dk_fastcall __naked;
/**
 * 16 bit 値を左詰めで表示します
 */
void printU16Left(const u16 value) __z88dk_fastcall __naked;

#if DEBUG
/**
 * 16 bit 値を 16 進 4 桁表示します. デバッグ用
 */
void printHex16(const u16 value) __z88dk_fastcall __naked;
#endif

// ---------------------------------------------------------------- 表示ユーティリティ
/** "READY!" を表示します. アトラクトモード時は何もしません */
void printReady();

#endif // PRINT_H_INCLUDED
