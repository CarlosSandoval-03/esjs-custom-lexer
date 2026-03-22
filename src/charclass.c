#include <stdio.h>
#include <ctype.h>
#include "../include/charclass.h"


CharClass classify_char(const int c) {
    if (c == EOF) return CHAR_EOF;
    if (c == '\n') return CHAR_NEWLINE;
    if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v') return CHAR_WHITESPACE;

    if (isalpha((unsigned char)c)) return CHAR_LETTER;
    if (isdigit((unsigned char)c)) return CHAR_DIGIT;

    switch (c) {
        case '_': return CHAR_UNDERSCORE;

        case '+': return CHAR_PLUS;
        case '-': return CHAR_MINUS;
        case '*': return CHAR_STAR;
        case '/': return CHAR_SLASH;
        case '%': return CHAR_PERCENT;
        case '=': return CHAR_EQUAL;
        case '!': return CHAR_BANG;
        case '<': return CHAR_LESS;
        case '>': return CHAR_GREATER;
        case '&': return CHAR_AMPERSAND;
        case '|': return CHAR_PIPE;
        case '^': return CHAR_CARET;
        case '~': return CHAR_TILDE;
        case '?': return CHAR_QUESTION;
        case '.': return CHAR_DOT;

        case ',': return CHAR_COMMA;
        case ';': return CHAR_SEMICOLON;
        case ':': return CHAR_COLON;
        case '{': return CHAR_LBRACE;
        case '}': return CHAR_RBRACE;
        case '[': return CHAR_LBRACKET;
        case ']': return CHAR_RBRACKET;
        case '(': return CHAR_LPAREN;
        case ')': return CHAR_RPAREN;

        case '"': return CHAR_DOUBLE_QUOTE;
        case '\'': return CHAR_SINGLE_QUOTE;
        case '`': return CHAR_BACKTICK;

        default:
            return CHAR_OTHER;
    }
}
