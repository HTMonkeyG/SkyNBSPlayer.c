#include "text.h"

void printHelp() {
  printVersion();
  wprintf(L"格式:\n");
  wprintf(L"  skycol-nbs.exe [文件] [选项]\n");
  wprintf(L"选项:\n");
  wprintf(L"  -v, --version            显示软件版本");
  wprintf(L"  -h, --help               显示本帮助页面");
  wprintf(L"  -i <file>,\n");
  wprintf(L"  --input <file>           指定输入文件, 最后指定的生效");
}

void printVersion() {
  wprintf(L"Sky:Cotl NBS Player %s\n", VERSION);
  wprintf(L"Copyright (c) 2025 HTMonkeyG\n");
  wprintf(L"<https://www.github.com/HTMonkeyG/SkyNBSPlayer-C>\n");
}