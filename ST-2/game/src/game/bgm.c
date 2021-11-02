/**
 * BGM
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/obj.h"
#include "../system/sound.h"
#include "../system/math.h"
#include "../objworks/objPlayer.h"  // OBJ_PLAYER_STEP_NORMAL
#include "../game/stage.h"
#include "bgm.h"


// ---------------------------------------------------------------- BGM
// BGM を鳴らす条件かを返します
static bool checkBgm()
{
    if (sdGetSePriority() < SD_SE_PRIORITY_2) { return false; } // 優先高い音が鳴ってる場合は鳴らさない
    Obj* pObj = objGetInUsePlayer();
    return pObj && pObj->step == OBJ_PLAYER_STEP_NORMAL;        // 死亡時は鳴らさない
}

// -------------------------------- BGM1
static u8 _bgm1Tab[5];

void bgm1Init()
{
    for (s8 i = COUNT_OF(_bgm1Tab) - 1; 0 <= i; i--) {
        _bgm1Tab[i] = (rand8() & 0x1f) + 0x03;
    }
}
static u16 bgm1Main(u16 ct)
{
    if (checkBgm()) {
        u8 ct2 = ct;
        Obj* pObj;
        for (pObj = objGetUserdEnemy(); pObj != nullptr && 0 < ct2; pObj = pObj->pNext, ct2--); // 音源の元となる Obj を探す
        if (pObj) {
            sdMake((_bgm1Tab[ct] << 8) + pObj->uGeo.geo8.yh * 64);// yh の値で, 0x0000～0x0640 加算されます
        } else {
            sdMake(0x0000);
        }
    }
    ct++;
    if (clampU8(stgGetNrEnemies(), 2, COUNT_OF(_bgm1Tab)) <= ct) { ct = 0; } // 敵が減ると周期を速める
    return ct;
}

// -------------------------------- BGM2
static u16 bgm2Main(u16 ct)
{
    if (ct < 0x1000) { ct = stgGetNrEnemies() * 0x100 + 0x1000; }
    if (checkBgm()) {
        sdMake(ct);
    }
    ct -= 0x0040;
    return ct;
}

// -------------------------------- BGM3
static u16 bgm3Main(u16 ct)
{
    if (checkBgm()) {
        sdMake(ct);
    }
    ct += 0x0200;
    if (stgGetNrEnemies() * 0x800 + 0x0c00 < ct) { ct = 0x0c00; }
    return ct;
}

// -------------------------------- BGM4
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

// ---------------------------------------------------------------- テーブル
const BgmSequencerDesc _bgmSequencerTab[NR_BGMS] = {
    { nullptr,  nullptr,  },
    { bgm1Init, bgm1Main, },
    { nullptr,  bgm2Main, },
    { nullptr,  bgm3Main, },
    { nullptr,  bgm4Main, },
};
