/**
 * システム
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "addr.h"
#include "sound.h"
#include "input.h"
#include "vram.h"
#include "sys.h"
#include "../game/bgm.h"
#include "../scenes/scene_title.h"

// ---------------------------------------------------------------- 変数
static void (*sys_scene_main_func_)(u16 counter);     // シーン メイン関数
u8      sys_ct_;                 // システム カウンタ
u16     sys_scene_ct_;           // シーン カウンタ
bool    b_sys_game_;             // ゲームモード
u8      sys_scene_work_[8];      // シーンが自由に使えるワーク

// ---------------------------------------------------------------- システム
void sysInit()
{
#if DEBUG
    // ゴミで埋める
    for (u8* p = (u8*)ADDR_SD3_ATT_TAB - 1; p != 0x0000; ) {
        --p;
        *p = 0xff;
    }
#endif
    b_sys_game_ = false;
}

void sysMain()
{
    // ゲーム モードでないならば,
    if (b_sys_game_ == false) {
        u8 trg = inputGetTrigger();
        // 右ボタンでスキップ
        if (trg & INPUT_MASK_R) {
            sys_scene_ct_ = 0;
        }
        // タイトル画面以外でスタートでタイトルへ
        if (trg & INPUT_MASK_P) {
            if (sys_scene_main_func_ != sceneTitleMain) {
                vramFill(0x0000);
                sysSetScene(sceneTitleInit, sceneTitleMain);
                return;
            }
        }
    }

    sys_scene_main_func_(sys_scene_ct_);
    sys_scene_ct_ --;
    sys_ct_ ++;
}

// ---------------------------------------------------------------- ゲーム モード

// ---------------------------------------------------------------- シーン
void sysSetScene(void (*init_func)(), void (*main_func)(u16))
{
    sys_scene_main_func_ = main_func;
    sdSetEnabled(false);
    sdPlayBgm(BGM_NONE);
    for (int i = 0; i < sizeof(sys_scene_work_); i++) {
        sys_scene_work_[i] = 0;
    }
    init_func();
}
