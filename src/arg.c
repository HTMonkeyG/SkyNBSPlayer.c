#include "arg.h"

int buildArgFrom(ArgCallback_t callback) {
  int ctr = 0
    , r = 0
    , state = AS_INITIAL
    , argc;
  wchar_t *s = GetCommandLineW()
    , **argv = CommandLineToArgvW(s, &argc)
    , buffer[260]
    , *p;
  if (!argv || !argc)
    return 0;
  for (int i = 0; i < argc; i++) {
    if (argv[i][0] == L'-') {
      p = argv[i];
      while (*p == L'-')
        p++;
      if (wcslen(p) > 0) {
        ctr = 0;
        wcscpy_s(buffer, 260, p);
      } else {
        ctr++;
        wcscpy_s(buffer, 260, argv[i]);
      }
    } else {
      ctr++;
      wcscpy_s(buffer, 260, argv[i]);
    }
    callback(buffer, ctr, &state);
    r++;
  }
  LocalFree(argv);
  return r;
}