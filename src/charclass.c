#include "../include/charclass.h"

#include <ctype.h>
#include <stdio.h>

CharClass classify_char(const int c) {
  if (c == EOF) return CHAR_EOF;
  if (c == '\n') return CHAR_NEWLINE;
  if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v')
    return CHAR_WHITESPACE;

  // Handle UTF-8 multi-byte sequences
  // Leading bytes (0xC0-0xEF) are classified as LETTER
  // Continuation bytes (0x80-0xBF) are classified as UTF8_CONTINUATION
  // This allows us to skip continuation bytes in the scanner
  const unsigned char ub = (unsigned char)c;
  if (ub >= 0xC0) {
    // UTF-8 leading byte - treat as letter
    return CHAR_LETTER;
  }
  if (ub >= 0x80) {
    // UTF-8 continuation byte (0x80-0xBF)
    return CHAR_UTF8_CONTINUATION;
  }

  if (isalpha((unsigned char)c)) return CHAR_LETTER;
  if (isdigit((unsigned char)c)) return CHAR_DIGIT;

  switch (c) {
    case '_':
      return CHAR_UNDERSCORE;

    case '+':
      return CHAR_PLUS;
    case '-':
      return CHAR_MINUS;
    case '*':
      return CHAR_STAR;
    case '/':
      return CHAR_SLASH;
    case '%':
      return CHAR_PERCENT;
    case '=':
      return CHAR_EQUAL;
    case '!':
      return CHAR_BANG;
    case '<':
      return CHAR_LESS;
    case '>':
      return CHAR_GREATER;
    case '&':
      return CHAR_AMPERSAND;
    case '|':
      return CHAR_PIPE;
    case '^':
      return CHAR_CARET;
    case '~':
      return CHAR_TILDE;
    case '?':
      return CHAR_QUESTION;
    case '.':
      return CHAR_DOT;
    case '$':
      return CHAR_DOLLAR_SIGN;

    case ',':
      return CHAR_COMMA;
    case ';':
      return CHAR_SEMICOLON;
    case ':':
      return CHAR_COLON;
    case '{':
      return CHAR_LBRACE;
    case '}':
      return CHAR_RBRACE;
    case '[':
      return CHAR_LBRACKET;
    case ']':
      return CHAR_RBRACKET;
    case '(':
      return CHAR_LPAREN;
    case ')':
      return CHAR_RPAREN;

    case '"':
      return CHAR_DOUBLE_QUOTE;
    case '\'':
      return CHAR_SINGLE_QUOTE;
    case '`':
      return CHAR_BACKTICK;
    case '\\':
      return CHAR_BACKSLASH;

    default:
      return CHAR_OTHER;
  }
}

int utf8_char_length(const int first_byte) {
  const unsigned char b = (unsigned char)first_byte;

  // ASCII character (0x00-0x7F)
  if (b <= 0x7F) return 1;

  // 2-byte UTF-8 sequence (110xxxxx)
  if ((b & 0xE0) == 0xC0) return 2;

  // 3-byte UTF-8 sequence (1110xxxx)
  if ((b & 0xF0) == 0xE0) return 3;

  // 4-byte UTF-8 sequence (11110xxx)
  if ((b & 0xF8) == 0xF0) return 4;

  // Invalid or continuation byte - treat as single byte
  return 1;
}