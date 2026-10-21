#include "Console.hpp"

#include <cstdio>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace console {
namespace {
bool gColors = true;
}

void setup() {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode))
    SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  else
    gColors = false;
#else
  gColors = isatty(fileno(stdout)) != 0;
#endif
}

bool colorsEnabled() { return gColors; }
void setColors(bool enabled) { gColors = enabled; }

bool stdinIsTerminal() {
#ifdef _WIN32
  return _isatty(_fileno(stdin)) != 0;
#else
  return isatty(fileno(stdin)) != 0;
#endif
}

std::string paint(const std::string &text, const char *ansiCode) {
  if (!gColors)
    return text;
  return std::string("\033[") + ansiCode + "m" + text + "\033[0m";
}

} // namespace console
