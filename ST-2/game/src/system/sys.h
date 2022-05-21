/**
 * システム
 * - 名前空間 SYS_ または sys
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef SYS_H_INCLUDED
#define SYS_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- private 変数. 直接触らない
extern u8   sys_ct_;
extern u16  sys_scene_ct_;
extern bool b_sys_game_;
extern u8   sys_scene_work_[8];

// ---------------------------------------------------------------- システム
/** システムの初期化を行います */
void sysInit();

/** 毎フレームの最初に呼んでください */
void sysMain();

/** 毎フレーム + 1 されるシステム カウンタを返します */
inline u8 sysGetCounter() { return sys_ct_; }

// ---------------------------------------------------------------- ゲーム モード(アトラクトorゲーム)
/** ゲーム モードをセットします(false/true = アトラクト/ゲーム). 初期値:false */
inline void sysSetMode(const bool b_game) { b_sys_game_ = b_game; }
/** 現在アトラクト モード(false)かゲーム モード(true)かを返します */
inline bool sysIsGameMode() { return b_sys_game_; }

// ---------------------------------------------------------------- シーン
/** シーンを設定します
 * - サウンドはオフ (sdSetEnabled(false)) となります
 * - シーンの Main 関数内からのみ呼んでください.
 *   オブジェクトからは呼んでは行けません(無限ループになります)
 * @param init_func シーン初期化関数
 * @param main_func 毎フレーム呼ばれる関数.
 * - 引数はシーンカウンタで, 0 になったら次のシーンへ移るようにしてください
 */
void sysSetScene(void (*init_func)(), void (*main_func)(u16));

/**
 * シーン カウンタを設定します.
 * 通常 sysSetScene() 内の init_func() の中で呼ばれます
 */
inline void sysSetSceneCounter(const u16 counter) { sys_scene_ct_  = counter; }

/** 各シーンで自由に使えるワークを返します */
inline u8 sysSceneGetWork(const u8 nr) { return sys_scene_work_[nr]; }
/** 各シーンで自由に使えるワークをセットします */
inline void sysSceneSetWork(const u8 nr, const u8 val) { sys_scene_work_[nr] = val; }
/** 各シーンで自由に使えるワークを + 1 します */
inline u8 sysSceneIncWork(const u8 nr) { return ++sys_scene_work_[nr]; }
/** 各シーンで自由に使えるワークを + 1 します */
inline u8 sysSceneDecWork(const u8 nr) { return --sys_scene_work_[nr]; }

/** 各シーンで自由に使えるワークを返します(16bit版) */
inline u16  sysSceneGetWork16(const u8 nr) { return ((u16*)sys_scene_work_)[nr]; }
inline void sysSceneSetWork16(const u8 nr, u16 val) { ((u16*)sys_scene_work_)[nr] = val; }


#endif // SYS_H_INCLUDED
