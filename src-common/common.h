/**
 * 共通ヘッダ
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

// ---------------------------------------------------------------- 型
typedef unsigned char  u8;
typedef signed   char  s8;
typedef unsigned short u16;
typedef signed   short s16;
typedef unsigned long  u32;
typedef signed   long  s32;
typedef unsigned char  bool;
typedef unsigned short uintptr_t;   // ポインタの大きさの整数(z88dkには無さそうだったので)

// アセンブラ用
#define SIZEOF_U8 1
#define SIZEOF_S8 1
#define SIZEOF_U16 2
#define SIZEOF_S16 2
#define SIZEOF_U32 4
#define SIZEOF_S32 4
#define SIZEOF_BOOL 1
#define SIZEOF_POINTER 2

// ---------------------------------------------------------------- 定数
#define true  1
#define false 0
#define nullptr ((void *)0x0000)

// ---------------------------------------------------------------- 便利マクロ
/** 配列の大きさを知ります */
#define COUNT_OF(array) (sizeof(array)/sizeof(array[0]))

/** 構造体 s のメンバ m のオフセット位置をバイトで返します. z88dkでは, 何故か定数式ではありません... */
#define OFFSET_OF(s, m) ((int)&(((s*)0)->m))

/** u8 の配列に 16bit 値を書き込むマクロ */
#define U8ARR16(val) (val & 0xff), ((val >> 8) & 0xff)

/** パラメータ抽出マクロ. アセンブラ版が asm_macros.h (PEXTA) にあります */
#define PEXT(val, mask, shift) (((val) & mask) >> shift)

// ---------------------------------------------------------------- static assert
/** cond が 0(false) ならば, コンパイルを停止します
 * @example
 * STATIC_ASSERT(false, "condition failed");
 */
#ifdef DEBUG
// 2023.05頃, typedef char foo[-1] がエラーにならなくなった代わりに, _Static_assert が実装されていた(warning 215 になる)
// typedef char static_assertion_##msg[(cond) ? 1 : 0]
#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
#define STATIC_ASSERT(cond, msg)
#endif

// ---------------------------------------------------------------- 引数を減らすための合同マクロ
// z88dk は, 引数を1つにして __z88dk_fastcall で呼び出すと,
// 最大 DE:HL レジスタ経由で渡されるのです
// 注: 上位ビットのカットはしてないので, 各自カットしてください */

/** s8 x 2 を, u16 にパック.    注: y の上位ビットのカットはしてません */
#define X8Y8(x, y)              (((u16)(x) << 8) | (y))
/** u8 x 2 を, u16 にパック.    注: y の上位ビットのカットはしてません */
#define W8H8(w, h)              (((u16)(w) << 8) | (h))
/** u8 x 2 を, u16 にパック.    注: y の上位ビットのカットはしてません */
#define H8W8(h, w)              (((u16)(h) << 8) | (w))
/** u8 + u16 を, u32 にパック.  注: x, y の上位ビットのカットはしてません */
#define X8Y16(x, y)             (((u32)(x) << 16) | (y))
/** u16 + u8 を, u32 にパック.  注: y の上位ビットのカットはしてません */
#define X16Y8(x, y)             (((u32)(x) << 16) | (y))
/** u16 x 2 を, u32 にパック.   注: y の上位ビットのカットはしてません */
#define X16Y16(x, y)            (((u32)(x) << 16) | (y))
/** u16 x 2 を, u32 にパック.   注: x の上位ビットのカットはしてません */
//#define Y16X16(y, x)            (((u32)(y) << 16) | (x))
/** u8 x 3 を, u32 にパック.    注: x, y, z の上位ビットのカットはしてません */
#define X8Y8Z8(x, y, z)         (((u32)(x) << 16) | ((y)<<8) | (z))
/** u8 x 4 を, u32 にパック.    注: y, z, w の上位ビットのカットはしてません */
#define X8Y8Z8W8(x, y, z, w)    (((u32)(x) << 24) | ((y)<<16) | ((z)<<8) | (w))
/** u8 x 2 + void * を, u32 にパック. 注: y, p の上位ビットのカットはしてません */
#define X8Y8P16(x, y, p)        (((u32)(x) << 24) | ((y)<<16) | (p))
/** u8 x 3 を u32 にパック.     注: z, y, x  の上位ビットのカットはしてません */
//#define Z8Y8X8(z, y, x)     (((u32)(z) << 16) | ((y)<<8) | x)


#endif // COMMON_H_INCLUDED
