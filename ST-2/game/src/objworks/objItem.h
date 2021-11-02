/**
 * アイテム オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_ITEM_H_INCLUDED
#define OBJ_ITEM_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- private 変数. 直接触らない
extern u16 _objItemNrGenereted;
extern u16 _objItemNrObtained;

// ---------------------------------------------------------------- システム
/** システム用. テーブルの初期化 */
void objItemInitTab();

// ---------------------------------------------------------------- アイテム数
/** 生成アイテム数を初期化します */
inline void objItemInitStatistics() {
    _objItemNrGenereted = 0;
    _objItemNrObtained = 0;
}
/** 生成アイテム数を返します */
inline u16 objItemGetNrGenereted() { return _objItemNrGenereted; }
/** 取得アイテム数を返します */
inline u16 objItemGetNrObtained() { return _objItemNrObtained; }

// ---------------------------------------------------------------- 初期化, メイン, 描画
void objItemInit(Obj* const pObj, Obj* const pParent);
bool objItemMain(Obj* const pObj);
void objItemDraw(Obj* const pObj, u8* drawAddr);

#endif // OBJ_ITEM_H_INCLUDED