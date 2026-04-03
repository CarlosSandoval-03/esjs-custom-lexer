cmake_minimum_required(VERSION 3.31)
project(esjs_custom_lexer C)

set(CMAKE_C_STANDARD 11)

add_executable(esjs_custom_lexer src/main.c
        include/charclass.h
        include/dfa.h
        include/token.h
        include/buffer.h
        include/scanner.h
        include/lexer.h
        include/keywords.h
        src/token.c
        src/keywords.c
        src/charclass.c
        src/buffer.c
        src/scanner.c
        src/lexer.c
        src/dfa.c)
