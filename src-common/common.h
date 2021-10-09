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

///** 構造体 type のメンバ member のオフセット位置をバイトで返します. コンパイル字に内部エラーが出るので現在ボツ */
//#define OFFSET_OF(type, member)  ((u8)&(((type*)nullptr)->member))

/** cond が 0(false) ならばここでコンパイルを停止します
 * - 配列の大きさが負といって停止します
 * - 手抜き版なので制限があります:
 *   -  msg は[A-Za-z0-9_] のみ. スペースは使えません.
 *   - スコープが届く範囲で同じメッセージは使えません
 * @example
 * STATIC_ASSERT(false, ConditionFailed);
 */
#define STATIC_ASSERT(cond, msg) typedef char static_assertion_##msg[2*(!!(cond))-1]

// ---------------------------------------------------------------- 引数を減らすための合同マクロ
#define X8Y8(x, y)          (((x) << 8) | (y))      /// 2つの s8 値を 1つの 16 bit 値にします. 不要ビットのカットはやってないので注意
#define W8H8(w, h)          (((w) << 8) | (h))      /// 2つの u8 値を 1つの 16 bit 値にします. 不要ビットのカットはやってないので注意

#endif // COMMON_H_INCLUDED
