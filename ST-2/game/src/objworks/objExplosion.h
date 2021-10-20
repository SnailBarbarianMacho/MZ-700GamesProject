/**
 * 爆発オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_EXPLOSION_H_INCLUDED
#define OBJ_EXPLOSION_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- 初期化, メイン, 描画
void objExplosionInit(Obj* const pObj, Obj* const pParent);
void objExplosionPlayerInit(Obj* const pObj, Obj* const pParent);
bool objExplosionMain(Obj* const pObj);
void objExplosionDraw(Obj* const pObj, u8* drawAddr);
void objExplosionPlayerDraw(Obj* const pObj, u8* drawAddr);


#endif // EXPLOSION_H_INCLUDED