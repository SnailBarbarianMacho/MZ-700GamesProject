/**
 * 数学関数
 *
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"

#ifndef MATH_H_INCLUDED
#define MATH_H_INCLUDED

// ---------------------------------------------------------------- システム
/** 数値テーブルの初期化などを行います */
void mathInit() __z88dk_fastcall;

// ---------------------------------------------------------------- 最大最小
inline u8 maxU8(u8 a, u8 b) { return a > b ? a : b; }   /// u8 版 max()
inline u8 minU8(u8 a, u8 b) { return a < b ? a : b; }   /// u8 版 min()
inline u8 clampU8(u8 a, u8 b, u8 c) { return minU8(maxU8(a, b), c); } /// a を [b, c] になるようにクランプします

inline s8 maxS8(s8 a, s8 b) { return a > b ? a : b; }   /// s8 版 max()
inline s8 minS8(s8 a, s8 b) { return a < b ? a : b; }   /// s8 版 min()
inline s8 clampS8(s8 a, s8 b, s8 c) { return minS8(maxS8(a, b), c); } /// a を [b, c] になるようにクランプします

inline u16 maxU16(u16 a, u16 b) { return a > b ? a : b; }   /// u16 版 max()
inline u16 minU16(u16 a, u16 b) { return a < b ? a : b; }   /// u16 版 min()
inline u16 clampU16(u16 a, u16 b, u16 c) { return minU16(maxU16(a, b), c); } /// a を [b, c] になるようにクランプします

inline s16 maxS16(s16 a, s16 b) { return a > b ? a : b; }   /// s16 版 max()
inline s16 minS16(s16 a, s16 b) { return a < b ? a : b; }   /// s16 版 min()
inline s16 clampS16(s16 a, s16 b, s16 c) { return minS16(maxS16(a, b), c); } /// a を [b, c] になるようにクランプします

// ---------------------------------------------------------------- 飽和演算
/**
 * u16 飽和加算
 */
u16 addSaturateU16(const u16 a, const u16 b) __naked;       ///
/**
 * u8  飽和加算
 * @param ab ((a << 8) | b).   common.h の X8Y8 マクロを使うと便利です
 */
u8 addSaturateU8(const u16 ab) __z88dk_fastcall __naked;

// ---------------------------------------------------------------- 三角関数
/**
 * x, y から向きを算出します
 * @param xy ((X << 8) | Y).   common.h の X8Y8 マクロを使うと便利です
 * @return 角度
 * -<pre>
 *         0xc0
 *           |
 *           |
 *  0x80 ----+---- 0x00
 *           |
 *           |
 *         0x40
 * </pre>
 */
u8 atan2(const u16 xy) __z88dk_fastcall;
/**
 * 向きから から y を算出します
 */
s8 sin(const u8 x) __z88dk_fastcall __naked;
/**
 * 向きから から x を算出します
 */
s8 cos(const u8 x) __z88dk_fastcall __naked;

// ---------------------------------------------------------------- 乱数
/**
 * 7 bit のリフレッシュ レジスタを返します
 * - 乱数としては非常に精度が低いです
 * - 破壊レジスタ: A, L
 */
u8 rand7r() __z88dk_fastcall __naked;
/**
 * 8 bit の乱数を返します
 * - M 系列疑似乱数で,周期は 131071 です.
 * - 初期値も設定できませんが, 1 フレームに 1 回呼ぶなど工夫すれば, れなりの乱数として使えます
 * - 破壊レジスタ: A, BC, DE, HL
 */
u8 rand8() __z88dk_fastcall __naked;

/** [-1, 1] の乱数を返します */
s8 rand8_sign() __z88dk_fastcall __naked;
/** [0, 7) の乱数を返します */
u8 rand8_7() __z88dk_fastcall __naked;
/** [0, 40) の乱数を返します */
u8 rand8_40() __z88dk_fastcall __naked;
/** [0, 25) の乱数を返します */
u8 rand8_25() __z88dk_fastcall __naked;

#endif //  MATH_H_INCLUDED
