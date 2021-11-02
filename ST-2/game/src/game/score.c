/**
 * スコア, ハイスコア, レベル, 残機, ゲームモード+ルール
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
#include "../game/gameMode.h"
#include "../objworks/objItem.h"
#include "../objworks/objEnemy.h"
#include "se.h"
#include "score.h"

// ---------------------------------------------------------------- 変数
u16  _score;
u16  _scoreForLife;  // 残機を増やす用のスコア
u16  _scoreHiScore;
u16  _scoreHiScoreCaravan;
u8   _scoreLevel;
u8   _scoreSubLevel;
u8   _scoreLeft;
bool _bScoreEnabled;
u16  _scoreNrContinues;
u16  _scoreNrMisses;
#if DEBUG
const u8* _scoreStepStr;
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
void scoreInit() __z88dk_fastcall
{
    scoreGameStart();
    _scoreHiScore   = 500;
    _scoreLeft      = 0;
    _bScoreEnabled  = true;

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
    _scoreStepStr = nullptr;
#endif
}

// ---------------------------------------------------------------- メイン
void scoreMain() __z88dk_fastcall
{
    if (!_bScoreEnabled) {
        _bScoreEnabled = true;
        return;
    }
    static const u8 str1Up[]  = { CHAR_1, CHAR_U, CHAR_P, 0 };
    static const u8 strHi[]   = { CHAR_COL7, CHAR_H, CHAR_I, 0 };

    // -------- スコア表示
    printSetAtb(VATB(7, 0, 0));
    if (_scoreHiScore < _score) {
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
    printU16Right(_score);
    printPutc(CHAR_0);

    // -------- ハイ スコア表示
    printSetAddr((u8*)VVRAM_TEXT_ADDR(13, 0));
    printString(strHi);
    printAddAddr(1);
    printU16Right(_scoreHiScore);
    printPutc(CHAR_0);

    // -------- 残機表示
    if (gameIsCaravan()) {
        printSetAddr((u8*)VVRAM_TEXT_ADDR(35, 0));
        u16 timer = gameGetCaravanTimer() / GAME_FPS;
        printU16Left(timer);
        printPutc(CHAR_PERIOD);
        timer = gameGetCaravanTimer() % GAME_FPS;
        static const u8 tab[] = {
            CHAR_0, CHAR_0, CHAR_0, CHAR_1,  CHAR_1, CHAR_1, CHAR_2, CHAR_2,
            CHAR_2, CHAR_2, CHAR_3, CHAR_3,  CHAR_3, CHAR_4, CHAR_4, CHAR_4,
            CHAR_5, CHAR_5, CHAR_5, CHAR_6,  CHAR_6, CHAR_6, CHAR_6, CHAR_7,
            CHAR_7, CHAR_7, CHAR_8, CHAR_8,  CHAR_8, CHAR_9, CHAR_9, CHAR_9, };
        printPutc(tab[timer]);
    } else {
        u8  left = _scoreLeft;
        if (5 < left) { left = 5; }
        u8* tAddr = (u8*)VVRAM_TEXT_ADDR(35, 0);
        u8* aAddr = (u8*)VVRAM_ATB_ADDR(35, 0);
        for (; 0 < left; left--) {
            *tAddr ++ = 0xc9;
            *aAddr ++ = VATB(5, 0, 0);
        }
    }

    if (!sysIsGameMode()) {
        // -------- ゲーム オーバー表示
#include "../../text/gameOver.h"
        printSetAddr((u8*)VVRAM_TEXT_ADDR(15, 15));
        printString(textGameOver);
    } else {
        // -------- レベル表示
        printSetAddr((u8*)VVRAM_TEXT_ADDR(25, 0));
        printPutc(CHAR_L);
        printPutc(CHAR_V);
        printAddAddr(4);
        printU8Left(_scoreLevel);

        // サブ レベル 4 * 4 文字 * 7段階
        static const u8 subLevelTextTab[] = {
            0x37, 0x7b, 0x3f, 0x43,
        };
        u8* addrText = (u8*)VVRAM_TEXT_ADDR(27, 0);
        u8* addrAtb  = (u8*)VVRAM_ATB_ADDR(27, 0);
        u8 sl  = _scoreSubLevel;
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
    // 現在のシーンと, 処理時間表示
    //printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 22)); printHex16(inputGet());//TEST
    //printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 22)); printHex16(inputGetTrigger());//TEST
    static const u8 strMs[] = { CHAR_CAPS, CHAR_M, CHAR_S, CHAR_CAPS, 0 };
    printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 23)); printU16Left(vramDebugGetProcessTime()); printString(strMs);
    if (_scoreStepStr) {
        printSetAddr((u8*)VVRAM_TEXT_ADDR(0, 24)); printString(_scoreStepStr);
    }
#endif
}

// ---------------------------------------------------------------- 制御

// ---------------------------------------------------------------- スタート, コンティニュー
void scoreGameStart()__z88dk_fastcall
{
    if (!sysIsGameMode()) {
        return;
    }
    scoreContinue();
    _scoreSubLevel    = 0;
    _scoreNrContinues = 0;
    _scoreNrMisses    = 0;
    _scoreLevel       = (gameGetMode() == GAME_MODE_EASY) ? START_LEVEL_EASY : START_LEVEL;
#if DEBUG
    _score            = 0;
    _scoreForLife     = SCORE_BONUS_SHIP - SCORE_BONUS_SHIP_1;
#endif
    objItemInitStatistics();
    objEnemyInitStatistics();
}

void scoreContinue()__z88dk_fastcall
{
#if DEBUG
#else
    _score        = 0;
    _scoreForLife = SCORE_BONUS_SHIP - SCORE_BONUS_SHIP_1;
#endif
    _scoreNrContinues++;
    //_score        = 300;//DEBUG
    //_scoreForLife += _score; // DEBUG
    //_scoreSubLevel     = 0; // レベルは下がらないが, サブレベルは 0 から再開
    _scoreLeft         = (GAME_MODE_SURVIVAL <= gameGetMode()) ? 0 : LEFT; // サバイバル, キャラバン モードは残機なし
}

// ---------------------------------------------------------------- スコア, ハイ スコア
bool scoreAdd(const u16 score)__z88dk_fastcall
{
    if (sysIsGameMode()) {
        _score = addSaturateU16(_score, score);
        if (gameIsIncLeft() && (_score != 0xffff)) {
            _scoreForLife += score; // ゲーム モードによっては残機は増えない. カンストしたら残機は増えない.
        }
        if (SCORE_BONUS_SHIP < _scoreForLife) {
            _scoreForLife -= SCORE_BONUS_SHIP;
            _scoreLeft++;
            sdPlaySe(SE_1UP);
            return true;
        }
    }
    return false;
}

bool scoreReflectHiScore()__z88dk_fastcall
{
    if (_scoreHiScore < _score) {
        _scoreHiScore = _score;
        return true;
    }
    return false;
}


// ---------------------------------------------------------------- 残機
bool scoreDecrementLeft()__z88dk_fastcall
{
    if (sysIsGameMode()) {
        _scoreNrMisses++;
        if (gameIsCaravan()) {  // キャラバン モードは何回死んでもいい
            return false;
        }
        if (_scoreLeft == 0) {
            return true;
        }
        if (gameGetMode() == GAME_MODE_HARD) {  // ハードモードではレベル 10% 引
            _scoreLevel -= _scoreLevel / 10;
            _scoreSubLevel = 0;
        }
        _scoreLeft --;
    }
    return false;
}

// ---------------------------------------------------------------- レベル
void scoreAddSubLevel(const u8 subLevel)__z88dk_fastcall
{
    if (_scoreLevel != 255) {    // カンスト
        _scoreSubLevel += subLevel;
        if (4 * 4 * 7 <= _scoreSubLevel) {
            _scoreSubLevel -= 4 * 4 * 7;
            _scoreLevel++;
            if (_scoreLevel == 255) {
                _scoreSubLevel = 0;
            }
            sdPlaySe(SE_LEVEL_UP);
        }
    }
}

// ---------------------------------------------------------------- デバッグ
