/**
 * プレイヤー弾オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_PLAYER_BULLET_H_INCLUDED
#define OBJ_PLAYER_BULLET_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- 初期化, メイン, 描画
void objPlayerBulletInit(Obj* const pObj, Obj* const pParent);
bool objPlayerBulletMain(Obj* const pObj);
void objPlayerBulletDisp(Obj* const pObj, u8* dispAddr);

#endif // OBJ_ENEMY_H_INCLUDED