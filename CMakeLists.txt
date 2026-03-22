cmake_minimum_required(VERSION 4.1)
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
        src/token.c)
