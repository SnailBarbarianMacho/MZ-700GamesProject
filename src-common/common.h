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

// ---------------------------------------------------------------- 定数
#define true  1
#define false 0
#define nullptr ((void*)0x0000)

// ---------------------------------------------------------------- 便利マクロ
/** 配列の大きさを知ります */
#define COUNT_OF(array) (sizeof(array)/sizeof(array[0]))

/** 構造体 s のメンバ m のオフセット位置をバイトで返します. z88dkでは, 何故か定数式ではありません... */
#define OFFSET_OF(s, m) ((int)&(((s*)0)->m))

// ---------------------------------------------------------------- static assert
/** cond が 0(false) ならば, コンパイルを停止します
 * - 配列の大きさが負といって停止します
 * @example
 * STATIC_ASSERT(false, ConditionFailed);
 */
#ifdef DEBUG
#define STATIC_ASSERT(cond, msg) typedef char static_assertion_##msg[2*(!!(cond))-1]
#else
#define STATIC_ASSERT(cond, msg)
#endif

// ---------------------------------------------------------------- 引数を減らすための合同マクロ
// z88dk は, 引数を 1 個にして __fastcall を付けた関数の引数は, HL レジスタ経由で渡されるのです
#define X8Y8(x, y)          (((x) << 8) | (y))      /// 2つの s8 値を 1つの 16 bit 値にします. 不要ビットのカットはやってないので注意
#define W8H8(w, h)          (((w) << 8) | (h))      /// 2つの u8 値を 1つの 16 bit 値にします. 不要ビットのカットはやってないので注意

#endif // COMMON_H_INCLUDED
