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
HANDLE hMutex, hHotkeyThread;
DWORD mainThreadId
  , hotkeyThreadId;
Vector_t builtTicks = {0};
SkyNBSPlayerOptions_t options = {
  .printVersion = 0,
  .printHelp = 0,
  .exitWhenDone = 0,
  .hkOpen = {
    .mod = MOD_CONTROL,
    .vk = 'O'
  },
  .hkPlay = {
    .mod = MOD_ALT,
    .vk = 'O'
  },
  .hkPause = {
    .mod = MOD_ALT,
    .vk = 'O'
  },
  .hkStop = {
    .mod = MOD_ALT,
    .vk = 'I'
  },
  .playerOptions = {
    .highTps = 0,
    .randomShift = 0,
    .shiftStrength = 0,
    .shiftType = 0,
    .minIntevalMs = 10
  }
};
SkyMusicPlayer_t player = {0};
wchar_t nbsPath[MAX_PATH] = {0}
  , exePath[MAX_PATH];

/**
 * Get game window handle with window name.
 */
i32 getGameWnd() {
  // Get game window handle.
  hSkyGameWnd = FindWindowW(NULL, L"光·遇");
  while (!hSkyGameWnd) {
    if (MBError(L"游戏未运行，是否重试？", MB_YESNO) == IDNO)
      return 0;
    hSkyGameWnd = FindWindowW(NULL, L"光·遇");
  }
  LOG(L"Get game window handle: 0x%X.\n", hSkyGameWnd);
  SetForegroundWindow(hSkyGameWnd);
  return 1;
}

/**
 * Open dialog box to browse file.
 */
i32 pickFile(const wchar_t *path, wchar_t *result, i32 maxLength) {
  OPENFILENAMEW ofn = {0};
  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = NULL;
  ofn.lpstrFile = result;
  ofn.nMaxFile = maxLength;
  ofn.lpstrFilter = L"All(*.*)\0*.*\0Note Block Studio(*.nbs)\0*.nbs\0Sky Studio(*.txt)\0*.txt\0\0";
  ofn.nFilterIndex = 2;
  ofn.lpstrFileTitle = NULL;
  ofn.lpstrInitialDir = path;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
  return GetOpenFileNameW(&ofn);
}

/**
 * Read file with given path and options, and preprocess it into song ticks.
 */
i32 readAndBuildSong(
  wchar_t *path,
  SkyNBSPlayerOptions_t *options,
  Vector_t *builtTicks
) {
  FILE *file;
  GeneralSongTicks_t song = {0};
  size_t fileSize, stringLength, tickCount;
  wchar_t *wbuffer;
  i08 bom[2]
    , *buffer;
  i32 result;

  if (!path || !builtTicks || !options)
    return 0;

  LOG(L"Reading song file: %ls\n", path);

  file = _wfopen(path, L"rb");
  if (!file)
    return 0;

  fseek(file, 0, SEEK_END);
  fileSize = ftell(file);
  rewind(file);

  // Get file encoding with bom.
  // Default encoding of Sky Studio is UTF16-LE, but we also need to support
  // UTF-8.
  if (fread(bom, 2, 1, file) != 1) {
    fclose(file);
    goto ErrRet;
  }
  fseek(file, 0, SEEK_SET);

  // I don't know why, but if I set bom as an u16 and directly pass &bom to
  // fread(), then the next if statement won't be compiled correctly, even with
  // "volatile u16 bom". So I set it as an byte array and use pointer reinter-
  // -pret cast to convert it.
  if (*(u16 *)&bom == 0xFEFF) {
    // UTF16-LE
    // cJSON only recieves char, so we should convert wchar_t to char.
    file = _wfreopen(path, L"r, ccs=UTF-8", file);
    wbuffer = malloc(fileSize + 2);
    fread(wbuffer, 1, fileSize, file);
    stringLength = WideCharToMultiByte(
      CP_ACP,
      WC_NO_BEST_FIT_CHARS,
      wbuffer,
      -1,
      NULL,
      0,
      NULL, 
      NULL
    );
    buffer = malloc(stringLength + 1);
    WideCharToMultiByte(
      CP_ACP,
      WC_NO_BEST_FIT_CHARS,
      wbuffer,
      -1,
      buffer,
      stringLength,
      NULL,
      NULL
    );
    free(wbuffer);
  } else {
    // Binary file or UTF-8 without BOM.
    buffer = malloc(fileSize);
    fread(buffer, 1, fileSize, file);
  }
  result = readSongFile(buffer, fileSize, &song);
  free(buffer);
  fclose(file);

  if (!result) {
ErrRet:
    MBError(L"文件读取失败", 0);
    return 0;
  }
  if (song.tps < 0) {
    MBError(L"无效音乐文件", 0);
    return 0;
  }
  LOG(L"Song meta:\n");
  LOG(L"- Name: %ls\n", song.name);
  LOG(L"- Author: %ls\n", song.originalAuthor);
  LOG(L"- Transcriber: %ls\n", song.author);
  LOG(L"- Tempo: %f\n", song.tps);
  LOG(L"Compiling notes...\n");
  buildTicksFrom(&options->playerOptions, &song, builtTicks);
  vec_size(builtTicks, &tickCount);
  LOG(L"Compiled %llu ticks.\n", tickCount);

  return 1;
}

/**
 * Create music player with global nbsPath.
 */
i32 reinitPlayer() {
  // Browse successed.
  vec_init(&builtTicks, sizeof(SkyMusicTick_t));
  snRemovePlayer(&player);
  if (!readAndBuildSong(nbsPath, &options, &builtTicks))
    return 0;
  SetForegroundWindow(hSkyGameWnd);
  return snCreatePlayer(
    &player,
    &options.playerOptions,
    hSkyGameWnd,
    &builtTicks
  );
}

/**
 * Config file reader.
 */
void cfgCallback(const wchar_t *key, const wchar_t *value) {
  wchar_t *p;
  u32 fps = 0;
  LOG(L"- %ls: %ls\n", key, value);

  if (!wcscmp(key, L"high_tps"))
    options.playerOptions.highTps = (wcstof(value, &p) != 0);
  else if (!wcscmp(key, L"frame_rate")) {
    fps = wcstoul(value, &p, 10);
    if (p == value)
      fps = 60;
    if (fps < 30)
      fps = 30;
    if (fps > 120)
      fps = 120;
    options.playerOptions.minIntevalMs = 2 * (u32)ceilf(1000. / (f32)fps);
  }
}

/**
 * Argv reader.
 */
void argCallback(const wchar_t *value, int count, int *state) {
  LOG(L"- %d %ls\n", count, value);

  if (!count && (!wcscmp(value, L"i") || !wcscmp(value, L"input")))
    // -i <file to read>
    *state = 1;
  
  if (*state == 1 && count > 0) {
    // -i <file to read>
    LOG(L"Input file: %ls\n", value);
    wcscpy_s(nbsPath, MAX_PATH, value);
    *state = 0;
  }
  if (*state == AS_INITIAL && count == 2) {
    // skycol-nbs.exe <file to read>
    LOG(L"Input file: %ls\n", value);
    options.exitWhenDone = 1;
    wcscpy_s(nbsPath, MAX_PATH, value);
    *state = 0;
  }
}

/**
 * Listening hotkey input.
 */
DWORD WINAPI hotkeyThread(LPVOID lpParam) {
  MSG msg;
 
  if (!RegisterHotKey(NULL, 1, options.hkOpen.mod, options.hkOpen.vk)) {
    MBError(L"注册打开文件快捷键失败", 0);
    return 1;
  }
  if (!RegisterHotKey(NULL, 2, options.hkPlay.mod, options.hkPlay.vk)) {
    MBError(L"注册播放快捷键失败", 0);
    return 1;
  }

  while (GetMessageW(&msg, NULL, 0, 0)) {
    if (msg.message == WM_USER_EXIT)
      // Terminate thread.
      PostQuitMessage(0);
    else if (msg.message == WM_QUIT)
      // Exit message loop.
      break;
    else if (msg.message != WM_HOTKEY || GetForegroundWindow() != hSkyGameWnd)
      // Ignore other messages.
      continue;
    else if (msg.message == WM_HOTKEY && msg.wParam == 1) {
      // Open a new file.
      snMusicStop(&player);
      // If opened a new file while playing, then does not exit.
      options.exitWhenDone = 0;
      // Try to browse file.
      if (!pickFile(exePath, nbsPath, MAX_PATH)) {
        // Browse failed.
        if (CommDlgExtendedError())
          MBError(L"选择文件失败", 0);
        else
          LOG(L"User cancelled file selection.\n");
        continue;
      }
      if (!reinitPlayer())
        continue;
    } else if (msg.message == WM_HOTKEY && msg.wParam == 2) {
      // Play current file.
      if (player.state > 0) {
        snMusicPlay(&player);
        LOG(L"Play, %d.\n", player.state);
      } else if (
        player.state == STOPPED_PROG
        || player.state == STOPPED_ESC
      ) {
        snMusicResume(&player);
        LOG(L"Resume, %d.\n", player.state);
      } else if (player.state == PLAYING) {
        snMusicPause(&player);
        LOG(L"Pause, %d.\n", player.state);
      }
    }
  }

  snRemovePlayer(&player);
  UnregisterHotKey(NULL, 1);
  UnregisterHotKey(NULL, 2);
  return 0;
}

/**
 * Initialize the software.
 */
i32 initSoftware() {
  wchar_t cfgPath[MAX_PATH]
    , *p;

#ifndef DEBUG_CONSOLE
  // Close console window.
  FreeConsole();
  freopen("skynbs-log.txt", "w", stdout);
  _setmode(_fileno(stdout), _O_U8TEXT);
#endif

#ifndef DEBUG_NO_INSTANCE_DUPLICATE_CHECK
  // Check whether another instance is running.
  hMutex = CreateMutexW(NULL, TRUE, L"__SKY_NBS__");
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    MBError(L"已有实例正在运行", 0);
    return 0;
  }
#endif

  // Get config file path.
  if (!GetModuleFileNameW(NULL, exePath, MAX_PATH)) {
    LOG(L"Failed to get module path.");
    goto ErrReadCfg;
  }
  p = wcsrchr(exePath, L'\\');
  if (!p) {
    LOG(L"Failed to initialize config file path.", 0);
    goto ErrReadCfg;
  }
  *p = 0;
  wcscpy(cfgPath, exePath);
  wcscat(cfgPath, L"\\skynbs-config.txt");

  // Open config file.
  FILE *f = _wfopen(cfgPath, L"r");
  if (!f) {
    LOG(L"Missing config file.");

ErrReadCfg:
    MBError(L"读取配置文件失败", 0);
    return 0;
  }

  // Read argv and config file.
  LOG(L"Reading config: %ls...\n", cfgPath);
  buildConfigFrom(f, cfgCallback);
  fclose(f);
  LOG(L"Reading argv...\n");
  buildArgFrom(argCallback);

  if (!getGameWnd())
    return 0;

  // Create hotkey listener.
  hHotkeyThread = CreateThread(NULL, 0, hotkeyThread, 0, 0, &hotkeyThreadId);
  if (!hHotkeyThread) {
    MBError(L"创建子线程失败", 0);
    return 0;
  }

  return 1;
}

/** 
 * Main function.
 */
DWORD mainThread() {
  MSG msg;
  DWORD pid;

  SetTimer(NULL, 1, 1000 / 32, NULL);

  while (GetMessageW(&msg, NULL, 0, 0)) {
    if (msg.message == WM_QUIT)
      break;
    else if (msg.message == WM_TIMER) {
      // Terminate hotkey thread when the game is closed or single file
      // playing ends.
      if (
        !GetWindowThreadProcessId(hSkyGameWnd, &pid)
        || (options.exitWhenDone && player.state == STOPPED_EOF)
      ) {
        // Exit hotkey process.
        PostThreadMessageW(hotkeyThreadId, WM_USER_EXIT, 0, 0);
        PostQuitMessage(0);
      }
    }
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  KillTimer(NULL, 1);

  return msg.wParam;
}

int main() {
  setlocale(LC_ALL, "");

  mainThreadId = GetCurrentThreadId();

  // Initialize.
  if (!initSoftware())
    return 1;

  if (wcslen(nbsPath)) {
    // If produced file path through command line, then only play this file.
    options.exitWhenDone = 1;
    reinitPlayer();
  }

  // Run main thread.
  mainThread();

  // Release resources.
  WaitForSingleObject(hHotkeyThread, INFINITE);
  ReleaseMutex(hMutex);
  CloseHandle(hHotkeyThread);
  CloseHandle(hMutex);
  return 0;
}
