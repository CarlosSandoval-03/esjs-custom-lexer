cmake_minimum_required(VERSION 4.1)
project(esjs_custom_lexer C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(HEADERS
        include/charclass.h
        include/dfa.h
        include/token.h
        include/buffer.h
        include/scanner.h
        include/lexer.h
        include/keywords.h
)

set(SOURCES
        src/main.c
        src/token.c
        src/keywords.c
        src/charclass.c
        src/buffer.c
        src/scanner.c
        src/lexer.c
        src/dfa.c
)

add_executable(esjs_custom_lexer ${SOURCES} ${HEADERS})

target_include_directories(esjs_custom_lexer PUBLIC include)
