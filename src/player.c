#include "player.h"

/**
 * Send key events to the game.
 */
static void sendTick(HWND hWnd, SkyMusicTick_t *tick) {
  for (int i = 0, j = 1; i < 15; i++, j <<= 1) {
    if (tick->keyDown & j)
      SendMessageW(hWnd, WM_KEYDOWN, KEYCODES[i], KEYS[i] << 16);
    if (tick->keyUp & j)
      SendMessageW(hWnd, WM_KEYUP, KEYCODES[i], (KEYS[i] << 16) | 1 | (0B110 << 29));
  }
}

/** 
 * Check what the player needs to do in the next real tick.
 */
int checkCanPlay(HWND hSkyGameWnd) {
  CURSORINFO ci = { sizeof(CURSORINFO) };
  RECT screenRect = {
    0, 0,
    GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)
  }, cr;

#ifdef DEBUG_NO_PLAY_STATE_CHECK
  return 0;
#endif
  
  if (!hSkyGameWnd || GetForegroundWindow() != hSkyGameWnd)
    // The player isn't initialized or the game window is
    // inactive.
    // Need to pause playing.
    return 1;

  if (GetClipCursor(&cr) && !EqualRect(&cr, &screenRect))
    // Mouse is captured, this only occurs when the game
    // window is dragged.
    // Need to pause playing.
    return 1;

  if (GetCursorInfo(&ci) && !(ci.flags & CURSOR_SHOWING))
    // Mouse is captured and hidden by the game.
    // Need to stop playing.
    return 2;

  // Can play
  return 0;
}

/**
 * Tick function of the player.
 * 
 * There's 2 types of tick in the player: real tick and inteval tick.
 * 
 * Real tick sends the KEYDOWN message, and inteval tick sends the KEYUP
 * message.
 * 
 * For every key, its key events be alternating between WM_KEYDOWN and WM_KEYUP,
 * and adjacent events must be separated by at least 10ms.
 * 
 * Therefore, the inteval tick sends the WM_KEYUP message, and it's intended to
 * keep the above limitations.
 */
static void CALLBACK snTick(
  UINT wTimerID,
  UINT msg,
  DWORD_PTR dwUser,
  DWORD dw1,
  DWORD dw2
) {
  SkyMusicPlayer_t *player = (SkyMusicPlayer_t *)dwUser;
  SkyMusicTick_t *lastTick;
  int ps = 0;

  if (player->checkState <= 0) {
    // Check state every 50ms.
    ps = checkCanPlay(player->hGameWnd);
    // Reset countdown.
    player->checkState = 1000 / player->inteval / 20;
    //printf("check - %d\n", player->tickCount);
    if (!ps) {
      player->savedTickCount = player->tickCount;
      player->savedTickIndex = player->tickIndex;
    }
  }
  player->checkState--;

  if (player->state != PLAYING)
    // Do nothing when isn't playing.
    return;
  if (player->state == STOPPED_PROG) {
    // Stopped by program, but timer isn't killed.
    if (player->timerId)
      timeKillEvent(player->timerId);
    player->timerId = 0;
    return;
  } if (ps == 2 || player->tickIndex >= player->maxIndex) {
    // Exit keyboard or EOF.
    // Need to kill timer and stop.
    timeKillEvent(player->timerId);
    player->timerId = 0;
    // Set state to STOPPED_*.
    player->state = player->tickIndex >= player->maxIndex ? STOPPED_EOF : STOPPED_ESC;
    return;
  } else if (ps == 1) {
    // Set state to PAUSED_*.
    player->state = PAUSED_BG;
    return;
  }

  // Next tick.
  player->tickCount++;
  if (player->tickCount < 0)
    // Pre-play state, do nothing.
    return;
  if (((i32)player->currentTick->tick) <= player->tickCount) {
    // Encountered a tick.
    lastTick = player->currentTick;
    player->tickIndex++;
    // Get next tick.
    vec_at(player->builtTicks, player->tickIndex, (void *)&player->currentTick);
    //printf("tick-%d %x %x\n", player->tickCount, lastTick->keyDown, lastTick->keyUp);
    // Send events to game window.
    sendTick(player->hGameWnd, lastTick);
  }
}

i32 snCreatePlayer(
  SkyMusicPlayer_t *player,
  SkyAutoPlayOptions_t *options,
  HWND hGameWnd,
  Vector_t *builtTicks
) {
  TIMECAPS tc;
  UINT res;
  if (!player || !hGameWnd || !builtTicks)
    return 0;
  // Get min and max system timer resolution.
  if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
    return 0;
  // Clamp target resolution.
  res = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
  // Initialize timer.
  player->timerRes = res;
  player->inteval = options->highTps ? 1 : 10;
  player->builtTicks = builtTicks;
  player->hGameWnd = hGameWnd;
  player->state = STOPPED_PROG;
  player->tickCount = 0;
  player->tickIndex = 0;
  vec_size(builtTicks, &player->maxIndex);
  vec_at(builtTicks, 0, (void **)&player->currentTick);
  player->timerId = 0;
  return 1;
}

i32 snMusicPlay(SkyMusicPlayer_t *player) {
  if (!player || player->state == PLAYING)
    return 0;
  if (checkCanPlay(player->hGameWnd))
    // Do nothing when the condition isn't met.
    return 0;

  if (player->state > 0) {
    // Paused player, the timer isn't killed.
    if (player->state == PAUSED_BG) {
      // Restore saved data.
      // Only when the event is automatically processed by snTick.
      player->tickCount = player->savedTickCount;
      player->tickIndex = player->savedTickIndex;
      vec_at(
        player->builtTicks,
        player->tickIndex,
        (void **)&player->currentTick
      );
    }
    // Set state.
    player->state = PLAYING;
  } else if (player->state < 0) {
    // Stopped player, the timer is killed.
    if (player->timerId)
      // Kill the timer may exists.
      timeKillEvent(player->timerId);
    player->state = PLAYING;

    // Reinitialize player.
    player->tickCount = 0;
    player->tickIndex = 0;
    vec_size(player->builtTicks, &player->maxIndex);
    vec_at(player->builtTicks, 0, (void **)&player->currentTick);
    player->timerId = timeSetEvent(
      player->inteval,
      player->timerRes,
      (LPTIMECALLBACK)snTick,
      (DWORD_PTR)player,
      TIME_PERIODIC | TIME_CALLBACK_FUNCTION | TIME_KILL_SYNCHRONOUS 
    );
  }

  return 1;
}

i32 snMusicResume(SkyMusicPlayer_t *player) {
  if (!player || player->state >= 0)
    return 0;
  if (checkCanPlay(player->hGameWnd))
    return 0;

  if (player->timerId)
    timeKillEvent(player->timerId);
  player->state = PLAYING;
  if (player->state == STOPPED_ESC) {
    // Restore saved data.
    // Only when the event is automatically processed by snTick.
    player->tickCount = player->savedTickCount;
    player->tickIndex = player->savedTickIndex;
    vec_at(
      player->builtTicks,
      player->tickIndex,
      (void **)&player->currentTick
    );
  }
  player->timerId = timeSetEvent(
    player->inteval,
    player->timerRes,
    (LPTIMECALLBACK)snTick,
    (DWORD_PTR)player,
    TIME_PERIODIC | TIME_CALLBACK_FUNCTION | TIME_KILL_SYNCHRONOUS 
  );

  return 1;
}

i32 snMusicPause(SkyMusicPlayer_t *player) {
  if (!player || player->state != PLAYING)
    return 0;
  player->state = PAUSED_PROG;
  return 1;
}

i32 snMusicStop(SkyMusicPlayer_t *player) {
  if (!player || player->state != PLAYING)
    return 0;
  // The timer callback will kill the timer automatically.
  player->state = STOPPED_PROG;
  if (player->timerId)
    timeKillEvent(player->timerId);
  player->timerId = 0;
  return 1;
}