/**
 * スコア表示オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_SCORE_H_INCLUDED
#define OBJ_SCORE_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- 初期化, メイン, 描画
void objScoreInit(Obj* const p_obj, Obj* const p_enemy);
bool objScoreMain(Obj* const p_obj);
void objScoreDraw(Obj* const p_obj, u8* draw_addr);

#endif // OBJ_SCORE_H_INCLUDED