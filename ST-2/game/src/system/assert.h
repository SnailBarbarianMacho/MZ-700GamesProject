/**
 * assert
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef ASSERT_H_INCLUDED
#define ASSERT_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- assert
#if DEBUG
/** ASSERT マクロ. cond を満たさない場合は, 値を表示して停止します(ソース名や行番号は表示しません) */
#define ASSERT(cond, val) do { if (!(cond)) { assert(val); } } while (false)
#else
#define ASSERT(cond, val)
#endif

#if DEBUG
/** assert 本体
 * - アセンブラから直接呼ぶ場合は, HL にコードを入れて, jp _assert などとします(スタック不要)
 */
void assert(u16 val) __z88dk_fastcall __naked;
#endif


#endif // ASSERT_H_INCLUDED