/**
 * 敵オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_ENEMY_H_INCLUDED
#define OBJ_ENEMY_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- private 変数. 直接触らない
extern u16 _objEnemyNrKilled;

// ---------------------------------------------------------------- 初期化
void objEnemyInit3_1(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit3_2(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit3_3(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit3_4(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit3_5(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit3_6(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit4_1(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit4_2(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit4_3(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit5_1(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit5_2(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit5_3(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit8_1(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit8_2(Obj* const p_obj, Obj* const p_parent);
void objEnemyInit8_3(Obj* const p_obj, Obj* const p_parent);

// ---------------------------------------------------------------- メイン
bool objEnemyMain3_1(Obj* const p_obj);
bool objEnemyMainDemo3_1(Obj* const p_obj); // Instruction 用
#define objEnemyMain3_2 objEnemyMain3_1
bool objEnemyMain3_3(Obj* const p_obj);
#define objEnemyMain3_4 objEnemyMain3_3
bool objEnemyMain3_5(Obj* const p_obj);
bool objEnemyMain3_6(Obj* const p_obj);
bool objEnemyMain4_1(Obj* const p_obj);
bool objEnemyMain4_2(Obj* const p_obj);
#define objEnemyMain4_2 objEnemyMain4_1
bool objEnemyMain4_3(Obj* const p_obj);
bool objEnemyMain5_1(Obj* const p_obj);
bool objEnemyMain5_2(Obj* const p_obj);
#define objEnemyMain5_3 objEnemyMain5_1
bool objEnemyMain8_1(Obj* const p_obj);
bool objEnemyMain8_2(Obj* const p_obj);
bool objEnemyMain8_3(Obj* const p_obj);

// ---------------------------------------------------------------- 描画
void objEnemyDraw3_1(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw3_2(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw3_3(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw3_4(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw3_5(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw3_6(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw4_1(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw4_2(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw4_3(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw5_1(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw5_2(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw5_3(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw8_1(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw8_2(Obj* const p_obj, u8* draw_addr);
void objEnemyDraw8_3(Obj* const p_obj, u8* draw_addr);

// ---------------------------------------------------------------- 倒した敵の数
/** 倒した敵の数を初期化します */
inline void objEnemyInitStatistics() { _objEnemyNrKilled = 0; }
/** 倒した敵の数を返します */
inline u16 objEnemyGetNrKilled() { return _objEnemyNrKilled; }

#endif // OBJ_ENEMY_H_INCLUDED