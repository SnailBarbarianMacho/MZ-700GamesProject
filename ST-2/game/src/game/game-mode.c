/**
 * ゲーム モード
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/vvram.h"
#include "../system/sound.h"
#include "se.h"
#include "game-mode.h"

// ---------------------------------------------------------------- 変数
u8                  game_mode_;
bool                game_hard_;
bool                b_game_caravan_;
bool                b_game_inc_left_;
u16                 game_timer_;
u8                  game_subtimer_;
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

    game_hard_             = false;
    if (GAME_MODE_HARD <= game_mode && game_mode <= GAME_MODE_MUBO) {
        game_hard_         = 5; // アイテム 5個増し
    }

    game_timer_            = 0;
    game_subtimer_         = 0;
    b_game_caravan_        = false;
    if (game_mode == GAME_MODE_CARAVAN) {
        game_timer_        = CARAVAN_TIME * GAME_FPS;
        b_game_caravan_    = true;
    }

    b_game_inc_left_       = true;
    if (GAME_MODE_SURVIVAL <= gameGetMode()) {  // サバイバル, むぼう, キャラバン モードは残機増加なし
        b_game_inc_left_ = false;
    }

    p_game_caravan_chime_  = CHIME_TAB_;
    game_caravan_chime_ct_ = 100 * GAME_FPS;
}

// ---------------------------------------------------------------- Setter/Getter
u16 gameDecTimer(void)
{
    if (game_timer_) {
        game_timer_ --;
        game_caravan_chime_ct_--;
        if (game_caravan_chime_ct_ == 0) {
            game_caravan_chime_ct_ = *p_game_caravan_chime_++;
            sdPlaySe(SE_CHIME);
        }
    }
    return game_timer_;
}

void gameIncTimer(void)
{
    if (game_timer_ != 0xffff || game_subtimer_ != 31) {
        game_subtimer_ ++;
        if (game_subtimer_ == 32) {
            game_subtimer_ = 0;
            game_timer_ ++;
        }
    }
}