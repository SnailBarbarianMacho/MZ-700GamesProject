/**
 * プレイヤー弾オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_ENEMY_BULLET_H_INCLUDED
#define OBJ_ENEMY_BULLET_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- 初期化, メイン, 描画
void objEnemyBulletInit(Obj* const p_obj, Obj* const p_parent);
void objEnemyBulletInitWithoutVelocity(Obj* const p_obj, Obj* const p_parent);// 方向は外付けで決める
bool objEnemyBulletMain(Obj* const p_obj);
void objEnemyBulletDraw(Obj* const p_obj, u8* draw_addr);

// ---------------------------------------------------------------- ユーティリティ
#if 0 // 当面使わない
/** 敵弾の発射位置と方向(プレーヤーへ)を決めます
 * @param xy 発射位置((X << 8) | Y). common.h の W8H8 マクロを使うと便利です
*/
void objEnemyBulletSetPosDir(Obj* const p_obj, const u16 xy);
#endif

#endif // OBJ_ENEMY_H_INCLUDED