#ifndef __x86_64__
#error This application must be compiled to 64 bit system.
#endif

#include <windows.h>
#include <stdio.h>
#include <locale.h>

#include "note.h"
#include "nbs.h"
#include "random.h"
#include "macros.h"
#include "player.h"
#include "arg.h"
#include "config.h"
#include "text.h"

//#define DEBUG_NO_GAME_RUNNING_CHECK
//#define DEBUG_NO_INSTANCE_DUPLICATE_CHECK
#define DEBUG_CONSOLE

#define MBError(text, type) (MessageBoxW(NULL, text, L"Error", MB_ICONERROR | type))

#ifdef DEBUG_CONSOLE
#define LOG (wprintf)
#else
#define LOG
#endif