/**
 * SE
  * - 名前空間 SE_ および se
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef SE_H_INCLUDED
#define SE_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- マクロ, 構造体
// sdPlaySe() の引数
#define SE_START            0
#define SE_SUB_STAGE_START  1
#define SE_LEVEL_UP         2
#define SE_1UP              3
#define SE_ENEMY_DEAD3      4
#define SE_ENEMY_DEAD45     5
#define SE_ENEMY_DEAD8      6
#define SE_ENEMY_DAMAGE     7
#define SE_GET_ITEM         8
#define SE_PLAYER_DEAD      9
#define SE_CONTINUE         10
#define SE_END              11
#define SE_END_GAME_OVER    12
#define SE_CHIME            13
#define NR_SES              14

// SE シーケンサ記述構造体
typedef struct s_SeSequencerDesc {
    void (*main_func)(u8); // sdMake() で音を出す関数. 引数は自動decrementされるカウンタ. カウンタ=0 か, 優先順位高い SE が鳴ると終了. nullptr 可
    u8       priority;      // 優先順位
    const u8 ct;            // カウンタ初期値
} SeSequencerDesc;

// ---------------------------------------------------------------- private 変数. 直接触らない
extern const SeSequencerDesc SE_SEQUENCE_TAB_[NR_SES];

// ---------------------------------------------------------------- SE シーケンサ
inline const SeSequencerDesc* seGetSequencerDesc(const u8 se) { return &SE_SEQUENCE_TAB_[se]; }

#endif // SE_H_INCLUDED