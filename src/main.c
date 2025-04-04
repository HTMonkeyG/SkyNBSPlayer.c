#include "main.h"

HWND skyGameWnd;
HANDLE hFinished;
NBS nbs;
char isPlaying = 0;
int tickCount = -1;
float tempo;
int aaa = 0;
NBSTickEffective *currentTick = NULL;

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
  NBSTickEffective *lastTick;
  SkyNoteKeys_t keys[100];
  float _t;
  int l;

  if (currentTick == NULL) {
    SetEvent(hFinished);
    return;
  }
  tickCount++;
  _t = (int)((float)currentTick->tick / tempo * 100);
  if (_t <= tickCount) {
    lastTick = currentTick;
    currentTick = currentTick->next;
    l = buildKeysFrom(lastTick, keys, 100);
    printf("%d %d %f %llu %d %d aaa\r", _t, lastTick->tick, tempo, currentTick, l, tickCount);
    sendKeySet(skyGameWnd, keys, l);
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
  currentTick = nbs.ticks;
  tickCount = -200;
  tempo = (float)nbs.header.tempo / 100.;
  isPlaying = 1;
  hFinished = CreateEventW(NULL, 1, 0, L"__PLAY_DONE__");
  SetForegroundWindow(skyGameWnd);
  startTick(&id);
  WaitForSingleObject(hFinished, INFINITE);
  stopTick(&id);
  freeNBSFile(&nbs);
  fclose(file);
  while (1);
  return 0;
}
