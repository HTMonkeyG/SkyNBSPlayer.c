#include <stdio.h>
#include <string.h>
#include <windows.h>

typedef struct {
  unsigned int mod;
  unsigned int vk;
} Hotkey_t;

typedef void (ConfigCallback_t)(const wchar_t *key, const wchar_t *value);

int buildConfigFrom(FILE *file, ConfigCallback_t callback);
int buildHotkeyFrom(const wchar_t *desc, Hotkey_t *hotkey);
