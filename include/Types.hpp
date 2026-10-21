#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>

using Key = std::int64_t;

struct Stats {
  std::size_t leftRotations = 0;
  std::size_t rightRotations = 0;
  std::size_t recolors = 0;
  std::size_t heightUpdates = 0;
  std::map<std::string, std::size_t> cases;

  std::size_t rotations() const { return leftRotations + rightRotations; }
};

struct SearchResult {
  bool found = false;
  int visited = 0;
};
