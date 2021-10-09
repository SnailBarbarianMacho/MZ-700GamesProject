/**
 * スコア, ハイスコア, レベル, 残機
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/vram.h"
#include "../system/sound.h"
#include "../system/math.h"
#include "../system/print.h"
#if DEBUG
#include "../system/input.h"
#endif
#include "../objworks/objItem.h"
#include "score.h"

// ---------------------------------------------------------------- 変数
static u16  sScore;
static u16  sScoreForLife;  // 残機を増やす用のスコア
static u16  sHiScore;
static u8   sLevel;
static u8   sSubLevel;
static u8   sLeft;
static bool sbScoreEnabled;
static u16  sNrContinues;
#if DEBUG
static const u8* spStepString;
#endif

// ---------------------------------------------------------------- マクロ
#define LEFT        2         // 残機数(自機含まず)
#define START_LEVEL 1         // 最初のレベル
//#define START_LEVEL 130       // TEST

// ---------------------------------------------------------------- サウンド
#define SE_LEVEL_UP_CT  48
#define SE_1UP_CT       128

// レベルアップ音
static void seLevelUp(u8 ct)
{
    sdMake(((ct & 7) + (ct / 8) + 1) << 8);
}
// 1UP 音
static void se1Up(u8 ct)
{
    sdMake(((ct >> 3) & 0x1) << 8);
}

// ---------------------------------------------------------------- 初期化
void scoreInit(void) __z88dk_fastcall
{
    scoreGameStart();
    sHiScore  = 500;
    sLeft     = 0;
    sbScoreEnabled = true;

    // サブ レベル テーブル作成
    u8* p = (u8*)ADDR_SUB_LEVEL;
    for (u8 i = 0; i < 7; i++) {
        u8 atb0 = ((i + 1) << 4) | i;
        u8 atb1 =  (i << 4)      | (i + 1);
        for (u8 j = 0; j < 4; j++) {
            *p++ = atb0;
            *p++ = atb0;
            *p++ = atb1;
            *p++ = atb0;
        }
    }

#if DEBUG
    spStepString = nullptr;
#endif
}

// ---------------------------------------------------------------- メイン
void scoreMain(void) __z88dk_fastcall
{
    if (!sbScoreEnabled) {
        sbScoreEnabled = true;
        return;
    }
    static const u8 str1Up[]  = { CHAR_1, CHAR_U, CHAR_P, 0 };
    static const u8 strZero[] = { CHAR_0, 0 };
    static const u8 strHi[]   = { CHAR_ATB, VATB(7, 0, 0), CHAR_H, CHAR_I, 0 };
    static const u8 strLv[]   = { CHAR_L, CHAR_V, 0 };

    static const u8 strGameOver[] = { CHAR_ATB, VATB(7, 0, 0),  CHAR_G, CHAR_A, CHAR_M, CHAR_E, CHAR_MOVE_RIGHT, 0x02, CHAR_O, CHAR_V, CHAR_E, CHAR_R, 0 };

    // スコア
    printSetAtb(VATB(7, 0, 0));
    if (sHiScore < sScore) {
        printSetAtb(VATB(6, 0, 0));    // ハイ スコア突破したら色が変わる
    }
    printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 0));
    if (sysIsGameMode()) {
        if (sysGetCounter() & 0x10) {
            printString(str1Up);        // プレイ中は点滅します
        } else {
            printAddAddr(3);
        }
    } else {
        printString(str1Up);
    }
    printAddAddr(1);
    printU16Right(sScore);
    printString(strZero);

    // ハイ スコア
    printSetAddr((u8*)VVRAM_TEXT_ADDR(13, 0));
    printString(strHi);
    printAddAddr(1);
    printU16Right(sHiScore);
    printString(strZero);

    // 残機
    u8  left = sLeft;
    if (5 < left) { left = 5; }
    u8* tAddr = (u8*)VVRAM_TEXT_ADDR(35, 0);
    u8* aAddr = (u8*)VVRAM_ATB_ADDR(35, 0);
    for (; 0 < left; left--) {
        *tAddr ++ = 0xc9;
        *aAddr ++ = VATB(5, 0, 0);
    }

    if (!sysIsGameMode()) {
        // ゲーム オーバー表示
        printSetAddr((u8*)VVRAM_TEXT_ADDR(15, 15));
        printString(strGameOver);
    } else {
        // レベル
        printSetAddr((u8*)VVRAM_TEXT_ADDR(25, 0));
        printString(strLv);
        printAddAddr(4);
        printU8Left(sLevel);

        // サブ レベル 4 * 4 文字 * 7段階
        static const u8 subLevelTextTab[] = {
            0x37, 0x7b, 0x3f, 0x43,
        };
        u8* addrText = (u8*)VVRAM_TEXT_ADDR(27, 0);
        u8* addrAtb  = (u8*)VVRAM_ATB_ADDR(27, 0);
        u8 sl  = sSubLevel;
        u8 j   = (sl / 4) & 3;
        u8 atb = ((u8*)ADDR_SUB_LEVEL)[sl & 0xf0];
        for (u8 i = 0; i < j; ++i) {
            *addrText++ = 0x43;
            *addrAtb++  = atb;
        }
        *addrText++ = subLevelTextTab[sl & 0x03];
        *addrAtb++  = ((u8*)ADDR_SUB_LEVEL)[sl];
        atb = ((u8*)ADDR_SUB_LEVEL)[(sl & 0xf0) + 2];
        for (u8 i = j + 1; i < 4; i++) {
            *addrText++ = 0x43;
            *addrAtb++  = atb;
        }
    }

#if DEBUG
    // 現在のステップと, 処理時間表示
    //printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 22)); printHex16(inputGet());//TEST
    //printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 22)); printHex16(inputGetTrigger());//TEST
    static const u8 strMs[] = { CHAR_CAPS, CHAR_M, CHAR_S, CHAR_CAPS, 0 };
    printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 23)); printU16Left(vramDebugGetProcessTime()); printString(strMs);
    if (spStepString) {
        printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 24)); printString(spStepString);
    }
#endif
}

// ---------------------------------------------------------------- 制御
void scoreSetDisabled()__z88dk_fastcall
{
    sbScoreEnabled = false;
}

// ---------------------------------------------------------------- スタート, コンティニュー
void scoreGameStart()__z88dk_fastcall
{
    if (!sysIsGameMode()) {
        return;
    }
    scoreContinue();
    sNrContinues  = sSubLevel = 0;
    sLevel        = START_LEVEL;
#if DEBUG
    sScore        = 0;
    sScoreForLife = SCORE_BONUS_SHIP - SCORE_BONUS_SHIP_1;
#endif
    objItemInitStatistics();
}

void scoreContinue()__z88dk_fastcall
{
#if DEBUG
#else
    sScore        = 0;
    sScoreForLife = SCORE_BONUS_SHIP - SCORE_BONUS_SHIP_1;
#endif
    sNrContinues++;
    //sScore        = 300;//DEBUG
    //sScoreForLife += sScore; // DEBUG
    //sSubLevel     = 0; // レベルは下がらないが, サブレベルは 0 から再開
    sLeft         = LEFT;
}

u16 scoreGetNrContinues()__z88dk_fastcall
{
    return sNrContinues;
}

// ---------------------------------------------------------------- スコア, ハイ スコア
bool scoreAdd(const u16 score)__z88dk_fastcall
{
    if (sysIsGameMode()) {
        sScore = addSaturateU16(sScore, score);
        if (sScore != 0xffff) {
            sScoreForLife += score; // カンストしたら残機は増えない
        }
        if (SCORE_BONUS_SHIP < sScoreForLife) {
            sScoreForLife -= SCORE_BONUS_SHIP;
            sLeft++;
            sdSetSeSequencer(se1Up, SD_SE_PRIORITY_0, SE_1UP_CT);
            return true;
        }
    }
    return false;
}

bool scoreReflectHiScore()__z88dk_fastcall
{
    if (sHiScore < sScore)
    {
        sHiScore = sScore;
        return true;
    }
    return false;
}


// ---------------------------------------------------------------- 残機
bool scoreDecrementLeft()__z88dk_fastcall
{
    if (sysIsGameMode()) {
        if (sLeft == 0) {
            return true;
        }
        sLeft --;
    }
    return false;
}

// ---------------------------------------------------------------- レベル
void scoreAddSubLevel(const u8 subLevel)__z88dk_fastcall
{
    if (sLevel != 255) {    // カンスト
        sSubLevel += subLevel;
        if (4 * 4 * 7 <= sSubLevel) {
            sSubLevel -= 4 * 4 * 7;
            sLevel++;
            if (sLevel == 255) {
                sSubLevel = 0;
            }
            sdSetSeSequencer(seLevelUp, SD_SE_PRIORITY_0, SE_LEVEL_UP_CT);
        }
    }
}

u8 scoreGetLevel()__z88dk_fastcall
{
    return sLevel;
}

void scoreResetLevel() {
    sLevel = 1;
}


// ---------------------------------------------------------------- デバッグ
#if DEBUG
void scoreSetStepString(const u8* const str)__z88dk_fastcall
{
    spStepString = str;
}
#endif