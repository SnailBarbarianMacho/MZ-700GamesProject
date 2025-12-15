/**
 * メイン ループ
 *
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../src-common/common.h"
#include "../../src-common/hard.h"
#include "deexo3.h"

// ゲーム本体(GAME_ORG～) を GAME_TMP 以降に転送します.
static void moveGame(void)
{
__asm
    // サイズ
    ld  HL, (3 + ADDR_GAME_ORG)                         // ADDR_GAME_ORG は Makefile で定義
    ld  DE, -5
    add HL, DE
    ld  BC, HL
    // 転送元
    ld  HL, 5 + ADDR_GAME_ORG
    // 転送先
    ld  DE, 0 + ADDR_GAME_TMP                           // ADDR_GAME_TMP は Makefile で定義
    ldir
__endasm;
}

static void go(void)
{
__asm
    // PCG-700 を無効化する
    BANKH_VRAM_MMIO     C
    PCG700_DISABLE      A
    BANKH_RAM           C

    // GO!
    jp  ADDR_GAME_ORG
__endasm;
}


void main(void) __naked
{
    moveGame();

    deexo3((const u8* const)ADDR_GAME_TMP, (u8* const)ADDR_GAME_ORG); // 展開

    go();
}
