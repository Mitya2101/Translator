cmake_minimum_required(VERSION 3.29)
project(Lexer)

set(CMAKE_CXX_STANDARD 20)

add_executable(Lexer main.cpp
        lexer.cpp
        lexer.h
        tokens.cpp
        tokens.h
        trie.cpp
        trie.h)
