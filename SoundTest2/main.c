/**
 * 三重和音サウンド テスト
 *
 * @author Snail Barbarian Macho (NWK)
 */
#include "../src-common/common.h"
#include "sound.h"

void main() __naked
{
    soundInit();
    sdSetEnabled(true);

#define L 20
    static const u8 mml0[] = {
        SD3_C3, L, SD3_G3, L, SD3_C4, L, SD3_E4, L,
        SD3_G2, L, SD3_D3, L, SD3_G3, L, SD3_B3, L,
        SD3_A2, L, SD3_E3, L, SD3_A3, L, SD3_C4, L,
        SD3_E2, L, SD3_B2, L, SD3_E3, L, SD3_G3, L,
        SD3_F2, L, SD3_C3, L, SD3_F3, L, SD3_A3, L,
        SD3_C3, L, SD3_G3, L, SD3_C4, L, SD3_E4, L,
        SD3_F2, L, SD3_C3, L, SD3_F3, L, SD3_A3, L,
        SD3_G2, L, SD3_D3, L, SD3_G3, L, SD3_B3, L,
        SD3_C3, 128,
        0,
    };
    static const u8 mml1[] = {
        SD3_E4, L*4, SD3_D4, L*4, SD3_C4, L*4, SD3_B3, L*4,
        SD3_A3, L*4, SD3_G3, L*4, SD3_A3, L*4, SD3_B3, L*4,
        SD3_E4, 128,
        0,
    };
    static const u8 mml2[] = {
        SD3_C4, L*4, SD3_B3, L*4, SD3_A3, L*4, SD3_G3, L*4,
        SD3_F3, L*4, SD3_E3, L*4, SD3_F3, L*4, SD3_G3, L*4,
        SD3_C4, 128,
        0,
    };
    sd3Play(mml0, mml1, mml2);

    while (1);
}
