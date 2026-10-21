@echo off
setlocal
cd /d "%~dp0"
if not exist bin mkdir bin
set FLAGS=-std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude
set CORE=src\AVLTree.cpp src\RBTree.cpp src\BalancedTree.cpp src\Console.cpp
g++ %FLAGS% %CORE% src\main.cpp -static -o bin\arvores.exe || exit /b 1
g++ %FLAGS% %CORE% tests\tests.cpp -static -o bin\testes.exe || exit /b 1
g++ %FLAGS% %CORE% src\compare.cpp -static -o bin\comparar.exe || exit /b 1
echo Executaveis gerados em bin\
