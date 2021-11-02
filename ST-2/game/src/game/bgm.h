/**
 * BGM
 * - 名前空間 BGM_ および bgm
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef BGM_H_INCLUDED
#define BGM_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- マクロ, 構造体
// sdPlayBgm() の引数
#define BGM_NONE    0   // 音無し
#define BGM_1       1
#define BGM_2       2
#define BGM_3       3
#define BGM_4       4
#define NR_BGMS     5

// BGM シーケンサ記述構造体
typedef struct s_BgmSequencerDesc {
    void (*init)();  // 初期化関数 nullptr 可
    u16 (*main)(u16);// sdMake() で音を出す関数. 引数は唯一の変数として使用し, 戻値で保存. 初期値 0. nullptr 可
} BgmSequencerDesc;

// ---------------------------------------------------------------- private 変数. 直接触らない
extern const BgmSequencerDesc _bgmSequencerTab[NR_BGMS];

// ---------------------------------------------------------------- BGM シーケンサ
inline const BgmSequencerDesc* bgmGetSequencerDesc(const u8 bgm) { return &_bgmSequencerTab[bgm]; }

#endif // BGM_H_INCLUDED