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
#include "../steps/stepTitle.h"

// ---------------------------------------------------------------- 変数
static void (*sStepMainFunc)(u16 counter);     // ステップ メイン関数
static u8   sSystemCt;              // システム カウンタ
static u16  sStepCounter;           // ステップ カウンタ
static bool sbGameMode;             // ゲームモード

// ---------------------------------------------------------------- システム
void sysInit(void (*initFunc)(), void (*mainFunc)(u16))
{
#if DEBUG
    // ゴミで埋める
    for (u8* p = (u8*)ADDR_SD3_ATT_TAB - 1; p != 0x0000; ) {
        --p;
        *p = 0xff;
    }
#endif
    sbGameMode = false;
    sysSetStep(initFunc, mainFunc);
}

void sysMain(void) __z88dk_fastcall
{
    // ゲーム モードでないならば,
    if (sbGameMode == false) {
        u8 trg = inputGetTrigger();
        // 右ボタンでスキップ
        if (trg & INPUT_MASK_R) {
            sStepCounter = 0;
        }
        // タイトル画面以外でスタートでタイトルへ
        if (trg & INPUT_MASK_P) {
            if (sStepMainFunc != stepTitleMain) {
                vramFill(0x0000);
                sysSetStep(stepTitleInit, stepTitleMain);
                return;
            }
        }
    }

    sStepMainFunc(sStepCounter);
    sStepCounter --;
    sSystemCt ++;
}

u8 sysGetCounter() __z88dk_fastcall
{
    return sSystemCt;
}

// ---------------------------------------------------------------- ゲーム モード
void sysSetGameMode(const bool bGameMode) __z88dk_fastcall
{
    sbGameMode = bGameMode;
}

bool sysIsGameMode() __z88dk_fastcall
{
    return sbGameMode;
}

// ---------------------------------------------------------------- ステップ
void sysSetStep(void (*initFunc)(), void (*mainFunc)(u16))
{
    sStepMainFunc = mainFunc;
    sdSetEnabled(false);
    sdSetBgmSequencer(nullptr, nullptr);
    initFunc();
}

void sysSetStepCounter(const u16 counter) __z88dk_fastcall
{
    sStepCounter  = counter;
}
