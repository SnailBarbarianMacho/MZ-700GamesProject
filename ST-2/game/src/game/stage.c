/**
 * ステージ管理
 * @author Snail Barbarian Macho (NWK)
 */

#include "../system/sys.h"
#include "../system/sound.h"
#include "../system/math.h"
#include "../objworks/obj_enemy.h"
#include "../objworks/obj_player.h"
#include "bgm.h"
#include "se.h"
#include "game_mode.h"
#include "stage.h"

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

static const u8 STG_TAB_[] = {
#if 1   // TEST STAGE 1 3_2 が後半に登場.
    ST_BGM1,
#if DEBUG // TEST いろいろ敵のテスト
    //E3_1(3), 0,
    //E3_1(1), 0, E3_2(1), 0, E3_3(1), 0, E3_4(1), 0, E3_5(1), 0, E3_6(1), 0,
    //E4_1(1), 0, E4_2(1), 0, E4_3(1), 0,
    //E5_1(1), 0, E5_2(1), 0, E5_3(1), 0,
    E8_1(1), 0, E8_2(1), 0, E8_3(1), 0,
    //ST_BGM2,
    //E3_6(4), 0,
    //E5_1(2), E5_2(2), E5_3(2), E4_1(1), E4_2(1), E4_3(1), E3_1(1), E3_2(1), E3_3(1), E3_4(1), E3_5(1), E3_6(1), 0,
    //ST_BGM3,
    //E4_2(1), 0,
    //E8_1(1), 0,
    ST_END,//TEST 即エンディング
#endif
    E3_1(3), 0,
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
    ST_END,// だいたい スコア 540000, レベル 144
};

static const u8 STG_MUBO_TAB_[] = {
    // いきなり最終面
    ST_BGM2,
    E8_1(3), 0,
    ST_BGM3,
    E8_2(2), 0,
    ST_BGM4,
    E8_3(1), 0,
    ST_END,// だいたい スコア 540000, レベル 144
};

static const u8 STG_CARAVAN_TAB_[] = {
    ST_BGM1,
    E3_1(15), 0, E3_1(15), 0,
    E3_2(15), 0, E3_2(15), 0,
    E3_3(15), 0, E3_3(15), 0,
    E3_4(15), 0, E3_4(15), 0,
    E3_5(15), 0, E3_5(15), 0,
    E3_6(15), 0, E3_6(15), 0,
    ST_BGM4,
    E8_1(5), 0,
    ST_BGM2,
    E4_1(15), 0, E4_1(15), 0,
    E4_2(15), 0, E4_2(15), 0,
    E4_3(15), 0, E4_3(15), 0,
    ST_BGM4,
    E8_2(5), 0,
    ST_BGM3,
    E5_1(15), 0, E5_1(15), 0,
    E5_2(15), 0, E5_2(15), 0,
    E5_3(15), 0, E5_3(15), 0,
    ST_BGM4,
    E8_3(5), 0,
    ST_END,
};


const u8* p_stg_tab_;        // ステージ テーブルへのポインタ
u8        stg_nr_;           // ステージ番号
u8        stg_nr_enemies_;   // 敵の残り

// ---------------------------------------------------------------- ステージ, サブステージ
void stgInit(u8 nr_skipped_stages) __z88dk_fastcall
{
    p_stg_tab_  = STG_TAB_;
    stg_nr_     = 1;
    if (gameGetMode() == GAME_MODE_MUBO   ) { p_stg_tab_ = STG_MUBO_TAB_;  stg_nr_ = 19; }
    if (gameGetMode() == GAME_MODE_CARAVAN) { p_stg_tab_ = STG_CARAVAN_TAB_; }
    // ステージを飛ばす
    for (u8 i = 0; i < nr_skipped_stages; ) {
        if (*p_stg_tab_++ == ST_CLEAR) { i++; }
    }
}


struct EnemyTab {
    void (*init_func)(Obj* const, Obj* const);
    bool (*main_func)(Obj* const);
    void (*draw_func)(Obj* const, u8*);
} static const ENEMY_TAB_[] = {
    { objEnemyInit3_1, objEnemyMain3_1, objEnemyDraw3_1, },
    { objEnemyInit3_2, objEnemyMain3_2, objEnemyDraw3_2, },
    { objEnemyInit3_3, objEnemyMain3_3, objEnemyDraw3_3, },
    { objEnemyInit3_4, objEnemyMain3_4, objEnemyDraw3_4, },
    { objEnemyInit3_5, objEnemyMain3_5, objEnemyDraw3_5, },
    { objEnemyInit3_6, objEnemyMain3_6, objEnemyDraw3_6, },
    { objEnemyInit4_1, objEnemyMain4_1, objEnemyDraw4_1, },
    { objEnemyInit4_2, objEnemyMain4_2, objEnemyDraw4_2, },
    { objEnemyInit4_3, objEnemyMain4_3, objEnemyDraw4_3, },
    { objEnemyInit5_1, objEnemyMain5_1, objEnemyDraw5_1, },
    { objEnemyInit5_2, objEnemyMain5_2, objEnemyDraw5_2, },
    { objEnemyInit5_3, objEnemyMain5_3, objEnemyDraw5_3, },
    { objEnemyInit8_1, objEnemyMain8_1, objEnemyDraw8_1, },
    { objEnemyInit8_2, objEnemyMain8_2, objEnemyDraw8_2, },
    { objEnemyInit8_3, objEnemyMain8_3, objEnemyDraw8_3, },
};


u8 stgSubInit(void)
{
    stg_nr_enemies_ = 0;
    while (true) {
        u8 p = *p_stg_tab_++;
        if (p == 0x00) {
            break;
        }
        u8 n = p >> 4;
        switch (p & 0x0f) {
        case ST_CLEAR:
            switch (n) {
            case ST_CLEAR >> 4:
                stg_nr_++;
                return STG_STATUS_CLEAR;
            case ST_END >> 4:
                return STG_STATUS_ENDING;
            case ST_BGM1 >> 4:
                sdPlayBgm(BGM_1);
                break;
            case ST_BGM2 >> 4:
                sdPlayBgm(BGM_2);
                break;
            case ST_BGM3 >> 4:
                sdPlayBgm(BGM_3);
                break;
            case ST_BGM4 >> 4:
                sdPlayBgm(BGM_4);
                break;
            }
            break;
        default:    // 敵を発生
            {
                const struct EnemyTab* const p_e = &ENEMY_TAB_[p & 0x0f];
                for (; n > 0; n--) {
                    if (!objCreateEnemy(p_e->init_func, p_e->main_func, p_e->draw_func, nullptr)) { break; }
                    stg_nr_enemies_++;
                }
            }
            break;
        } // switch()
    } // while (true)
    sdPlaySe(SE_SUB_STAGE_START);
    return STG_STATUS_OK;
}

// ---------------------------------------------------------------- 敵数
