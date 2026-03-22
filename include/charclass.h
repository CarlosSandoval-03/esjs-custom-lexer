#ifndef ESJS_CUSTOM_LEXER_BUFFER_H
#define ESJS_CUSTOM_LEXER_BUFFER_H

typedef enum {
    CHAR_LETTER,
    CHAR_DIGIT,
    CHAR_UNDERSCORE,
    CHAR_WHITESPACE,
    CHAR_NEWLINE,

    CHAR_PLUS,        // +
    CHAR_MINUS,       // -
    CHAR_STAR,        // *
    CHAR_SLASH,       // /
    CHAR_PERCENT,     // %
    CHAR_EQUAL,       // =
    CHAR_BANG,        // !
    CHAR_LESS,        // <
    CHAR_GREATER,     // >
    CHAR_AMPERSAND,   // &
    CHAR_PIPE,        // |
    CHAR_CARET,       // ^
    CHAR_TILDE,       // ~
    CHAR_QUESTION,    // ?
    CHAR_DOT,         // .

    CHAR_COMMA,       // ,
    CHAR_SEMICOLON,   // ;
    CHAR_COLON,       // :
    CHAR_LBRACE,      // {
    CHAR_RBRACE,      // }
    CHAR_LBRACKET,    // [
    CHAR_RBRACKET,    // ]
    CHAR_LPAREN,      // (
    CHAR_RPAREN,      // )

    CHAR_DOUBLE_QUOTE, // "
    CHAR_SINGLE_QUOTE, // '
    CHAR_BACKTICK,     // `

    CHAR_EOF,
    CHAR_OTHER
} CharClass;

CharClass classify_char(int c);

#endif // ESJS_CUSTOM_LEXER_BUFFER_H