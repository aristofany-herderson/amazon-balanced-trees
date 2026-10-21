#!/usr/bin/env sh
set -e
cd "$(dirname "$0")"
mkdir -p bin
CXX="${CXX:-g++}"
FLAGS="-std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude"
CORE="src/AVLTree.cpp src/RBTree.cpp src/BalancedTree.cpp src/Console.cpp"
case "$(uname -s)" in MINGW*|MSYS*|CYGWIN*) EXT=".exe"; LINK="-static" ;; *) EXT=""; LINK="" ;; esac
$CXX $FLAGS $CORE src/main.cpp $LINK -o "bin/arvores$EXT"
$CXX $FLAGS $CORE tests/tests.cpp $LINK -o "bin/testes$EXT"
$CXX $FLAGS $CORE src/compare.cpp $LINK -o "bin/comparar$EXT"
echo "Executáveis gerados em bin/"
