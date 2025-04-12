#include <stdio.h>
#include <string.h>

typedef void (ConfigCallback_t)(const wchar_t *key, const wchar_t *value);

int buildConfigFrom(FILE *file, ConfigCallback_t callback);