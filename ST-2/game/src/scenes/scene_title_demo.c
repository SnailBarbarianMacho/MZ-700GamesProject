/**
 * タイトル デモ シーン
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/input.h"
#include "../system/vram.h"
#include "../system/print.h"
#include "../system/math.h"
#include "../system/obj.h"
#include "../game/score.h"
#include "scene_title.h"
#include "scene_title_demo.h"

// ---------------------------------------------------------------- マクロ
#define STEP_CT 120


// ---------------------------------------------------------------- 初期化
#if DEBUG
static const u8 SCENE_NAME_[] = { DC_T, DC_D, DC_E, DC_M, DC_O, 0, };
#endif
void sceneTitleDemoInit(void)
{
    objInit();
#if DEBUG
    scoreSetStepString(SCENE_NAME_);
#endif
    sysSetSceneCounter(STEP_CT);
}


// ---------------------------------------------------------------- メイン
static void sceneWait1sec(void) __z88dk_fastcall __naked {//TEST
__asm
// 3579545 サイクルで 1 秒
ld BC, 57734        // 10
LOOP:
    dec BC          // 6
    ex  (SP), HL    // 19
    ex  (SP), HL    // 19
    ld  A, B        // 4
    or  A, C        // 4
    jp  nz, LOOP    // 10/10
    ret             // 10
    //  (6+19+19+4+4+10) * 57734 + 10 + 10 = 3579528
__endasm;
}

void sceneTitleDemoMain(u16 scene_ct)
{
    s16 ct = ((s16)STEP_CT - scene_ct) / 2 - 20;
    printSetAddr((u8*)VVRAM_TEXT_ADDR(3, 2)); printStringWithLength(sceneTitleGetStrNwkPresents(), ct);

    // -------- タイトル表示
#if 0
u16 param = 0xc000 | scene_ct;
s8 f = sceneTitleTest(param);// TEST
printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 19)); printHex16(param);
printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 20));
printPutc(f & 0x80 ? CHAR_S : CHAR_HYPHEN);
printPutc(f & 0x40 ? CHAR_Z : CHAR_HYPHEN);
printPutc(CHAR_PERIOD);
printPutc(f & 0x10 ? CHAR_H : CHAR_HYPHEN);
printPutc(CHAR_PERIOD);
printPutc(f & 0x04 ? CHAR_P : CHAR_HYPHEN);
printPutc(f & 0x02 ? CHAR_N : CHAR_HYPHEN);
printPutc(f & 0x01 ? CHAR_C : CHAR_HYPHEN);
for (u16 i = 0; i < 40000; i++);
#endif

    sceneTitleDrawTitleS(     (u8*)VVRAM_TEXT_ADDR(clampS16(scene_ct - 10 +  7,  7, VRAM_WIDTH), 5));
    sceneTitleDrawTitleT(     (u8*)VVRAM_TEXT_ADDR(clampS16(scene_ct - 10 + 14, 14, VRAM_WIDTH), 5));
    sceneTitleDrawTitleHyphen((u8*)VVRAM_TEXT_ADDR(clampS16(scene_ct - 10 + 21, 21, VRAM_WIDTH), 8));
    sceneTitleDrawTitle2(     (u8*)VVRAM_TEXT_ADDR(clampS16(scene_ct - 10 + 26, 26, VRAM_WIDTH), 5));
    if (scene_ct == 0)  {
        sysSetScene(sceneTitleInit, sceneTitleMain);
    }
}
