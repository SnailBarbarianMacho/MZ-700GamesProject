/**
 * プレーヤー オブジェクト
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_PLAYER_H_INCLUDED
#define OBJ_PLAYER_H_INCLUDED

#include "../../../../src-common/common.h"
#include "../system/obj.h"

// ---------------------------------------------------------------- マクロ
#define OBJ_PLAYER_STEP_NORMAL          0   // 通常状態. ct != 0 ならば, 無敵になります.
#define OBJ_PLAYER_STEP_DEAD            1   // 死亡
#define OBJ_PLAYER_STEP_CONTINUE        2   // コンティニュー
#define OBJ_PLAYER_STEP_END_SURVIVAL    3   // サバイバル モード終了
#define OBJ_PLAYER_STEP_END_CARAVAN     4   // キャラバン モード終了
#define OBJ_PLAYER_STEP_DEMO            5   // デモ モード用. 外部で制御します

// ---------------------------------------------------------------- 初期化, メイン, 描画
void objItemInit(Obj* const pObj, Obj* const pParent);
void objPlayerInit(Obj* const pObj, Obj* const pParent);
bool objPlayerMain(Obj* const pObj);
void objPlayerDraw(Obj* const pObj, u8* drawAddr);

// ---------------------------------------------------------------- ユーティリティ
/** プレーヤーを通常状態にします */
void objPlayerSetNormalStep(Obj* const pPlayer);

#endif // OBJ_PLAYER_H_INCLUDE