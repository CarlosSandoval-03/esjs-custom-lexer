#ifndef ESJS_CUSTOM_LEXER_TOKEN_H
#define ESJS_CUSTOM_LEXER_TOKEN_H

#include <stdio.h>

// WARNING: Each value requires its own representation in token.c
typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_CONST,
    TOKEN_LET,
    TOKEN_VAR,
    TOKEN_FUNCTION,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_DO,
    TOKEN_SWITCH,
    TOKEN_CASE,
    TOKEN_DEFAULT,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_TRY,
    TOKEN_CATCH,
    TOKEN_FINALLY,
    TOKEN_THROW,
    TOKEN_CLASS,
    TOKEN_EXTENDS,
    TOKEN_NEW,
    TOKEN_THIS,
    TOKEN_SUPER,
    TOKEN_IMPORT,
    TOKEN_EXPORT,
    TOKEN_FROM,
    TOKEN_AS,
    TOKEN_ASYNC,
    TOKEN_AWAIT,
    TOKEN_YIELD,
    TOKEN_IN,
    TOKEN_OF,
    TOKEN_INSTANCEOF,
    TOKEN_TYPEOF,
    TOKEN_VOID,
    TOKEN_DELETE,
    TOKEN_DEBUGGER,
    TOKEN_WITH,
    TOKEN_STATIC,
    TOKEN_GET,
    TOKEN_SET,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_UNDEFINED,
    TOKEN_ENUM,
    TOKEN_IMPLEMENTS,
    TOKEN_INTERFACE,
    TOKEN_PACKAGE,
    TOKEN_PRIVATE,
    TOKEN_PROTECTED,
    TOKEN_PUBLIC,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_REGEX,
    TOKEN_EOF,
    TOKEN_ERROR,

    TOKEN_PLUS, // +
    TOKEN_MINUS, // -
    TOKEN_TIMES, // *
    TOKEN_DIVIDE, // /
    TOKEN_POWER, // **
    TOKEN_MOD, // %

    TOKEN_ASSIGN, // =
    TOKEN_MOD_ASSIGN, // %=
    TOKEN_DIV_ASSIGN, // /=
    TOKEN_TIMES_ASSIGN, // *=
    TOKEN_MINUS_ASSIGN, // -=
    TOKEN_PLUS_ASSIGN, // +=
    TOKEN_POWER_ASSIGN, // **=

    TOKEN_EQUAL, // ==
    TOKEN_STRICT_EQUAL, // ===
    TOKEN_NOT_EQUAL, // !=
    TOKEN_STRICT_NOT_EQUAL, // !==

    TOKEN_LESS, // <
    TOKEN_LESS_EQUAL, // <=
    TOKEN_GREATER, // >
    TOKEN_GREATER_EQUAL, // >=

    TOKEN_LOGICAL_AND, // &&
    TOKEN_LOGICAL_OR, // ||

    TOKEN_NOT, // !
    TOKEN_BIT_AND, // &
    TOKEN_BIT_OR, // |
    TOKEN_BIT_XOR, // ^
    TOKEN_BIT_NOT, // ~

    TOKEN_SPREAD, // ...
    TOKEN_PERIOD, // .
    TOKEN_COMMA, // ,
    TOKEN_SEMICOLON, // ;
    TOKEN_COLON, // :

    TOKEN_OPENING_KEY, // {
    TOKEN_CLOSING_KEY, // }
    TOKEN_OPENING_BRA, // [
    TOKEN_CLOSING_BRA, // ]
    TOKEN_OPENING_PAR, // (
    TOKEN_CLOSING_PAR, // )

    TOKEN_INCREMENT, // ++
    TOKEN_DECREMENT, // --

    TOKEN_ARROW, // =>
    TOKEN_TERNARY, // ?
    TOKEN_NULISH, // ??

    _TOKEN_TYPE_ENUM_COUNT // This value is used to keep track of the number of
    // elements
} TokenType;

typedef struct {
    TokenType type;
    const char *lexeme_start;
    size_t lexeme_length;
    int line;
    int column;
} Token;

const char *token_type_to_string(TokenType type);

#endif // ESJS_CUSTOM_LEXER_TOKEN_H