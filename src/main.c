/**
 * Sky: CotL NBS Music Player
 * 
 * An autoplay tool designed for Netease edition of Sky on Windows.
 * 
 * Copyright (c) 2025 HTMonkeyG
 * 
 * (https://www.github.com/HTMonkeyG/SkyNBSPlayer-C)
 */

#include "main.h"

HWND hSkyGameWnd;
NBS nbs;
Vector_t builtTicks = {0};
f32 tempo;
SkyMusicTick_t *currentTick = NULL;
SkyAutoPlayOptions_t options = {
  .highTps = 0,
  .randomShift = 0,
  .shiftStrength = 0,
  .shiftType = 0,
  .fps = 60
};
wchar_t nbsPath[MAX_PATH] = {0}
  , cfgPath[MAX_PATH];

i32 readNBS(wchar_t *path, NBS *nbs) {
  FILE *file;
  size_t fileSize;
  i32 result;

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
  result = readNBSFile(buffer, fileSize, nbs);
  free(buffer);
  fclose(file);

  return result;
}

void cfgCallback(const wchar_t *key, const wchar_t *value) {
  wchar_t *p;
  LOG(L"%s: %s\n", key, value);

  if (!wcscmp(key, L"high_tps"))
    options.highTps = (wcstof(value, &p) != 0);
  else if (!wcscmp(key, L"frame_rate"))
    options.fps = wcstoul(value, &p, 10);
}

void argCallback(const wchar_t *value, int count, int *state) {
  LOG(L"%d %s\n", count, value);

  if (!count && (!wcscmp(value, L"i") || !wcscmp(value, L"input")))
    // -i <file to read>
    *state = 1;
  
  if (*state == 1 && count > 0) {
    // -i <file to read>
    LOG(L"Input file: %s\n", value);
    wcscpy_s(nbsPath, MAX_PATH, value);
    *state = 0;
  }
  if (*state == AS_INITIAL && count == 2) {
    // skycol-nbs.exe <file to read>
    LOG(L"Input file: %s\n", value);
    wcscpy_s(nbsPath, MAX_PATH, value);
    *state = 0;
  }
}
 /*
DWORD WINAPI hotkeyThread(LPVOID lpParam) {
  MSG msg;

 
  if (!RegisterHotKey(NULL, 1, hotkeyMod, hotkeyVK))
    return 1;
  SetEvent(hEvent);

  while (GetMessageW(&msg, NULL, 0, 0)) {
    switch (msg.message) {
      case WM_HOTKEY:
        break;
      case WM_TIMER:
        break;
    }
  }

Exit:
  UnregisterHotKey(NULL, 1);
  return 0;
}*/

int main() {
  HANDLE hMutex;
  SkyMusicPlayer_t player;
  wchar_t *p;

  setlocale(LC_ALL, "zh_CN.UTF-8");

#ifndef DEBUG_CONSOLE
  // Close console window.
  FreeConsole();
#endif

#ifndef DEBUG_NO_INSTANCE_DUPLICATE_CHECK
  // Check whether another instance is running.
  hMutex = CreateMutexW(NULL, TRUE, L"__SKY_NBS__");
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    MBError(L"已有实例正在运行", 0);
    return 1;
  }
#endif

  // Get config file path.
  if (!GetModuleFileNameW(NULL, cfgPath, MAX_PATH)) {
    MBError(L"获取可执行文件目录失败", 0);
    return 0;
  }
  p = wcsrchr(cfgPath, L'\\');
  if (!p) {
    MBError(L"初始化配置文件目录失败", 0);
    return 0;
  }
  *p = 0;
  wcscat(cfgPath, L"\\skynbs-config.txt");
  FILE *f = _wfopen(cfgPath, L"r");
  if (!f) {
    MBError(L"配置文件不存在", 0);
    return 0;
  }

  // Read argv and config file.
  LOG(L"Reading config: %s...\n", cfgPath);
  buildConfigFrom(f, cfgCallback);
  LOG(L"Reading argv...\n");
  buildArgFrom(argCallback);

#ifndef DEBUG_NO_GAME_RUNNING_CHECK
  // Get game window handle.
  hSkyGameWnd = FindWindowW(NULL, L"光·遇");
  while (!hSkyGameWnd) {
    if (MBError(L"游戏未运行，是否重试？", MB_YESNO) == IDNO)
      return 1;
    hSkyGameWnd = FindWindowW(NULL, L"光·遇");
  }
  LOG(L"Get game window handle: %d.\n", hSkyGameWnd);
#endif

  if (!wcslen(nbsPath)) {
    MBError(L"未指定文件路径", 0);
    wcscpy(nbsPath, L"./example.nbs");
  }

  LOG(L"Reading NBS file: %s\n", nbsPath);
  if (!readNBS(nbsPath, &nbs)) {
    MBError(L"文件读取失败", 0);
    return 1;
  }
  if (nbs.header.tempo < 0) {
    MBError(L"无效NBS文件", 0);
    return 1;
  }
  LOG(L"Tempo: %f\n", (f32)nbs.header.tempo / 100.);
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
      LOG(L"Completed.");
      break;
    case STOPPED_ESC:
      LOG(L"Stopped by exit piano keyboard.");
      break;
    case STOPPED_PROG:
      LOG(L"Stopped by program.");
      break;
    case PAUSED_BG:
      LOG(L"Paused by leave game window.");
      break;
    default:
      break;
  }
  while (1);
  ReleaseMutex(hMutex);
  CloseHandle(hMutex);
  return 0;
}
