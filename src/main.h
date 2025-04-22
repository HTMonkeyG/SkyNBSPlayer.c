#ifndef __x86_64__
#error This application must be compiled to 64 bit system.
#endif

#include <windows.h>
#include <stdio.h>
#include <locale.h>
#include <commdlg.h>

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

//#define DEBUG_NO_GAME_RUNNING_CHECK
//#define DEBUG_NO_INSTANCE_DUPLICATE_CHECK
#define DEBUG_CONSOLE

typedef struct {
  i8 runOnce;
  i8 printHelp;
  i8 printVersion;
  Hotkey_t hkPlay;
  Hotkey_t hkPause;
  Hotkey_t hkStop;
  Hotkey_t hkOpen;
  SkyAutoPlayOptions_t playerOptions;
} SkyNBSPlayerOptions_t;