#include <windows.h>

#include "macros.h"
#include "vector.h"
#include "note.h"

//#define DEBUG_NO_PLAY_STATE_CHECK

typedef enum {
  /** Stopped by exit music keyboard. */
  STOPPED_ESC = -3,
  /** Stopped by end of file. */
  STOPPED_EOF = -2,
  /** Stopped by program. */
  STOPPED_PROG = -1,
  /** Playing. */
  PLAYING = 0,
  /** Paused by unfocus game window. */
  PAUSED_BG = 1,
  /** Paused by program. */
  PAUSED_PROG = 2
} SkyMusicPlayState_t;

typedef struct {
  /* Constant values */
  /** Current player state */
  SkyMusicPlayState_t state;
  u32 timerId;
  u32 timerRes;
  u32 inteval;
  HWND hGameWnd;
  Vector_t *builtTicks;
  SkyMusicTick_t *currentTick;
  u64 maxIndex;
  i64 tickIndex;
  i32 tickCount;
  i32 checkState;
  i32 savedTickCount;
  i64 savedTickIndex;
} SkyMusicPlayer_t;

i32 snCreatePlayer(
  SkyMusicPlayer_t *player,
  SkyAutoPlayOptions_t *options,
  HWND hGameWnd,
  Vector_t *builtTicks
);
i32 snMusicPlay(SkyMusicPlayer_t *player);
i32 snMusicResume(SkyMusicPlayer_t *player);
i32 snMusicPause(SkyMusicPlayer_t *player);
i32 snMusicStop(SkyMusicPlayer_t *player);