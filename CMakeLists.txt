cmake_minimum_required(VERSION 3.16)
project(AmazonBalancedTrees LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

if(MSVC)
  add_compile_options(/W4 /utf-8)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(trees STATIC src/AVLTree.cpp src/RBTree.cpp src/BalancedTree.cpp src/Console.cpp)
target_include_directories(trees PUBLIC include)

add_executable(arvores src/main.cpp)
add_executable(testes tests/tests.cpp)
add_executable(comparar src/compare.cpp)
foreach(target arvores testes comparar)
  target_link_libraries(${target} PRIVATE trees)
endforeach()

enable_testing()
add_test(NAME testes COMMAND testes)
