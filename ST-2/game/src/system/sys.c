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
#include "../scenes/sceneTitle.h"

// ---------------------------------------------------------------- 変数
static void (*_sysSceneMainFunc)(u16 counter);     // シーン メイン関数
u8      _sysCt;                 // システム カウンタ
u16     _sysSceneCt;            // シーン カウンタ
bool    _bSysGame;              // ゲームモード
u8      _sysSceneWork[8];        // シーンが自由に使えるワーク

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
    _bSysGame = false;
}

void sysMain()
{
    // ゲーム モードでないならば,
    if (_bSysGame == false) {
        u8 trg = inputGetTrigger();
        // 右ボタンでスキップ
        if (trg & INPUT_MASK_R) {
            _sysSceneCt = 0;
        }
        // タイトル画面以外でスタートでタイトルへ
        if (trg & INPUT_MASK_P) {
            if (_sysSceneMainFunc != sceneTitleMain) {
                vramFill(0x0000);
                sysSetScene(sceneTitleInit, sceneTitleMain);
                return;
            }
        }
    }

    _sysSceneMainFunc(_sysSceneCt);
    _sysSceneCt --;
    _sysCt ++;
}

// ---------------------------------------------------------------- ゲーム モード

// ---------------------------------------------------------------- シーン
void sysSetScene(void (*initFunc)(), void (*mainFunc)(u16))
{
    _sysSceneMainFunc = mainFunc;
    sdSetEnabled(false);
    sdPlayBgm(BGM_NONE);
    for (int i = 0; i < sizeof(_sysSceneWork); i++) {
        _sysSceneWork[i] = 0;
    }
    initFunc();
}
