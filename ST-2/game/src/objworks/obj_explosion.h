/**
 * 爆発オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_EXPLOSION_H_INCLUDED
#define OBJ_EXPLOSION_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- 初期化, メイン, 描画
void objExplosionInit(Obj* const p_obj, Obj* const p_parent);
void objExplosionPlayerInit(Obj* const p_obj, Obj* const p_parent);
bool objExplosionMain(Obj* const p_obj);
void objExplosionDraw(Obj* const p_obj, u8* draw_addr);
void objExplosionPlayerDraw(Obj* const p_obj, u8* draw_addr);


#endif // EXPLOSION_H_INCLUDED