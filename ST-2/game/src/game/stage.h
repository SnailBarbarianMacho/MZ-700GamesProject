/**
 * ステージ管理
 * - 名前空間 STG_ または stg
 * - ステージ進行, 敵の配置や敵数管理
 * @author Snail Barbarian Macho (NWK)
 */
#ifndef STAGE_H_INCLUDED
#define STAGE_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- ステージ, サブステージ
#define STG_STATUS_OK     0 /// 通常初期化 OK
#define STG_STATUS_CLEAR  1 /// ステージ クリア
#define STG_STATUS_ENDING 2 /// エンディングへ

/** ステージの初期化. ゲーム開始時に1回
 * @param skipStage スキップするステージ数. 0 ならば 1 面から.
 */
void stgInit(u8 skipStage)__z88dk_fastcall;

/** サブ ステージを1つ進めて初期化します.
 * - 場合によってはステージ クリアとか, エンディングに step が移行します
 * @return STG_STATUS_XXXX を返します
 */
u8 stgSubInit();
/** 現在のステージ数を返します */
u8 stgGetStageNr();

// ---------------------------------------------------------------- 敵数
/** 敵数を減らします. 残りが 0 でなければ true. 0 になったら サブ ステージを初期化し, ステージクリアなら false を返します */
u8 stgDecrementEnemies();
/** 現在の敵数を返します */
u8 stgGetNrEnemies();

#endif  // STAGE_H_INCLUDED
