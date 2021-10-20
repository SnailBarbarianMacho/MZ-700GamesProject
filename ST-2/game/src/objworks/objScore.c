/**
 * 爆発オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/obj.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "objScore.h"


// ---------------------------------------------------------------- 初期化
void objScoreInit(Obj* const pObj, Obj* const pEnemy)
{
    OBJ_INIT(pObj, 0, 0, 0, 0, 0, 0);
    pObj->ct = 32;
    pObj->uGeo.geo8.xh = pEnemy->uGeo.geo8.xh + pEnemy->uGeo.geo8.w / 2 - 2;
    pObj->uGeo.geo8.yh = pEnemy->uGeo.geo8.yh + pEnemy->uGeo.geo8.h / 2;
    pObj->uObjWork.score.score = pEnemy->uObjWork.enemy.score * 10;
}

// ---------------------------------------------------------------- メイン
bool objScoreMain(Obj* const pObj)
{
    if (pObj->ct == 0) {
        return false;
    }
    pObj->ct--;
    return true;
}

// ---------------------------------------------------------------- 描画
void objScoreDraw(Obj* const pObj, u8* drawAddr)
{
    printSetAtb(VATB(7, 0, 0));
    printSetAddr(drawAddr);
    printU16Left(pObj->uObjWork.score.score);
}
