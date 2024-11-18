/**
 * スコア, ハイスコア, レベル, 残機, ゲームモード+ルール
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../../../../src-common/hard.h"
#include "../system/addr.h"
#include "../system/sys.h"
#include "../system/vram.h"
#include "../system/sound.h"
#include "../system/math.h"
#include "../system/print.h"
#include "../system/input.h"
#include "../game/game_mode.h"
#include "../objworks/obj_item.h"
#include "../objworks/obj_enemy.h"
#include "se.h"
#include "score.h"

// ---------------------------------------------------------------- 変数
u16  score_;
u16  score_for_life_;  // 残機を増やす用のスコア
u16  score_hi_score_;
u16  score_hi_score_caravan_;
u8   score_level_;
u8   score_sub_level_;
u8   score_left_;
bool b_score_enabled_;
u16  score_nr_continues_;
u16  score_nr_misses_;
#if DEBUG
const u8* score_step_str_;
#endif

// ---------------------------------------------------------------- マクロ
#define LEFT        2         // 残機数(自機含まず)
#if DEBUG
#define START_LEVEL_EASY 50   // 最初のレベル(EASY)
#define START_LEVEL 1         // 最初のレベル
//#define START_LEVEL 130       // TEST デバッグ用
#else
#define START_LEVEL_EASY 50   // 最初のレベル(EASY)
#define START_LEVEL 1         // 最初のレベル
#endif

// ---------------------------------------------------------------- 初期化
void scoreInit(void) __z88dk_fastcall
{
    scoreGameStart();
    score_hi_score_   = 500;
    score_left_       = 0;
    b_score_enabled_  = true;

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
    score_step_str_ = nullptr;
#endif
}

// ---------------------------------------------------------------- メイン
/** 555 カーソルタイマーの点滅をゲット */
static u8 cursorBlank(void) __naked
{
__asm
    BANK_VRAM_MMIO(C)           // バンク切替
    LD      A, (MMIO_8255_PORTC)
    BANK_RAM(C)                 // バンク切替
    AND     A, 0 + MMIO_8255_PORTC_556OUT_MASK
    LD      L, A
    RET
__endasm;
}


static const u8 STR_1UP_[]  = { DC_1, DC_U, DC_P, 0 };
static const u8 STR_HI_[]   = { DC_COL7, DC_H, DC_I, 0 };
static const u8 SUB_LEVEL_TEXT_TAB_[] = { // サブ レベル 4 * 4 文字 * 7段階
    0x37, 0x7b, 0x3f, 0x43,
};
#if DEBUG
static const u8 STR_MS_[] = { DC_CAPS, DC_M, DC_S, DC_CAPS, 0 };// msec
#endif
#include "../../text/game_over.h"
#include "../../text/input_am7j.h"
#include "../../text/input_mz1x03.h"
void scoreMain(void) __z88dk_fastcall
{
    if (!b_score_enabled_) {
        b_score_enabled_ = true;
        return;
    }

    // -------- スコア表示
    printSetAtb(VATB(7, 0, 0));
    if (score_hi_score_ < score_) {
        printSetAtb(VATB(6, 0, 0));    // ハイ スコア突破したら色が変わる
    }
    printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 0));
    if (sysIsGameMode()) {
        if (cursorBlank()) {
            printString(STR_1UP_);        // プレイ中は点滅します
        } else {
            printAddAddr(3);
        }
    } else {
        printString(STR_1UP_);
    }
    printAddAddr(1);
    printU16Right(score_);
    printPutc(DC_0);

    // -------- ハイ スコア表示
    printSetAddr((u8*)VVRAM_TEXT_ADDR(13, 0));
    printString(STR_HI_);
    printAddAddr(1);
    printU16Right(score_hi_score_);
    printPutc(DC_0);

    // -------- 残機表示
    if (!gameCanIncLeft()) {
        if (gameIsCaravan()) {          // キャラバン モード
            printSetAtb(VATB(4, 0, 0));
            if      (gameGetTimer() < 10 * GAME_FPS) { printSetAtb(VATB(2, 0, 0)); }
            else if (gameGetTimer() < 100 * GAME_FPS) { printSetAtb(VATB(6, 0, 0)); }
            printSetAddr((u8*)VVRAM_TEXT_ADDR(35, 0));
            printU16LeftDecimal1(gameGetTimer() / GAME_FPS, gameGetTimer() % GAME_FPS);
        } else {                        // サバイバル, むぼう
            // 時間表示
            printSetAtb(VATB(2, 0, 0));
            printSetAddr((u8*)VVRAM_TEXT_ADDR(35, 0));
            if (gameGetTimer() < 1000) {
                printU16LeftDecimal1(gameGetTimer(), gameGetSubtimer());
            } else {
                printU16Left(gameGetTimer());
            }
        }
        printSetAtb(VATB(7, 0, 0));
    } else {
        // 残機表示
        u8  left = score_left_;
        if (5 < left) { left = 5; }
        u8* t_addr = (u8*)VVRAM_TEXT_ADDR(35, 0);
        u8* a_addr = (u8*)VVRAM_ATB_ADDR(35, 0);
        for (; 0 < left; left--) {
            *t_addr ++ = 0xc9;
            *a_addr ++ = VATB(5, 0, 0);
        }
    }

    if (!sysIsGameMode()) {
        // -------- ゲーム オーバー表示
        printSetAddr((u8*)VVRAM_TEXT_ADDR(15, 15));
        printString(text_game_over);

        // -------- AM7J / MZ-1X03 検出表示
        if (INPUT_JOY_MODE_AM7J_DETECTED <= inputGetJoyMode() && inputGetJoyMode() < INPUT_JOY_MODE_MZ1X03_DETECTING) {
            printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 24));
            printString(text_input_am7j);
        } else if (INPUT_JOY_MODE_MZ1X03_DETECTED <= inputGetJoyMode() && inputGetJoyMode() <= INPUT_JOY_MODE_MAX) {
            printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 24));
            printString(text_input_mz1x03);
        }
    } else {
        // -------- レベル表示
        printSetAddr((u8*)VVRAM_TEXT_ADDR(25, 0));
        printPutc(DC_L);
        printPutc(DC_V);
        printAddAddr(4);
        printU8Left(score_level_);

        // サブ レベル 4 * 4 文字 * 7段階
        u8* t_addr = (u8*)VVRAM_TEXT_ADDR(27, 0);
        u8* a_addr = (u8*)VVRAM_ATB_ADDR(27, 0);
        u8 sl  = score_sub_level_;
        u8 j   = (sl >> 2) & 3;
        u8 atb = ((u8*)ADDR_SUB_LEVEL)[sl & 0xf0];
        for (u8 i = 0; i < j; ++i) {
            *t_addr++ = 0x43;
            *a_addr++  = atb;
        }
        *t_addr++ = SUB_LEVEL_TEXT_TAB_[sl & 0x03];
        *a_addr++  = ((u8*)ADDR_SUB_LEVEL)[sl];
        atb = ((u8*)ADDR_SUB_LEVEL)[(sl & 0xf0) + 2];
        for (u8 i = j + 1; i < 4; i++) {
            *t_addr++ = 0x43;
            *a_addr++  = atb;
        }
    }

#if DEBUG
    // 入力, 処理時間, 現在のシーン表示
#if 1 // 入力と Vカウンタ表示
    printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 22));
    printHex8(inputGet());
//    printHex9(inputGetTrigger());
    printHex8(inputGetJoy());
    printU8Right(inputGetJoyMode());
//    printU16Left(objEnemyGetNrKilled());
//    printHex16(vramGet8253Ct2());
//    printHex16(vramTransGetCounter());
//    printU16Right(vramGetVCounter());
#endif

    printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 23));
    printU16Left(vramDebugGetProcessTime()); printString(STR_MS_);

    if (score_step_str_) {
        printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 24));
        printString(score_step_str_);
    }
#endif
}

// ---------------------------------------------------------------- 制御

// ---------------------------------------------------------------- スタート, コンティニュー
void scoreGameStart(void)__z88dk_fastcall
{
    if (!sysIsGameMode()) {
        return;
    }
    scoreContinue();
    score_sub_level_    = 0;
    score_nr_continues_ = 0;
    score_nr_misses_    = 0;
    score_level_        = (gameGetMode() == GAME_MODE_EASY) ? START_LEVEL_EASY : START_LEVEL;
#if DEBUG
    score_              = 0;
    score_for_life_     = SCORE_BONUS_SHIP - SCORE_BONUS_SHIP_1;
#endif
    objItemInitStatistics();
    objEnemyInitStatistics();
}

void scoreContinue(void)__z88dk_fastcall
{
#if DEBUG
#else
    score_          = 0;
    score_for_life_ = SCORE_BONUS_SHIP - SCORE_BONUS_SHIP_1;
#endif
    score_nr_continues_++;
    //score_           = 300;//DEBUG
    //score_for_life_ += score_; // DEBUG
    //score_sub_level_ = 0; // レベルは下がらないが, サブレベルは 0 から再開
    score_left_        = 0; // サバイバル, むぼう, キャラバン モードは残機なし
    if (gameCanIncLeft()) { score_left_ = LEFT; }
}

// ---------------------------------------------------------------- スコア, ハイ スコア
bool scoreAdd(const u16 score)__z88dk_fastcall
{
    if (sysIsGameMode()) {
        score_ = addSaturateU16(score_, score);
        if (gameCanIncLeft() && (score_ != 0xffff)) { // ゲーム モードによっては残機は増えない. カンストしたら残機は増えない
            score_for_life_ += score;
        }
        if (SCORE_BONUS_SHIP < score_for_life_) {
            score_for_life_ -= SCORE_BONUS_SHIP;
            score_left_++;
            sdPlaySe(SE_1UP);
            return true;
        }
    }
    return false;
}


bool scoreReflectHiScore(void)__z88dk_fastcall
{
    if (score_hi_score_ < score_) {
        score_hi_score_ = score_;
        return true;
    }
    return false;
}


// ---------------------------------------------------------------- 残機
bool scoreDecrementLeft(void)__z88dk_fastcall
{
    if (sysIsGameMode()) {
        score_nr_misses_++;
        if (gameIsCaravan()) {  // キャラバン モードは何回死んでもいい
            return false;
        }
        if (score_left_ == 0) {
            return true;
        }
        if (gameGetMode() == GAME_MODE_HARD) {  // ハードモードではレベル 10% 引
#pragma disable_warning 110 // 除算最適化警告
#pragma save
            score_level_ -= score_level_ / 8;   // 実際は 8%
#pragma restore
            score_sub_level_ = 0;
        }
        score_left_ --;
    }
    return false;
}

// ---------------------------------------------------------------- レベル
void scoreAddSubLevel(const u8 sub_level)__z88dk_fastcall
{
    if (score_level_ != 255) {    // カンスト
        score_sub_level_ += sub_level;
        if (4 * 4 * 7 <= score_sub_level_) {
            score_sub_level_ -= 4 * 4 * 7;
            score_level_++;
            if (score_level_ == 255) {
                score_sub_level_ = 0;
            }
            sdPlaySe(SE_LEVEL_UP);
        }
    }
}

// ---------------------------------------------------------------- デバッグ
