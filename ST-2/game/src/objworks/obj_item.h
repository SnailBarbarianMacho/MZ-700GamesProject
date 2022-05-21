/**
 * アイテム オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_ITEM_H_INCLUDED
#define OBJ_ITEM_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- private 変数. 直接触らない
extern u16 obj_item_nr_generated_; // 生成したアイテム数
extern u16 obj_item_nr_obtained_;  // プレイヤーが入手したアイテム数

// ---------------------------------------------------------------- システム
/** システム用. テーブルの初期化 */
void objItemInitTab();

// ---------------------------------------------------------------- アイテム数
/** 生成アイテム数を初期化します */
inline void objItemInitStatistics() {
    obj_item_nr_generated_ = 0;
    obj_item_nr_obtained_  = 0;
}
/** 生成アイテム数を返します */
inline u16 objItemGetNrGenereted() { return obj_item_nr_generated_; }
/** 取得アイテム数を返します */
inline u16 objItemGetNrObtained() { return obj_item_nr_obtained_; }

// ---------------------------------------------------------------- 初期化, メイン, 描画
void objItemInit(Obj* const p_obj, Obj* const p_parent);
bool objItemMain(Obj* const p_obj);
void objItemDraw(Obj* const p_obj, u8* draw_addr);

#endif // OBJ_ITEM_H_INCLUDED