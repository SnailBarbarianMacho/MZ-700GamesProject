/**
 * アイテム オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_ITEM_H_INCLUDED
#define OBJ_ITEM_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- システム
/** システム用. テーブルの初期化 */
void objItemInitTab();

// ---------------------------------------------------------------- アイテム数
/** 生成アイテム数を初期化します */
void objItemInitStatistics();
/** 生成アイテム数を返します */
u16 objItemGetNrGeneretedItems();
/** 取得アイテム数を返します */
u16 objItemGetNrObtainedItems();

// ---------------------------------------------------------------- 初期化, メイン, 描画
void objItemInit(Obj* const pObj, Obj* const pParent);
bool objItemMain(Obj* const pObj);
void objItemDisp(Obj* const pObj, u8* dispAddr);

#endif // OBJ_ITEM_H_INCLUDED