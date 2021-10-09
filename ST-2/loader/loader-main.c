/**
 * メイン ループ
 *
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../src-common/common.h"
#include "deexo3.h"

// ゲーム本体(GAME_ORG～) を GAME_TMP 以降に転送します.
static void moveGame()
{
__asm
    // サイズ
    ld  HL, (#ADDR_GAME_ORG + 3)// ADDR_GAME_ORG は Makefile で定義
    ld  DE, #-5
    add HL, DE
    ld  BC, HL
    // 転送元
    ld  HL, #(ADDR_GAME_ORG + 5)
    // 転送先
    ld  DE, #ADDR_GAME_TMP      // ADDR_GAME_TMP は Makefile で定義
    ldir
__endasm;
}

void main() __naked
{
    moveGame();                 // 転送

#if 0
    u8* p = (u8*)ADDR_GAME_ORG;
    for (u16 i = 0; i < 0x8000; i++) {
        *p++ = 0xff;
    }
    p = (u8*)ADDR_DEEXO_WORK;
    for (u16 i = 0; i < 0x100; i++) {
        *p++ = 0xff;
    }
#endif

    deexo3((const u8* const)ADDR_GAME_TMP, (u8* const)ADDR_GAME_ORG); // 展開

__asm
    jp  ADDR_GAME_ORG           // GO!
__endasm;
}
