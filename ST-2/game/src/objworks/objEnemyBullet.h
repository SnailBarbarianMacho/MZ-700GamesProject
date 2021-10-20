/**
 * プレイヤー弾オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_ENEMY_BULLET_H_INCLUDED
#define OBJ_ENEMY_BULLET_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- 初期化, メイン, 描画
void objEnemyBulletInit(Obj* const pObj, Obj* const pParent);
void objEnemyBulletInitWithoutVelocity(Obj* const pObj, Obj* const pParent);// 方向は外付けで決める
bool objEnemyBulletMain(Obj* const pObj);
void objEnemyBulletDraw(Obj* const pObj, u8* drawAddr);

// ---------------------------------------------------------------- ユーティリティ
#if 0 // 当面使わない
/** 敵弾の発射位置と方向(プレーヤーへ)を決めます
 * @param xy 発射位置((X << 8) | Y). common.h の W8H8 マクロを使うと便利です
*/
void objEnemyBulletSetPosDir(Obj* const pObj, const u16 xy);
#endif

#endif // OBJ_ENEMY_H_INCLUDED