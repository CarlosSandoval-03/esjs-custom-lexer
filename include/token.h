#pragma once

#include <stdio.h>

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_EOF,
    TOKEN_ERROR

    TOKEN_PLUS,             // +
    TOKEN_MINUS,            // -
    TOKEN_TIMES,            // *
    TOKEN_DIVIDE,           // /
    TOKEN_POWER,            // **
    TOKEN_MOD,              // %

    TOKEN_ASSIGN,           // =
    TOKEN_MOD_ASSIGN,       // %=
    TOKEN_DIV_ASSIGN,       // /=
    TOKEN_TIMES_ASSIGN,     // *=
    TOKEN_MINUS_ASSIGN,     // -=
    TOKEN_PLUS_ASSIGN,      // +=
    TOKEN_POWER_ASSIGN,     // **=

    TOKEN_EQUAL,            // ==
    TOKEN_STRICT_EQUAL,     // ===
    TOKEN_NOT_EQUAL,        // !=
    TOKEN_STRICT_NOT_EQUAL, // !==

    TOKEN_LESS,             // <
    TOKEN_LESS_EQUAL,       // <=
    TOKEN_GREATER,          // >
    TOKEN_GREATER_EQUAL,    // >=

    TOKEN_LOGICAL_AND,      // &&
    TOKEN_LOGICAL_OR,       // ||

    TOKEN_NOT,              // !
    TOKEN_BIT_AND,          // &
    TOKEN_BIT_OR,           // |
    TOKEN_BIT_XOR,          // ^
    TOKEN_BIT_NOT,          // ~

    TOKEN_SPREAD,           // ...
    TOKEN_PERIOD,           // .
    TOKEN_COMMA,            // ,
    TOKEN_SEMICOLON,        // ;
    TOKEN_COLON,            // :

    TOKEN_OPENING_KEY       // {
    TOKEN_CLOSING_KEY,      // }
    TOKEN_OPENING_BRA,      // [
    TOKEN_CLOSING_BRA,      // ]
    TOKEN_OPENING_PAR,      // (
    TOKEN_CLOSING_PAR,      // )

    TOKEN_INCREMENT,        // ++
    TOKEN_DECREMENT,        // --

    TOKEN_ARROW,            // =>
    TOKEN_TERNARY,          // ?
    TOKEN_NULISH,           // ??
} TokenType;

typedef struct {
    TokenType type;
    const char *lexeme_start;
    size_t lexeme_length;
    int line;
    int column;
} Token;