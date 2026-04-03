#ifndef ESJS_CUSTOM_LEXER_CHARCLASS_H
#define ESJS_CUSTOM_LEXER_CHARCLASS_H

// WARNING: Each value requires its own representation in charclass.c
typedef enum {
  CHAR_LETTER,
  CHAR_DIGIT,
  CHAR_UNDERSCORE,
  CHAR_WHITESPACE,
  CHAR_NEWLINE,
  CHAR_UTF8_CONTINUATION,  // UTF-8 multi-byte character continuation
                           // (0x80-0xFF)

  CHAR_PLUS,         // +
  CHAR_MINUS,        // -
  CHAR_STAR,         // *
  CHAR_SLASH,        // /
  CHAR_PERCENT,      // %
  CHAR_EQUAL,        // =
  CHAR_BANG,         // !
  CHAR_LESS,         // <
  CHAR_GREATER,      // >
  CHAR_AMPERSAND,    // &
  CHAR_PIPE,         // |
  CHAR_CARET,        // ^
  CHAR_TILDE,        // ~
  CHAR_QUESTION,     // ?
  CHAR_DOT,          // .
  CHAR_DOLLAR_SIGN,  // $

  CHAR_COMMA,      // ,
  CHAR_SEMICOLON,  // ;
  CHAR_COLON,      // :
  CHAR_LBRACE,     // {
  CHAR_RBRACE,     // }
  CHAR_LBRACKET,   // [
  CHAR_RBRACKET,   // ]
  CHAR_LPAREN,     // (
  CHAR_RPAREN,     // )

  CHAR_DOUBLE_QUOTE,  // "
  CHAR_SINGLE_QUOTE,  // '
  CHAR_BACKTICK,      // `
  CHAR_BACKSLASH,     // \\

  CHAR_EOF,
  CHAR_OTHER,

  _CHAR_ENUM_COUNT  // This value is used to keep track of the number of
                    // elements
} CharClass;

CharClass classify_char(int c);

/**
 * @brief Determines the UTF-8 character length in bytes from its first byte.
 * Returns 1 for ASCII (0x00-0x7F), 2 for 0xC0-0xDF, 3 for 0xE0-0xEF, 4 for
 * 0xF0-0xF7. Returns 1 for invalid leading bytes.
 */
int utf8_char_length(int first_byte);

#endif  // ESJS_CUSTOM_LEXER_CHARCLASS_H