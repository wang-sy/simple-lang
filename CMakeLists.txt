cmake_minimum_required(VERSION 3.10)
project(simple_lang)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(simple_lang main.cpp parser/parser.cpp scanner/scanner.cpp token/token.cpp token/position.cpp)