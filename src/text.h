#include <stdio.h>
#include <windows.h>
#include <stdarg.h>

#define VERSION L"v0.0.4"

#define LOG (logTimeStamp)

void printHelp();
void printVersion();
void logTimeStamp(const wchar_t *format, ...);