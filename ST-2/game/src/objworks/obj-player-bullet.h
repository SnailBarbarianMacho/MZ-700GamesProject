/**
 * プレイヤー弾オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_PLAYER_BULLET_H_INCLUDED
#define OBJ_PLAYER_BULLET_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- 初期化, メイン, 描画
void objPlayerBulletInit(Obj* const p_obj, Obj* const p_parent);
bool objPlayerBulletMain(Obj* const p_obj);
void objPlayerBulletDraw(Obj* const p_obj, u8* draw_addr);

#endif // OBJ_ENEMY_H_INCLUDED