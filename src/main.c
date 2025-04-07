#include "main.h"

HWND skyGameWnd;
HANDLE hFinished;
NBS nbs;
Vector_t builtTicks = {0};
i64 tickIndex;
u64 maxIndex;
i8 isPlaying = 0
  , stopState = 0;
i32 tickCount = -1;
f32 tempo;
SkyMusicTick_t *currentTick = NULL;

/** 
 * Check what the player needs to do in the next real tick.
 */
int checkCanPlay() {
  CURSORINFO ci = { sizeof(CURSORINFO) };
  RECT screenRect = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) }
    , cr;
  
  if (!skyGameWnd || GetForegroundWindow() != skyGameWnd)
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
 * Real tick sends the KEYDOWN message, and inteval tick sends the
 * KEYUP message.
 * 
 * For every key, its key events be alternating between
 * WM_KEYDOWN and WM_KEYUP, and adjacent events must be separated by at
 * least 10ms.
 * 
 * Therefore, the inteval tick sends the WM_KEYUP message, and it's
 * intended to keep the above limitations.
 */
void CALLBACK tick(UINT wTimerID, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2) {
  SkyMusicTick_t *lastTick;
  float _t;
  int ps = checkCanPlay();

  if (ps == 2 || tickIndex >= maxIndex) {
    SetEvent(hFinished);
    stopState = tickIndex >= maxIndex ? 0 : 1;
    return;
  } else if (ps == 1)
    return;

  tickCount++;
  if (((i32)currentTick->tick) <= tickCount) {
    lastTick = currentTick;
    tickIndex++;
    vec_at(&builtTicks, tickIndex, (void *)&currentTick);
    printf("%x %x %x t%d %d aaa\n", lastTick, lastTick->keyDown, lastTick->keyUp, lastTick->tick, tickCount);
    sendTick(skyGameWnd, lastTick);
  }
}

int startTick(UINT *id) {
  TIMECAPS tc;
  UINT res;
  if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
    return 0;
  res = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
  *id = timeSetEvent(10, 1, (LPTIMECALLBACK)tick, res, TIME_PERIODIC | TIME_CALLBACK_FUNCTION | TIME_KILL_SYNCHRONOUS );
  return 1;
}

void stopTick(UINT *id) {
  timeKillEvent(*id);
}

size_t getFileSize(FILE* file) {
  fseek(file, 0, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);
  return fileSize;
}

int main() {
  UINT id = 0;

  skyGameWnd = FindWindowW(NULL, L"光·遇");
  if (!skyGameWnd) {
    MessageBoxW(NULL, L"游戏未运行", L"Error", MB_ICONERROR);
    return 1;
  }

  FILE* file = fopen("example.nbs", "r");
  if (file == NULL) {
    MessageBoxW(NULL, L"文件读取失败", L"Error", MB_ICONERROR);
    return 1;
  }

  size_t fileSize = getFileSize(file);
  char *buffer = malloc(fileSize);
  fread(buffer, 1, fileSize, file);
  readNBSFile(buffer, 0, &nbs);
  SkyAutoPlayOptions_t options = {0};
  buildTicksFrom(&options, &nbs, &builtTicks);
  tickCount = 0;//-200;
  tickIndex = 0;
  vec_size(&builtTicks, &maxIndex);
  vec_at(&builtTicks, 0, (void **)&currentTick);
  isPlaying = 1;
  hFinished = CreateEventW(NULL, 1, 0, L"__PLAY_DONE__");
  SetForegroundWindow(skyGameWnd);
  startTick(&id);
  WaitForSingleObject(hFinished, INFINITE);
  stopTick(&id);
  freeNBSFile(&nbs);
  fclose(file);
  switch (stopState) {
    case 0:
      printf("Completed.  ");
      break;
    case 1:
      printf("Stopped by exit piano keyboard.  ");
      break;
  }
  while (1);
  return 0;
}
