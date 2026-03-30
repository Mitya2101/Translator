cmake_minimum_required(VERSION 3.16)
project(MyLang)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MyLang
        main.cpp

        lexer.cpp
        tokens.cpp
        trie.cpp

        Syntaxer.cpp
        SyntaxerNode.cpp
        TFunc.cpp

        vm.cpp
)

target_include_directories(MyLang PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
configure_file(${CMAKE_SOURCE_DIR}/keywords.txt
               ${CMAKE_BINARY_DIR}/keywords.txt
               COPYONLY)


enable_testing()
add_subdirectory(tests)

