/**
 * ゲーム モード
 * @author Snail Barbarian Macho (NWK)
 */

#include "../../../../src-common/common.h"
#include "../system/vram.h"
#include "../system/sound.h"
#include "se.h"
#include "gameMode.h"

// ---------------------------------------------------------------- 変数
u8                  _gameMode;
bool                _gameHard;
bool                _bGameCaravan;
bool                _bGameIncLeft;
u16                 _gameCaravanTimer;
static const u16*   _gameCaravanChimePtr;
static u16          _gameCaravanChimeCt;

static const u16 _gameCaravanChimeTab[] = {
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
void gameSetMode(const u8 gameMode) __z88dk_fastcall
{
    _gameMode            = gameMode;
    _gameHard            = (gameMode >= GAME_MODE_HARD) ? 1 : 0;
    _bGameCaravan        = gameMode == GAME_MODE_CARAVAN;
    _bGameIncLeft        = (_bGameCaravan || (gameMode == GAME_MODE_SURVIVAL)) ? false : true;
    _gameCaravanTimer    = CARAVAN_TIME * GAME_FPS;
    _gameCaravanChimePtr = _gameCaravanChimeTab;
    _gameCaravanChimeCt  = 100 * GAME_FPS;
}

// ---------------------------------------------------------------- Setter/Getter
u16 gameDecCaravanTimer()
{
    if (_gameCaravanTimer) {
        _gameCaravanTimer --;
        _gameCaravanChimeCt--;
        if (_gameCaravanChimeCt == 0) {
            _gameCaravanChimeCt = *_gameCaravanChimePtr++;
            sdPlaySe(SE_CHIME);
        }
    }
    return _gameCaravanTimer;
}
