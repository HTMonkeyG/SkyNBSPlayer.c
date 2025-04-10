/**
 * Sky: CotL NBS Music Player
 * 
 * An autoplay tool designed for Netease edition of Sky on Windows.
 * 
 * Copyright (c) 2025 HTMonkeyG
 * 
 * (https://www.github.com/HTMonkeyG/SkyNBSPlayer)
 */

#include "main.h"

HWND hSkyGameWnd;
NBS nbs;
Vector_t builtTicks = {0};
f32 tempo;
SkyMusicTick_t *currentTick = NULL;
SkyAutoPlayOptions_t options = {0};

i32 readNBS(wchar_t *path, NBS *nbs) {
  FILE *file;
  size_t fileSize;

  if (!path || !nbs)
    return 0;

  file = _wfopen(path, L"rb");
  if (!file)
    return 0;

  fseek(file, 0, SEEK_END);
  fileSize = ftell(file);
  rewind(file);

  i08 *buffer = malloc(fileSize);
  fread(buffer, 1, fileSize, file);
  readNBSFile(buffer, 0, nbs);

  free(buffer);
  fclose(file);

  return 1;
}

int main() {
  HANDLE hMutex;
  SkyMusicPlayer_t player;

  hMutex = CreateMutexW(NULL, TRUE, L"__SKY_NBS__");
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    MessageBoxW(NULL, L"已有实例正在运行", L"Error", MB_ICONERROR);
    return 1;
  }

  // Get game window handle.
  hSkyGameWnd = FindWindowW(NULL, L"光·遇");
  while (!hSkyGameWnd) {
    if (MessageBoxW(NULL, L"游戏未运行，是否重试？", L"Error", MB_ICONERROR | MB_YESNO) == IDNO)
      return 1;
    hSkyGameWnd = FindWindowW(NULL, L"光·遇");
  }

  if (!readNBS(L"example.nbs", &nbs)) {
    MessageBoxW(NULL, L"文件读取失败", L"Error", MB_ICONERROR);
    return 1;
  }
  buildTicksFrom(&options, &nbs, &builtTicks);
  freeNBSFile(&nbs);

  SetForegroundWindow(hSkyGameWnd);

  snCreatePlayer(&player, &options, hSkyGameWnd, &builtTicks);
  Sleep(100);
  snMusicPlay(&player);

  while (1) {
    if (player.state < 0)
      break;
    if (player.state > 0)
      snMusicPlay(&player);
    Sleep(100);
  }
  switch (player.state) {
    case STOPPED_EOF:
      printf("Completed.");
      break;
    case STOPPED_ESC:
      printf("Stopped by exit piano keyboard.");
      break;
    case STOPPED_PROG:
      printf("Stopped by program.");
      break;
    case PAUSED_BG:
      printf("Paused by leave game window.");
      break;
    default:
      break;
  }
  while (1);
  ReleaseMutex(hMutex);
  CloseHandle(hMutex);
  return 0;
}
