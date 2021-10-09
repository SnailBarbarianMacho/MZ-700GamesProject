/**
 * スコア表示オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_SCORE_H_INCLUDED
#define OBJ_SCORE_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- 初期化, メイン, 描画
void objScoreInit(Obj* const pObj, Obj* const pEnemy);
bool objScoreMain(Obj* const pObj);
void objScoreDisp(Obj* const pObj, u8* dispAddr);

#endif // OBJ_SCORE_H_INCLUDED