#ifndef ESJS_CUSTOM_LEXER_LEXER_H
#define ESJS_CUSTOM_LEXER_LEXER_H

#include "scanner.h"

typedef struct {
    Scanner scanner;
    TokenType previous_type;
    int has_previous;
} Lexer;

/**
 * @brief Initializes the lexer over an input buffer.
 */
void lexer_init(Lexer * lexer, Buffer * buffer);

/**
 * @brief Scans the next token using maximal munch.
 *
 * @param lexer Lexer instance.
 * @param token Output token.
 * @return int Non-zero when a token was produced, 0 on EOF.
 */
int lexer_next_token(Lexer * lexer, Token * token);

#endif // ESJS_CUSTOM_LEXER_LEXER_H