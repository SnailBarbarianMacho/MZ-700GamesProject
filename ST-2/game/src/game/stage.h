/**
 * ステージ管理
 * - 名前空間 STG_ または stg
 * - ステージ進行, 敵の配置や敵数管理
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef STAGE_H_INCLUDED
#define STAGE_H_INCLUDED

#include "../../../../src-common/common.h"


// ---------------------------------------------------------------- private 変数. 直接触らない
extern u8 stg_nr_;
extern u8 stg_nr_enemies_;

// ---------------------------------------------------------------- ステージ, サブステージ
#define STG_STATUS_OK     0 /// 通常初期化 OK
#define STG_STATUS_CLEAR  1 /// ステージ クリア
#define STG_STATUS_ENDING 2 /// エンディングへ

/** ステージの初期化. ゲーム開始時に1回
 * @param nr_skipped_stages スキップするステージ数. 0 ならば 1 面から.
 */
void stgInit(u8 nr_skipped_stages)__z88dk_fastcall;

/** サブ ステージを1つ進めて初期化します.
 * - 場合によってはステージ クリアとか, エンディングにシーンが移行します
 * @return STG_STATUS_XXXX を返します
 */
u8 stgSubInit(void);
/** 現在のステージ数を返します */
inline u8 stgGetStageNr(void) { return stg_nr_; }

// ---------------------------------------------------------------- 敵数
/** 敵数を減らします. 残りが 0 でなければ true. 0 になったら サブ ステージを初期化し, ステージクリアなら false を返します */
inline u8 stgDecrementEnemies(void) {
    stg_nr_enemies_--;
    return stg_nr_enemies_;
}
/** 現在の敵数を返します */
inline u8 stgGetNrEnemies(void) { return stg_nr_enemies_; }

#endif  // STAGE_H_INCLUDED
