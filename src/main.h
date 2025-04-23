#ifndef __x86_64__
#error This application must be compiled to 64 bit system.
#endif

#include <windows.h>
#include <stdio.h>
#include <locale.h>
#include <commdlg.h>
#include <fcntl.h>

#include "note.h"
#include "format/format.h"
#include "format/nbs.h"
#include "format/abc.h"
#include "random.h"
#include "macros.h"
#include "player.h"
#include "arg.h"
#include "config.h"
#include "text.h"

// Debug macros.
//#define DEBUG_NO_INSTANCE_DUPLICATE_CHECK
#define DEBUG_CONSOLE

// Software macros.
#define WM_USER_EXIT (0x8000 + 1)

#define MBError(text, type) (MessageBoxW(NULL, text, L"Error", MB_ICONERROR | type))

// Structs.
typedef struct {
  i8 exitWhenDone;
  i8 printHelp;
  i8 printVersion;
  Hotkey_t hkPlay;
  Hotkey_t hkPause;
  Hotkey_t hkStop;
  Hotkey_t hkOpen;
  SkyAutoPlayOptions_t playerOptions;
} SkyNBSPlayerOptions_t;