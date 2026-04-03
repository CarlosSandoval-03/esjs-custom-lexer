#include <stdio.h>

#include "../include/lexer.h"

static int print_token(const Token *token) {
    const char *name = token_type_to_string(token->type);

    if (token->type == TOKEN_ERROR) {
        printf(">>> Error lexico (linea: %d, posicion: %d)\n", token->line,
               token->column);
        return -1;
    }

    if (token->type == TOKEN_KEYWORD) {
        printf("<%.*s,%d,%d>\n", (int) token->lexeme_length, token->lexeme_start,
               token->line, token->column);
        return 0;
    }

    if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_NUMBER ||
        token->type == TOKEN_STRING || token->type == TOKEN_REGEX) {
        printf("<%s,%.*s,%d,%d>\n", name, (int) token->lexeme_length,
               token->lexeme_start, token->line, token->column);
        return 0;
    }

    printf("<%s,%d,%d>\n", name, token->line, token->column);
    return 0;
}

int main(void) {
    Buffer buffer;
    Lexer lexer;
    Token token;

    dfa_init();
    buffer_init(&buffer, stdin);
    lexer_init(&lexer, &buffer);

    while (lexer_next_token(&lexer, &token)) {
        if (token.type == TOKEN_EOF) {
            break;
        }

        const int res = print_token(&token);
        if (res < 0) break; // If exists an error, stop the analysis
    }

    buffer_destroy(&buffer);
    return 0;
}