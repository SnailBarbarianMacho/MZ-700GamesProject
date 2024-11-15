/**
 * BGM
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/obj.h"
#include "../system/sound.h"
#include "../system/math.h"
#include "../objworks/obj_player.h"  // OBJ_PLAYER_STEP_NORMAL
#include "../game/stage.h"
#include "bgm.h"


// ---------------------------------------------------------------- BGM
// BGM を鳴らす条件かを返します
static bool checkBgm(void)
{
    if (sdGetSePriority() < SD_SE_PRIORITY_2) { return false; } // 優先高い音が鳴ってる場合は鳴らさない
    Obj* p_obj = objGetInUsePlayer();
    return p_obj && p_obj->step == OBJ_PLAYER_STEP_NORMAL;        // 死亡時は鳴らさない
}


// -------------------------------- BGM1
static u8 bgm1_tab_[5];

void bgm1Init(void)
{
    for (s8 i = COUNT_OF(bgm1_tab_) - 1; 0 <= i; i--) {
        bgm1_tab_[i] = (rand8() & 0x1f) + 0x03;
    }
}


static u16 bgm1Main(u16 ct)
{
    if (checkBgm()) {
        u8 ct2 = ct;
        Obj* p_obj;
        for (p_obj = objGetUserdEnemy(); p_obj != nullptr && 0 < ct2; p_obj = p_obj->p_next, ct2--); // 音源の元となる Obj を探す
        if (p_obj) {
            sdMake((bgm1_tab_[ct] << 8) + p_obj->u_geo.geo8.yh * 64);// yh の値で, 0x0000～0x0640 加算されます
        } else {
            sdMake(0x0000);
        }
    }
    ct++;
    if (clampU8(stgGetNrEnemies(), 2, COUNT_OF(bgm1_tab_)) <= ct) { ct = 0; } // 敵が減ると周期を速める
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
static const u8 BGM4_TAB_[] = {
    SD1_C3, SD1_DS3,  SD1_C3, SD1_DS3,  SD1_C3, SD1_DS3,  SD1_CS3, SD1_E3,
    SD1_C3, SD1_DS3,  SD1_C3, SD1_DS3,  SD1_C3, SD1_DS3,  SD1_CS3, SD1_E3,
    SD1_D3, SD1_F3,   SD1_D3, SD1_F3,   SD1_D3, SD1_F3,   SD1_DS3, SD1_FS3,
    SD1_D3, SD1_F3,   SD1_D3, SD1_F3,   SD1_D3, SD1_F3,   SD1_CS3, SD1_E3,
};
static u16 bgm4Main(u16 ct)
{
#define LEN 32
    u16 pos = (ct / LEN) << 1;
    sd1Play(BGM4_TAB_[pos + ((ct >> 1) & 1)]);

    ct++;
    if ((u16)COUNT_OF(BGM4_TAB_) * LEN / 2 <= ct) { ct = 0; }
    return ct;
#undef LEN
}


// ---------------------------------------------------------------- テーブル
const BgmSequencerDesc bgm_sequencer_tab_[NR_BGMS] = {
    { nullptr,  nullptr,  },
    { bgm1Init, bgm1Main, },
    { nullptr,  bgm2Main, },
    { nullptr,  bgm3Main, },
    { nullptr,  bgm4Main, },
};
