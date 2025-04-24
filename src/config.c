#include "config.h"

int buildConfigFrom(FILE *file, ConfigCallback_t callback) {
  int counter = 0;
  wchar_t buffer[128]
    , *line, *o, *p, *q;
  while (!feof(file)) {
    fgetws(buffer, 128, file);
    o = buffer + wcslen(buffer) - 1;
    // Cut the line feed and space after the line.
    while (buffer <= o && (*o == L'\n' || *o == L'\r' || *o == L'\t' || *o == L' '))
      *o = 0, o--;
    line = buffer;
    // Skip whitespace.
    while (*line == L'\t' || *line == L' ')
      line++;
    // Process comments.
    if (line[0] == L'#' || !line[0])
      continue;
    p = wcschr(line, L':');
    // Invalid line.
    if (p == NULL)
      continue;
    // Trim the space after the key.
    q = p - 1;
    while (q > line && (*q == L'\t' || *q == L' '))
      *q = 0, q--;
    // Set line terminator of the key.
    *p = 0;
    p++;
    // Trim the space before the value.
    while (*p == L'\t' || *p == L' ' || *p == L'\r')
      p++;
    if (*p == L'\n')
      // Empty value.
      callback(line, L"");
    else
      callback(line, p);
    counter++;
  }

  return counter;
}

int buildHotkeyFrom(const wchar_t *desc, Hotkey_t *hotkey) {
  wchar_t buffer[128]
    , nums[32]
    , *p;
  size_t l, m, n;
  unsigned int mod = 0
    , vk = 0;

  if (!desc || !hotkey)
    return 0;

  wcsncpy(buffer, desc, 127);
  buffer[127] = 0;
  l = wcslen(desc);

  if (!l)
    return 0;

  for (m = 0; m < l;) {
    while (buffer[m] == L' ' || buffer[m] == L'+')
      m++;
    if (!wcsnicmp(buffer + m, L"alt", 3)) {
      mod |= MOD_ALT;
      m += 3;
    } else if (!wcsnicmp(buffer + m, L"ctrl", 4)) {
      mod |= MOD_CONTROL;
      m += 4;
    } else if (!wcsnicmp(buffer + m, L"shift", 5)) {
      mod |= MOD_SHIFT;
      m += 5;
    } else if (buffer[m] >= L'0' && buffer[m] <= L'9') {
      n = m;
      do
        n++;
      while (buffer[n] >= L'0' && buffer[n] <= L'9' && n - m < 31);
      memcpy(nums, buffer + m, (n - m) * sizeof(wchar_t));
      nums[31] = 0;
      vk = wcstoul(nums, &p, 10);
      break;
    } else
      return 0;
  }

  hotkey->mod = mod;
  hotkey->vk = vk;

  return 1;
}