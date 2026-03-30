cmake_minimum_required(VERSION 3.16)
project(MyLang)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(MyLang
        main.cpp

        src/lexer/lexer.cpp
        src/lexer/tokens.cpp
        src/lexer/trie.cpp

        src/Syntaxer/Syntaxer.cpp
        src/Syntaxer/SyntaxerNode.cpp
        src/SYntaxer/TFunc.cpp

        src/VirtualMachine/vm.cpp
)

target_include_directories(MyLang PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
configure_file(${CMAKE_SOURCE_DIR}/include/lexer/keywords.txt
               ${CMAKE_BINARY_DIR}/include/lexer/keywords.txt
               COPYONLY)


enable_testing()
add_subdirectory(tests)

