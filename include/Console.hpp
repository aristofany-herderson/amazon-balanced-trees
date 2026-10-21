#pragma once

#include <string>

namespace console {

// Ativa UTF-8 e sequências ANSI no terminal do Windows.
void setup();

bool colorsEnabled();
void setColors(bool enabled);
bool stdinIsTerminal();

std::string paint(const std::string &text, const char *ansiCode);

inline constexpr const char *kRed = "1;31";
inline constexpr const char *kBold = "1";
inline constexpr const char *kGreen = "32";
inline constexpr const char *kYellow = "33";
inline constexpr const char *kCyan = "36";
inline constexpr const char *kDim = "2";

} // namespace console
