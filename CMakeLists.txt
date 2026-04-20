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
        include/esjs_lexer.h
)

# Library sources — everything except the CLI driver.
set(LIB_SOURCES
        src/token.c
        src/keywords.c
        src/charclass.c
        src/buffer.c
        src/scanner.c
        src/lexer.c
        src/dfa.c
        include/esjs_lexer.h
)

# ── Static library (importable by external projects) ─────────────────────────
add_library(esjs_lexer STATIC ${LIB_SOURCES} ${HEADERS})
target_include_directories(esjs_lexer PUBLIC include)

# ── CLI executable (links against the library) ────────────────────────────────
add_executable(esjs_custom_lexer src/main.c)
target_link_libraries(esjs_custom_lexer PRIVATE esjs_lexer)