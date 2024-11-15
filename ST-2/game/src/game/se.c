/**
 * SE
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/sound.h"
#include "../system/math.h"
#include "se.h"

// ---------------------------------------------------------------- SE
// -------------------------------- スタート
#define SE_START_CT           48
#define SE_SUB_STAGE_START_CT 64

static void seStart(u8 ct)          // スタート
{
#pragma disable_warning 110 // 除算最適化警告
#pragma save
    sdMake(((ct & 3) + (ct / 4) + 1) << 9);
#pragma restore
}
static void seSubStageStart(u8 ct)  // サブステージ開始
{
    sdMake(((ct & 3) + 1) << 9);
}

// -------------------------------- 1UP, レベルアップ
#define SE_LEVEL_UP_CT  48
#define SE_1UP_CT       128

static void seLevelUp(u8 ct)        // レベルアップ
{
#pragma disable_warning 110 // 除算最適化警告
#pragma save
    sdMake(((ct & 7) + (ct / 8) + 1) << 8);
#pragma restore
}
static void se1Up(u8 ct)            // 1UP
{
    sdMake(((ct >> 3) & 0x1) << 8);
}

// -------------------------------- 敵
#define SE_ENEMY_DEAD3_CT     20
#define SE_ENEMY_DEAD45_CT    64
#define SE_ENEMY_DEAD8_CT     128
#define SE_ENEMY_DAMAGE_CT    3

static const u8 ENEMY_DEAD_TAB_[SE_ENEMY_DEAD3_CT] = {
    0x15, 0x1d, 0x1a, 0x1b, 0x1a, 0x13, 0x12, 0x12, 0x14, 0x13, 0x11, 0x12, 0x11, 0x0f, 0x0e, 0x0d, 0x07, 0x0b, 0x08, 0x05,
};
static void seEnemyDead3(u8 ct)// 雑魚死亡
{
    sdMake(ENEMY_DEAD_TAB_[ct] << 10);
}

static void seEnemyDead45(u8 ct)// 中ボス死亡
{
    sdMake((20 - (ct & 15)) << 10);
}

static void seEnemyDead8(u8 ct)// 大ボス死亡
{
    sdMake((22 - (ct & 7)) << 10);
}

static void seEnemyDamage(u8 ct)// 敵ダメージ
{
    sdMake((SE_ENEMY_DAMAGE_CT - ct + 1) << 7);
}

// -------------------------------- アイテム
#define SE_GET_ITEM_CT     5

static void seGetItem(u8 ct)// アイテム取得
{
    sdMake((ct + 1) << 8);
}

// -------------------------------- プレーヤー
#define SE_PLAYER_DEAD_CT 48
#define SE_CONTINUE_CT    10

static void sePlayerDead(u8 ct)// ミス
{
    sdMake(((10 - (ct & 7)) << 10) + rand8());
}
static void seContinue(u8 ct)// コンティニュー
{
    sdMake((ct & 1) ? 0x0000 : 0x0200);
}

// -------------------------------- エンディング
#define SE_END_CT           128
#define SE_END_GAME_OVER_CT  70

static void seEnd(u8 ct)
{
#pragma disable_warning 110 // 除算最適化警告
#pragma save
    sdMake(((ct & 31) + (ct / 32) + 1) << 9);
#pragma restore
}

#pragma disable_warning 85
#pragma save
static void seEndGameOver(u8 ct)
{
    sdMake(rand8() << 7);
}
#pragma restore

// -------------------------------- キャラバン
#define SE_CHIME_CT     20

static void seChime(u8 ct)
{
    sdMake((ct < SE_CHIME_CT * 3 / 4) ? (u16)SD_MAKE_INTERVAL(SD_FREQ_C6) : (u16)SD_MAKE_INTERVAL(SD_FREQ_E6));
}


// ---------------------------------------------------------------- テーブル
const SeSequencerDesc SE_SEQUENCE_TAB_[NR_SES] = {
    { seStart,          SD_SE_PRIORITY_2, SE_START_CT           },
    { seSubStageStart,  SD_SE_PRIORITY_1, SE_SUB_STAGE_START_CT },
    { seLevelUp,        SD_SE_PRIORITY_0, SE_LEVEL_UP_CT        },
    { se1Up,            SD_SE_PRIORITY_0, SE_1UP_CT             },
    { seEnemyDead3,     SD_SE_PRIORITY_1, SE_ENEMY_DEAD3_CT     },
    { seEnemyDead45,    SD_SE_PRIORITY_1, SE_ENEMY_DEAD45_CT    },
    { seEnemyDead8,     SD_SE_PRIORITY_1, SE_ENEMY_DEAD8_CT     },
    { seEnemyDamage,    SD_SE_PRIORITY_3, SE_ENEMY_DAMAGE_CT    },
    { seGetItem,        SD_SE_PRIORITY_2, SE_GET_ITEM_CT        },
    { sePlayerDead,     SD_SE_PRIORITY_1, SE_PLAYER_DEAD_CT     },
    { seContinue,       SD_SE_PRIORITY_1, SE_CONTINUE_CT        },
    { seEnd,            SD_SE_PRIORITY_0, SE_END_CT             },
    { seEndGameOver,    SD_SE_PRIORITY_0, SE_END_GAME_OVER_CT   },
    { seChime,          SD_SE_PRIORITY_0, SE_CHIME_CT           },
};
