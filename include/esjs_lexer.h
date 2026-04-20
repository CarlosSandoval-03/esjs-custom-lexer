/**
* @file esjs_lexer.h
 * @brief Single-include public API for embedding the EsJS lexer as a library.
 *
 * Include this header (and only this header) when using the lexer from an
 * external project. It re-exports the three modules that make up the public
 * surface:
 *
 *  - token.h   : TokenType enum, Token struct, token_type_to_string()
 *  - buffer.h  : Buffer struct, buffer_init(), buffer_destroy(), buffer_get()
 *  - lexer.h   : Lexer struct, lexer_init(), lexer_next_token()
 *
 * ## Minimal usage example
 *
 * @code
 * #include "esjs_lexer.h"
 * #include <stdio.h>
 *
 * int main(void) {
 *   FILE *f = fopen("programa.esjs", "r");
 *
 *   Buffer buf;
 *   buffer_init(&buf, f);
 *
 *   Lexer lexer;
 *   lexer_init(&lexer, &buf);   // also calls dfa_init() internally
 *
 *   Token tok;
 *   while (lexer_next_token(&lexer, &tok)) {
 *     if (tok.type == TOKEN_EOF)   break;
 *     if (tok.type == TOKEN_ERROR) { /* handle error *\/ break; }
 *     // use tok.type, tok.lexeme_start, tok.lexeme_length, tok.line, tok.column
 *   }
 *
 *   buffer_destroy(&buf);
 *   fclose(f);
 *   return 0;
 * }
 * @endcode
 *
 * ## Linking
 *
 * CMake:
 * @code
 *   add_subdirectory(esjs-custom-lexer)
 *   target_link_libraries(my_target PRIVATE esjs_lexer)
 * @endcode
 *
 * GNU Make / manual compilation:
 * @code
 *   # Build the static library first:
 *   make -C esjs-custom-lexer lib
 *   # Then compile your code:
 *   gcc -Iesjs-custom-lexer/include -Lesjs-custom-lexer -lesjs_lexer my_program.c -o my_program
 * @endcode
 *
 * @note dfa_init() is called automatically by lexer_init() and is idempotent,
 *       so it is safe to create multiple Lexer instances from different buffers
 *       without reinitializing the DFA.
 */
#ifndef ESJS_LEXER_PUBLIC_H
#define ESJS_LEXER_PUBLIC_H

#include "token.h"
#include "buffer.h"
#include "lexer.h"

#endif  /* ESJS_LEXER_PUBLIC_H */