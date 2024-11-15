/**
 * 背景の星
 * - 名前空間 STARS_ または stars
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef STARS_H_INCLUDED
#define STARS_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- private 変数. 直接触らない
extern bool b_stars_enabled_;

// ---------------------------------------------------------------- 初期化, メイン
void starsInit(void);
void starsMain(void);

// ---------------------------------------------------------------- 制御
/** 星の描画を停止します. 次のフレームでは許可されます */
inline void starsSetDisabled(void) { b_stars_enabled_ = false; }

#endif // STARS_H_INCLUDED
