#include <stdio.h>
#include <string.h>
#include <windows.h>

#define AS_REJECT  0x7FFFFFFF
#define AS_INITIAL 0xFFFFFFFF

typedef void (ArgCallback_t)(const wchar_t *value, int count, int *state);

int buildArgFrom(ArgCallback_t callback);