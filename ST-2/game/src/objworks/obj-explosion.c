/**
 * 爆発オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/obj.h"
#include "../system/vram.h"
#include "../system/sound.h"
#include "../system/math.h"
#include "../../cg/explosion0.h"
#include "../../cg/explosion1.h"
#include "../../cg/explosion2.h"
#include "../../cg/explosion3.h"
#include "../../cg/explosion4.h"
#include "../../cg/explosion5.h"
#include "../../cg/explosion6.h"
#include "../../cg/explosion7.h"
#include "obj-player-bullet.h"


// ---------------------------------------------------------------- 初期化
void objExplosionInit(Obj* const p_obj, Obj* const p_parent)
{
    OBJ_INIT(p_obj, 0, 0, 0, 0, 0, 0);
    p_obj->ct = 8;
    p_obj->u_geo.geo8.xh = p_parent->u_geo.geo8.xh;
    p_obj->u_geo.geo8.yh = p_parent->u_geo.geo8.yh;
    if (3 < p_parent->u_geo.geo8.h) { // w は死亡時に 0 になるので使わない
        u8 h2 = p_parent->u_geo.geo8.h - 2;
        p_obj->u_geo.geo8.xh += rand8() % h2;
        p_obj->u_geo.geo8.yh += rand8() % h2;
    }
}
void objExplosionPlayerInit(Obj* const p_obj, Obj* const p_parent)
{
    OBJ_INIT(p_obj, 0, 0, 0, 0, 0, 0);
    p_obj->ct = 16;
    p_obj->u_geo.geo8.xh = p_parent->u_geo.geo8.xh - 1;
    p_obj->u_geo.geo8.yh = p_parent->u_geo.geo8.yh;
}

// ---------------------------------------------------------------- メイン
bool objExplosionMain(Obj* const p_obj)
{
    if (p_obj->ct == 0) {
        return false;
    }
    p_obj->ct--;
    return true;
}
bool objExplosionPlayerMain(Obj* const p_obj)
{
    if (p_obj->ct == 0) {
        return false;
    }
    p_obj->ct--;
    return true;
}

// ---------------------------------------------------------------- 描画
static const u8* const EXPLOSION_TAB_[] = {
    cg_explosion7, cg_explosion6, cg_explosion5, cg_explosion4,
    cg_explosion3, cg_explosion2, cg_explosion1, cg_explosion0,
};
void objExplosionDraw(Obj* const p_obj, u8* draw_addr)
{
    vVramDraw3x3Transparent(draw_addr, EXPLOSION_TAB_[p_obj->ct]);
}

static const u8* const PLAYER_EXPLOSION_TAB_[] = {
    cg_explosion7, cg_explosion6, cg_explosion7, cg_explosion6,
    cg_explosion5, cg_explosion4, cg_explosion5, cg_explosion4,
    cg_explosion3, cg_explosion2, cg_explosion3, cg_explosion2,
    cg_explosion1, cg_explosion0, cg_explosion1, cg_explosion0,
};
void objExplosionPlayerDraw(Obj* const p_obj, u8* draw_addr)
{
    vVramDraw3x3Transparent(draw_addr, PLAYER_EXPLOSION_TAB_[p_obj->ct]);
}
