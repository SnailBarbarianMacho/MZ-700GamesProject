/**
 * 背景の星
 * - 名前空間 STARS_ または stars
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef STARS_H_INCLUDED
#define STARS_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- 初期化, メイン
void starsInit(void) __z88dk_fastcall;
void starsMain(void) __z88dk_fastcall;

// ---------------------------------------------------------------- 制御
/** 星の描画を停止します. 次のフレームでは許可されます */
void starsSetDisabled()__z88dk_fastcall;

#endif // STARS_H_INCLUDED
