/**
 * 爆発オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/obj.h"
#include "../system/vram.h"
#include "../system/sound.h"
#include "../system/math.h"
#include "../../cg/Explosion0.h"
#include "../../cg/Explosion1.h"
#include "../../cg/Explosion2.h"
#include "../../cg/Explosion3.h"
#include "../../cg/Explosion4.h"
#include "../../cg/Explosion5.h"
#include "../../cg/Explosion6.h"
#include "../../cg/Explosion7.h"
#include "objPlayerBullet.h"


// ---------------------------------------------------------------- 初期化
void objExplosionInit(Obj* const pObj, Obj* const pParent)
{
    OBJ_INIT(pObj, 0, 0, 0, 0, 0, 0);
    pObj->ct = 8;
    pObj->uGeo.geo8.xh = pParent->uGeo.geo8.xh;
    pObj->uGeo.geo8.yh = pParent->uGeo.geo8.yh;
    if (3 < pParent->uGeo.geo8.h) { // w は死亡時に 0 になるので使わない
        u8 h2 = pParent->uGeo.geo8.h - 2;
        pObj->uGeo.geo8.xh += rand8() % h2;
        pObj->uGeo.geo8.yh += rand8() % h2;
    }
}
void objExplosionPlayerInit(Obj* const pObj, Obj* const pParent)
{
    OBJ_INIT(pObj, 0, 0, 0, 0, 0, 0);
    pObj->ct = 16;
    pObj->uGeo.geo8.xh = pParent->uGeo.geo8.xh - 1;
    pObj->uGeo.geo8.yh = pParent->uGeo.geo8.yh;
}

// ---------------------------------------------------------------- メイン
bool objExplosionMain(Obj* const pObj)
{
    if (pObj->ct == 0) {
        return false;
    }
    pObj->ct--;
    return true;
}
bool objExplosionPlayerMain(Obj* const pObj)
{
    if (pObj->ct == 0) {
        return false;
    }
    pObj->ct--;
    return true;
}

// ---------------------------------------------------------------- 描画
void objExplosionDraw(Obj* const pObj, u8* drawAddr)
{
    static const u8* const pExplosionTab[] = {
        sExplosion7, sExplosion6, sExplosion5, sExplosion4,
        sExplosion3, sExplosion2, sExplosion1, sExplosion0,
    };
    vVramDraw3x3Transparent(drawAddr, pExplosionTab[pObj->ct]);
}
void objExplosionPlayerDraw(Obj* const pObj, u8* drawAddr)
{
    static const u8* const pExplosionTab[] = {
        sExplosion7, sExplosion6, sExplosion7, sExplosion6,
        sExplosion5, sExplosion4, sExplosion5, sExplosion4,
        sExplosion3, sExplosion2, sExplosion3, sExplosion2,
        sExplosion1, sExplosion0, sExplosion1, sExplosion0,
    };
    vVramDraw3x3Transparent(drawAddr, pExplosionTab[pObj->ct]);
}
