cmake_minimum_required(VERSION 3.19)
project(simple_lang)

set(CMAKE_CXX_STANDARD 11)

add_executable(simple_lang main.cpp token.h error.h token.cpp scanner.cpp scanner.h position.cpp position.h)