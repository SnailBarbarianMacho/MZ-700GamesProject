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
extern bool _bStarsEnabled;

// ---------------------------------------------------------------- 初期化, メイン
void starsInit();
void starsMain();

// ---------------------------------------------------------------- 制御
/** 星の描画を停止します. 次のフレームでは許可されます */
inline void starsSetDisabled() { _bStarsEnabled = false; }

#endif // STARS_H_INCLUDED
