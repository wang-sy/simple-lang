cmake_minimum_required(VERSION 3.10)
project(simple_lang)

set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")
add_compile_options(-g)   #添加语句

include_directories(.)

add_executable(simple_lang main.cpp parser/parser.cpp scanner/scanner.cpp token/token.cpp token/position.cpp check/check.cpp parser/var_table.cpp)