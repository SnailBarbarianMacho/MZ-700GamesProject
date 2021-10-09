/**
 * ステージ管理
 * @author Snail Barbarian Macho (NWK)
 */

#include "../system/sys.h"
#include "../system/sound.h"
#include "../system/math.h"
#include "../objworks/objEnemy.h"
#include "../objworks/objPlayer.h"
#include "stage.h"

// ---------------------------------------------------------------- サウンド
#define SE_SUB_STAGE_START_CT  64
// サブステージ開始音
static void seSubStageStart(u8 ct)
{
    sdMake(((ct & 3) + 1) << 9);
}

// ---------------------------------------------------------------- サウンド
static u8 bgmTab[5];
// BGM を鳴らす条件かを返します
static inline bool checkBgm()
{
    if (sdGetSePriority() < SD_SE_PRIORITY_2) { return false; } // 優先高い音が鳴ってる場合は鳴らさない
    Obj* pObj = objGetInUsePlayer();
    return pObj && pObj->step == OBJ_PLAYER_STEP_NORMAL;        // 死亡時は鳴らさない
}

static void bgm1Init()
{
    for (s8 i = COUNT_OF(bgmTab) - 1; 0 <= i; i--) {
        bgmTab[i] = (rand8() & 0x1f) + 0x03;
    }
}
static u16 bgm1Main(u16 ct)
{
    if (checkBgm()) {
        u8 ct2 = ct;
        Obj* pObj;
        for (pObj = objGetUserdEnemy(); pObj != nullptr && 0 < ct2; pObj = pObj->pNext, ct2--); // 音源の元となる Obj を探す
        if (pObj) {
            sdMake((bgmTab[ct] << 8) + pObj->uGeo.geo8.yh * 64);// yh の値で, 0x0000～0x0640 加算されます
        } else {
            sdMake(0x0000);
        }
    }
    ct++;
    if (clampU8(stgGetNrEnemies(), 2, COUNT_OF(bgmTab)) <= ct) { ct = 0; } // 敵が減ると周期を速める
    return ct;
}

static u16 bgm2Main(u16 ct)
{
    if (ct < 0x1000) { ct = stgGetNrEnemies() * 0x100 + 0x1000; }
    if (checkBgm()) {
        sdMake(ct);
    }
    ct -= 0x0040;
    return ct;
}

static u16 bgm3Main(u16 ct)
{
    if (checkBgm()) {
        sdMake(ct);
    }
    ct += 0x0200;
    if (stgGetNrEnemies() * 0x800 + 0x0c00 < ct) { ct = 0x0c00; }
    return ct;
}

static u16 bgm4Main(u16 ct)
{
#define LEN 32
    static const u8 tab[] = {
        SD1_C3, SD1_DS3,  SD1_C3, SD1_DS3,  SD1_C3, SD1_DS3,  SD1_CS3, SD1_E3,
        SD1_C3, SD1_DS3,  SD1_C3, SD1_DS3,  SD1_C3, SD1_DS3,  SD1_CS3, SD1_E3,
        SD1_D3, SD1_F3,   SD1_D3, SD1_F3,   SD1_D3, SD1_F3,   SD1_DS3, SD1_FS3,
        SD1_D3, SD1_F3,   SD1_D3, SD1_F3,   SD1_D3, SD1_F3,   SD1_CS3, SD1_E3,
    };
    u16 pos = (ct / LEN) << 1;
    sd1Play(tab[pos + ((ct >> 1) & 1)]);

    ct++;
    if ((u16)COUNT_OF(tab) * LEN / 2 <= ct) { ct = 0; }
    return ct;
#undef LEN
}

// ---------------------------------------------------------------- ステージ テーブル
#define E3_1(n) (0x00 | ((n) * 16))
#define E3_2(n) (0x01 | ((n) * 16))
#define E3_3(n) (0x02 | ((n) * 16))
#define E3_4(n) (0x03 | ((n) * 16))
#define E3_5(n) (0x04 | ((n) * 16))
#define E3_6(n) (0x05 | ((n) * 16))
#define E4_1(n) (0x06 | ((n) * 16))
#define E4_2(n) (0x07 | ((n) * 16))
#define E4_3(n) (0x08 | ((n) * 16))
#define E5_1(n) (0x09 | ((n) * 16))
#define E5_2(n) (0x0a | ((n) * 16))
#define E5_3(n) (0x0b | ((n) * 16))
#define E8_1(n) (0x0c | ((n) * 16))
#define E8_2(n) (0x0d | ((n) * 16))
#define E8_3(n) (0x0e | ((n) * 16))
#define ST_CLEAR        0x0f
#define ST_BGM1         0x1f
#define ST_BGM2         0x2f
#define ST_BGM3         0x3f
#define ST_BGM4         0x4f
#define ST_END          0x5f

static const u8 sStageTab[] = {
#if 1
#if 1   // STAGE 1 3_2 が後半に登場.
    ST_BGM1,
    E3_1(3), 0,
    //ST_END,//TEST 即エンディング
    E3_1(4), 0,
    E3_1(5), 0,
    E3_1(6), 0,

    E3_1(6), 0,
    E3_2(1), 0,
    E3_1(6), 0,
    E3_1(7), 0,
    ST_CLEAR,
#endif
#if 1   // STAGE 2 3_1, 3_2 混載. スコア 5000, レベル 1
    ST_BGM1,
    E3_1(7), 0,
    E3_1(5), E3_2(1), 0,
    E3_1(6), E3_2(1), 0,
    E3_1(7), E3_2(1), 0,

    E3_1(4), E3_2(2), 0,
    E3_1(5), E3_2(2), 0,
    E3_1(4), E3_2(3), 0,
    E3_1(4), E3_2(3), 0,
    ST_CLEAR,
#endif
#if 1   // STAGE 3 3_3 初登場. スコア 13000, レベル 2
    ST_BGM1,
    E3_2(4), 0,
    E3_2(5), 0,
    E3_2(6), 0,
    E3_2(4), E3_3(1), 0,

    E3_2(5), E3_3(2), 0,
    E3_2(5), E3_3(3), 0,
    E3_2(5), E3_3(4), 0,
    E3_2(4), E3_3(5), 0,
    ST_CLEAR,
#endif
#if 1   // STAGE 4 3_4, 4_1 初登場. スコア 25000, レベル 5
    ST_BGM1,
    E3_4(1), 0,
    E3_4(2), 0,
    E3_4(3), 0,
    E3_2(3), E3_3(6), 0,

    E3_2(1), E3_3(7), 0,
    E3_2(1), E3_3(7), E3_4(1), 0,
    E3_3(8), 0,
    ST_BGM2,
    E4_1(1), 0,
    ST_CLEAR,
#endif
#if 1   // STAGE 5 3_5 初登場. スコア 36000, レベル 8
    ST_BGM1,
    E3_4(3), 0,
    E3_4(4), 0,
    E3_4(3), E3_3(2), 0,
    E3_4(3), E3_3(5), 0,

    E3_5(2), 0,
    E3_5(2), 0,
    E3_5(3), 0,
    E3_5(4), E3_4(1), 0,
    ST_CLEAR,
#endif
#if 1   // STAGE 6 3_6, 4_1 初登場. スコア 48000, レベル 10
    ST_BGM1,
    E3_5(3), 0,
    E3_6(4), 0,
    E3_6(4), E3_1(2), 0,
    E3_6(4), E3_2(2), 0,

    E3_6(4), E3_3(2), 0,
    E3_6(4), E3_4(2), 0,
    E3_6(4), E3_5(2), 0,
    ST_BGM2,
    E4_1(1), 0,   // 2回目の 4_1. 少しは倒しやすくなったかな?
    ST_CLEAR,
#endif
#if 1   // STAGE 7 ボス1 スコア 65000 レベル 12
    ST_BGM4,
    E8_1(1), 0,
    ST_CLEAR,
#endif
#if 1   // STAGE 8 3シリーズ総集編. スコア 70000, レベル 14
    ST_BGM1,
    E3_1(4), E3_2(4), E3_3(4), E3_4(3), 0,
    E3_2(4), E3_3(4), E3_4(4), E3_5(3), 0,
    E3_3(4), E3_4(4), E3_5(4), E3_6(3), 0,
    E3_1(4), E3_3(4), E3_4(4), E3_6(3), 0,

    E3_2(4), E3_4(4), E3_5(4), E3_6(3), 0,
    E3_1(10), E3_2(1), E3_3(1), E3_4(1), E3_5(1), E3_6(1), 0,
    E3_1(1), E3_2(10), E3_3(1), E3_4(1), E3_5(1), E3_6(1), 0,
    E3_1(2), E3_2(2), E3_3(2), E3_4(3), E3_5(3), E3_6(3), 0,
    ST_CLEAR,
#endif
#if 1   // STAGE 9 4_2 初登場. スコア 100000, レベル 21
    ST_BGM2,
    E4_1(3), 0,
    E4_1(3), E3_2(3), 0,
    E4_1(4), E3_3(3), 0,
    ST_BGM1,
    E4_1(5), 0,

    E4_2(1), 0,
    E4_2(1), E4_1(1), 0,
    ST_BGM2,
    E4_2(2), 0,
    E4_2(2), E4_1(2), 0,
    ST_CLEAR,
#endif
#if 1   // STAGE 10 4 シリーズのみ. 4_3 初登場. スコア 120000, レベル 25
    ST_BGM1,
    E4_1(6), 0,
    E4_1(7), 0,
    E4_1(8), 0,
    E4_2(3), 0,

    ST_BGM2,
    E4_2(3), E4_1(2), 0,
    E4_3(1), 0,
    E4_3(2), 0,
    E4_3(3), 0,
    ST_CLEAR,
#endif
#if 1   // STAGE 11 3～4 混在. スコア 150000, レベル 29
    ST_BGM1,
    E3_1(2), E3_2(2), E3_3(2), E3_4(2), E3_5(2), E4_1(2), 0,
    E3_1(2), E3_2(2), E3_3(2), E3_4(2), E3_5(2), E4_1(2), 0,
    E3_1(2), E3_2(2), E3_3(1), E3_4(1), E3_5(1), E3_6(2), E4_1(2), 0,
    E3_1(2), E3_2(2), E3_3(3), E3_4(2), E3_4(2), E3_6(2), E4_1(1), E4_2(1), 0,

    E3_1(1), E3_2(2), E3_3(1), E3_4(2), E3_5(1), E3_6(1), E4_1(1), E4_3(1), 0,
    E3_1(2), E3_2(2), E3_3(2), E3_4(2), E3_5(2), E3_6(1), E4_1(1), E4_2(1), 0,
    E3_1(2), E3_2(2), E3_3(2), E3_4(2), E3_4(2), E3_6(2), E4_1(2), E4_2(1), 0,
    ST_BGM2,
    E3_1(2), E3_2(2), E3_3(2), E3_4(2), E3_4(1), E3_6(1), E4_1(2), E4_2(1), E4_3(2), 0,
    ST_CLEAR,
#endif
#if 1 // STAGE 12 5_1 初登場. スコア 190000, レベル 37
    ST_BGM3,
    E5_1(1), 0,
    E5_1(2), 0,
    E5_1(3), 0,
    E4_1(5), E5_1(2), 0,

    ST_BGM2,
    E4_2(2), E5_1(2), 0,
    E4_3(4), E5_1(4), 0,
    E5_1(5), 0,
    E5_1(6), E3_6(4), 0,
    ST_CLEAR,
#endif
#if 1   // STAGE 13 ボス2 スコア 220000 レベル 48
    ST_BGM4,
    E8_2(1), 0,
    ST_CLEAR,
#endif
#if 1 // STAGE 14 5_1 複合攻撃. スコア 230000, レベル 50
    ST_BGM3,
    E5_1(2), E3_1(4), E3_4(4), E4_1(4), 0,
    E5_1(2), E3_2(4), E3_5(4), E4_2(1), 0,
    E5_1(2), E3_3(4), E3_6(4), E4_3(4), 0,
    E5_1(2), E4_1(4), E4_2(1), E4_3(4), 0,

    ST_BGM1,
    E5_1(3), E4_1(6), E3_1(6), 0,
    E5_1(3), E3_1(6), E3_2(6), 0,
    ST_BGM2,
    E5_1(4), E3_4(5), E3_6(4), E4_3(2), 0,
    E5_1(4), E3_5(4), E3_6(2), E4_2(1), E4_3(3), 0,
    ST_CLEAR,
#endif
#if 1 // STAGE 15 5_2 初登場. スコア 290000, レベル 65
    ST_BGM1,
    E5_2(2), 0,
    E5_2(3), 0,
    E5_2(4), 0,
    E5_2(4), E4_1(2), 0,

    ST_BGM2,
    E5_2(2), E4_2(2), 0,
    E5_2(4), E4_3(4), 0,
    ST_BGM3,
    E5_2(5), 0,
    E5_2(6), 0,
    ST_CLEAR,
#endif
#if 1 // STAGE 16 5_2 複合攻撃. スコア 330000, レベル 77
    ST_BGM1,
    E5_2(2), E4_2(1), E3_3(12), 0,
    ST_BGM2,
    E5_2(2), E3_2(4), E3_4(4), E4_1(4), 0,
    E5_2(2), E4_2(2), E3_3(11), 0,
    E5_2(2), E4_1(4), E4_2(2), E4_3(4), 0,

    E5_2(3), E4_2(3), E3_3(9), 0,
    E5_2(3), E4_3(3), E3_4(3), E3_5(3), E3_4(6),
    E5_2(4), E4_3(5), E3_6(4), E4_3(1), 0,
    E5_2(4), E4_1(4), E4_2(2), E4_3(2), E3_1(2), 0,
    ST_CLEAR,
#endif
#if 1 // STAGE 17 5_3 初登場. スコア 390000, レベル 94
    ST_BGM3,
    E5_3(1), 0,
    E5_3(2), 0,
    E5_3(3), 0,
    E5_3(4), 0,

    E5_3(3), E4_3(3), E3_4(7), 0,
    E5_3(3), E4_3(3), E3_4(2), E3_5(4), 0,
    E5_3(3), E4_3(10), 0,
    E5_3(3), E4_3(3), E3_4(2), E3_5(2), E3_6(2), 0,
    ST_CLEAR,
#endif
#if 1 // STAGE 18 総攻撃. スコア 440000, レベル 112
    ST_BGM1,
    E5_1(2), E5_2(2), E5_3(2), E4_1(1), E4_2(1), E4_3(1), E3_1(1), E3_2(1), E3_3(1), E3_4(1), E3_5(1), E3_6(1), 0,
    ST_BGM2,
    E5_1(3), E4_1(5), E3_1(7), 0,// 緑一色
    E5_2(3), E4_2(5), E3_3(7), 0,// 黄一色
    E5_3(3), E4_3(5), E3_4(7), 0,// 青一色

    ST_BGM1,
    E3_1(15), 0,
    ST_BGM2,
    E5_3(1), E4_2(14), 0,
    ST_BGM1,
    E3_3(15), 0,
    ST_BGM3,
    E5_1(2), E5_2(2), E5_3(2), E4_1(1), E4_2(1), E4_3(1), E3_1(1), E3_2(1), E3_3(1), E3_4(1), E3_5(1), E3_6(1), 0,
    ST_CLEAR,
#endif
#if 1 // STAGE 19 ボス4 スコア 510000, レベル 133
    ST_BGM2,
    E8_1(3), 0,
    ST_BGM3,
    E8_2(2), 0,
    ST_BGM4,
    E8_3(1), 0,
#endif
#endif
    ST_END,// だいたい スコア 540000, レベル 144
};
static const u8* spStgTab;          // ステージ テーブルへのポインタ
static       u8  sStgNr;            // ステージ番号
static       u8  sNrEnemies;        // 敵の残り

// ---------------------------------------------------------------- ステージ, サブステージ
void stgInit(u8 skipStage)__z88dk_fastcall
{
    spStgTab   = sStageTab;
    sStgNr     = 1;
    // ステージを飛ばす
    for (u8 i = 0; i < skipStage; ) {
        if (*spStgTab++ == ST_CLEAR) { i++; }
    }
}

u8 stgSubInit()
{
    sNrEnemies = 0;
    while (true) {
        u8 p = *spStgTab++;
        if (p == 0x00) {
            break;
        }
        u8 n = p >> 4;
        switch (p & 0x0f) {
        case ST_CLEAR:
            switch (n) {
            case ST_CLEAR >> 4:
                sStgNr++;
                return STG_STATUS_CLEAR;
            case ST_END >> 4:
                return STG_STATUS_ENDING;
            case ST_BGM1 >> 4:
                sdSetBgmSequencer(bgm1Init, bgm1Main);
                break;
            case ST_BGM2 >> 4:
                sdSetBgmSequencer(nullptr, bgm2Main);
                break;
            case ST_BGM3 >> 4:
                sdSetBgmSequencer(nullptr, bgm3Main);
                break;
            case ST_BGM4 >> 4:
                sdSetBgmSequencer(nullptr, bgm4Main);
                break;
            }
            break;
        default:
            {
                struct s_Tab {
                    void (*initFunc)(Obj* const, Obj* const);
                    bool (*mainFunc)(Obj* const);
                    void (*dispFunc)(Obj* const, u8* dispAddr);
                } static const tab[] = {
                    { objEnemyInit3_1, objEnemyMain3_1, objEnemyDisp3_1, },
                    { objEnemyInit3_2, objEnemyMain3_2, objEnemyDisp3_2, },
                    { objEnemyInit3_3, objEnemyMain3_3, objEnemyDisp3_3, },
                    { objEnemyInit3_4, objEnemyMain3_4, objEnemyDisp3_4, },
                    { objEnemyInit3_5, objEnemyMain3_5, objEnemyDisp3_5, },
                    { objEnemyInit3_6, objEnemyMain3_6, objEnemyDisp3_6, },
                    { objEnemyInit4_1, objEnemyMain4_1, objEnemyDisp4_1, },
                    { objEnemyInit4_2, objEnemyMain4_2, objEnemyDisp4_2, },
                    { objEnemyInit4_3, objEnemyMain4_3, objEnemyDisp4_3, },
                    { objEnemyInit5_1, objEnemyMain5_1, objEnemyDisp5_1, },
                    { objEnemyInit5_2, objEnemyMain5_2, objEnemyDisp5_2, },
                    { objEnemyInit5_3, objEnemyMain5_3, objEnemyDisp5_3, },
                    { objEnemyInit8_1, objEnemyMain8_1, objEnemyDisp8_1, },
                    { objEnemyInit8_2, objEnemyMain8_2, objEnemyDisp8_2, },
                    { objEnemyInit8_3, objEnemyMain8_3, objEnemyDisp8_3, },
                };
                const struct s_Tab* const pTab = &tab[p & 0x0f];
                for (; n > 0; n--) {
                    if (!objCreateEnemy(pTab->initFunc, pTab->mainFunc, pTab->dispFunc, nullptr)) { break; }
                    sNrEnemies++;
                }
            }
            break;
        } // switch()
    } // while (true)
    sdSetSeSequencer(seSubStageStart, SD_SE_PRIORITY_1, SE_SUB_STAGE_START_CT);
    return STG_STATUS_OK;
}

u8 stgGetStageNr()
{
    return sStgNr;
}

// ---------------------------------------------------------------- 敵数
u8 stgDecrementEnemies()
{
    sNrEnemies--;
    return sNrEnemies;
}

/** 現在の敵数を返します. 敵の数が減ったら難易度をあげるなどの処理に使います */
u8 stgGetNrEnemies()
{
    return sNrEnemies;
}
