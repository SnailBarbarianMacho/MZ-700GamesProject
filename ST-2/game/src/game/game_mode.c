/**
 * ゲーム モード
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/vram.h"
#include "../system/sound.h"
#include "se.h"
#include "game_mode.h"

// ---------------------------------------------------------------- 変数
u8                  game_mode_;
bool                game_hard_;
bool                b_game_caravan_;
bool                b_game_inc_left_;
u16                 game_caravan_timer_;
static const u16*   p_game_caravan_chime_;
static u16          game_caravan_chime_ct_;

static const u16 CHIME_TAB_[] = {
    1 * GAME_FPS,   // 残り200秒
    99 * GAME_FPS,
    1 * GAME_FPS,   // 残り100秒
    49 * GAME_FPS,
    1 * GAME_FPS,   // 残り50秒
    39 * GAME_FPS,
    1 * GAME_FPS,   // 残り10秒
    4 * GAME_FPS,
    1 * GAME_FPS,   // 残り5秒
    1 * GAME_FPS,
    1 * GAME_FPS,
    1 * GAME_FPS,
    65535,
};

// ---------------------------------------------------------------- マクロ
#if DEBUG
#define CARAVAN_TIME   300 // 秒
//#define CARAVAN_TIME   3// デバッグ用
#else
#define CARAVAN_TIME   300 // 秒
#endif

// ---------------------------------------------------------------- ゲーム モード
void gameSetMode(const u8 game_mode) __z88dk_fastcall
{
    game_mode_             = game_mode;
    game_hard_             = (game_mode >= GAME_MODE_HARD) ? 1 : 0;
    b_game_caravan_        = game_mode == GAME_MODE_CARAVAN;
    b_game_inc_left_       = (b_game_caravan_ || (game_mode == GAME_MODE_SURVIVAL)) ? false : true;
    game_caravan_timer_    = CARAVAN_TIME * GAME_FPS;
    p_game_caravan_chime_  = CHIME_TAB_;
    game_caravan_chime_ct_ = 100 * GAME_FPS;
}

// ---------------------------------------------------------------- Setter/Getter
u16 gameDecCaravanTimer(void)
{
    if (game_caravan_timer_) {
        game_caravan_timer_ --;
        game_caravan_chime_ct_--;
        if (game_caravan_chime_ct_ == 0) {
            game_caravan_chime_ct_ = *p_game_caravan_chime_++;
            sdPlaySe(SE_CHIME);
        }
    }
    return game_caravan_timer_;
}
