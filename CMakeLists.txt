cmake_minimum_required(VERSION 3.10)
project(simple_lang)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(simple_lang main.cpp parser/parser.cpp scanner/scanner.cpp token/token.cpp token/position.cpp ast/expr.h ast/stmt.h ast/decl.h ast/field.h ast/file.h ast/type_node.h ast/define.h)