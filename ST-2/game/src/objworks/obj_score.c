/**
 * 爆発オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/obj.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "obj_score.h"


// ---------------------------------------------------------------- 初期化
void objScoreInit(Obj* const p_obj, Obj* const p_enemy)
{
    OBJ_INIT(p_obj, 0, 0, 0, 0, 0, 0);
    p_obj->ct = 32;
    p_obj->u_geo.geo8.xh = p_enemy->u_geo.geo8.xh + p_enemy->u_geo.geo8.w / 2 - 2;
    p_obj->u_geo.geo8.yh = p_enemy->u_geo.geo8.yh + p_enemy->u_geo.geo8.h / 2;
    p_obj->u_obj_work.score.score = p_enemy->u_obj_work.enemy.score * 10;
}

// ---------------------------------------------------------------- メイン
bool objScoreMain(Obj* const p_obj)
{
    if (p_obj->ct == 0) {
        return false;
    }
    p_obj->ct--;
    return true;
}

// ---------------------------------------------------------------- 描画
void objScoreDraw(Obj* const p_obj, u8* draw_addr)
{
    printSetAtb(VATB(7, 0, 0));
    printSetAddr(draw_addr);
    printU16Left(p_obj->u_obj_work.score.score);
}
