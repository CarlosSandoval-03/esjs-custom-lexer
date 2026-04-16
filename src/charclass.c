/**
 * @file charclass.c
 * @brief Implementation of the character classification layer.
 *
 * classify_char() maps every possible byte value (plus EOF) to a CharClass
 * equivalence class. The DFA transition table is indexed by these classes, so
 * the automaton never deals with raw byte values directly.
 *
 * Priority rules applied in classify_char():
 *  1. EOF is detected first.
 *  2. Newline is detected before generic whitespace (it needs separate
 *     handling for line counting).
 *  3. UTF-8 leading bytes (>= 0xC0) are detected before isalpha() so that
 *     non-ASCII identifier characters are recognized as CHAR_LETTER.
 *  4. UTF-8 continuation bytes (0x80-0xBF) are classified as
 *     CHAR_UTF8_CONTINUATION so the scanner can skip them without treating
 *     each byte as an independent DFA symbol.
 *  5. The lowercase letter 'u' is classified as CHAR_U_LOWER (before the
 *     generic CHAR_LETTER catch) to enable the DFA to enter the Unicode
 *     escape sequence sub-automaton on `\u`.
 */
#include "../include/charclass.h"

#include <ctype.h>
#include <stdio.h>

/**
 * @brief Maps a raw byte (or EOF) to its CharClass equivalence class.
 *
 * See charclass.h for the full documentation of each CharClass value and the
 * priority rules that govern classification order.
 *
 * @param c  Raw byte from the input stream, or EOF (-1).
 * @return   Corresponding CharClass value.
 */
CharClass classify_char(const int c) {
  if (c == EOF) return CHAR_EOF;
  if (c == '\n') return CHAR_NEWLINE;

  /* Horizontal whitespace: space, horizontal tab, carriage return, form feed,
   * vertical tab. These share the same DFA treatment (skip at start state). */
  if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v')
    return CHAR_WHITESPACE;

  /* UTF-8 multi-byte sequence classification.
   *
   * The UTF-8 encoding uses the high bits of the first byte to indicate the
   * sequence length:
   *   0xxxxxxx          -> ASCII (1 byte)
   *   10xxxxxx          -> continuation byte (must not appear as a leading byte)
   *   110xxxxx 10xxxxxx -> 2-byte sequence
   *   1110xxxx ...      -> 3-byte sequence
   *   11110xxx ...      -> 4-byte sequence
   *
   * Leading bytes (0xC0-0xFF, i.e. >= 0xC0) are classified as CHAR_LETTER
   * because any non-ASCII code point that is a valid identifier character
   * (letters from accented Latin, Greek, etc.) starts with such a byte.
   *
   * Continuation bytes (0x80-0xBF) carry no standalone semantic meaning and
   * are classified as CHAR_UTF8_CONTINUATION so the scanner knows to consume
   * them without an additional DFA step.
   */
  const unsigned char ub = (unsigned char)c;
  if (ub >= 0xC0) {
    return CHAR_LETTER;
  }
  if (ub >= 0x80) {
    return CHAR_UTF8_CONTINUATION;
  }

  /* Distinguish 'u' from other alphabetic characters to enable recognition
   * of \uXXXX and \u{XXXX} Unicode escape sequences inside identifiers. */
  if (c == 'u') return CHAR_U_LOWER;

  if (isalpha((unsigned char)c)) return CHAR_LETTER;
  if (isdigit((unsigned char)c)) return CHAR_DIGIT;

  switch (c) {
    case '_':  return CHAR_UNDERSCORE;

    /* Arithmetic and bitwise operators */
    case '+':  return CHAR_PLUS;
    case '-':  return CHAR_MINUS;
    case '*':  return CHAR_STAR;
    case '/':  return CHAR_SLASH;
    case '%':  return CHAR_PERCENT;
    case '=':  return CHAR_EQUAL;
    case '!':  return CHAR_BANG;
    case '<':  return CHAR_LESS;
    case '>':  return CHAR_GREATER;
    case '&':  return CHAR_AMPERSAND;
    case '|':  return CHAR_PIPE;
    case '^':  return CHAR_CARET;
    case '~':  return CHAR_TILDE;
    case '?':  return CHAR_QUESTION;
    case '.':  return CHAR_DOT;
    case '$':  return CHAR_DOLLAR_SIGN;

    /* Punctuation and grouping */
    case ',':  return CHAR_COMMA;
    case ';':  return CHAR_SEMICOLON;
    case ':':  return CHAR_COLON;
    case '{':  return CHAR_LBRACE;
    case '}':  return CHAR_RBRACE;
    case '[':  return CHAR_LBRACKET;
    case ']':  return CHAR_RBRACKET;
    case '(':  return CHAR_LPAREN;
    case ')':  return CHAR_RPAREN;

    /* String and escape delimiters */
    case '"':  return CHAR_DOUBLE_QUOTE;
    case '\'': return CHAR_SINGLE_QUOTE;
    case '`':  return CHAR_BACKTICK;
    case '\\': return CHAR_BACKSLASH;

    default:   return CHAR_OTHER;
  }
}

/**
 * @brief Returns the byte length of a UTF-8 encoded code point given its
 *        first byte.
 *
 * The function uses bitwise masks to inspect the high-order bits, following
 * the standard UTF-8 encoding scheme:
 *  - 0xxxxxxx (0x00-0x7F) -> 1 byte (ASCII)
 *  - 110xxxxx (0xC0-0xDF) -> 2 bytes
 *  - 1110xxxx (0xE0-0xEF) -> 3 bytes
 *  - 11110xxx (0xF0-0xF7) -> 4 bytes
 *
 * Invalid bytes and continuation bytes (0x80-0xBF) return 1 to allow the
 * caller to advance past them without entering an infinite loop.
 *
 * @param first_byte The first (or only) byte of a UTF-8 sequence.
 * @return           Byte width of the sequence: 1, 2, 3, or 4.
 */
int utf8_char_length(const int first_byte) {
  const unsigned char b = (unsigned char)first_byte;

  if (b <= 0x7F) return 1;          /* ASCII (0xxxxxxx) */
  if ((b & 0xE0) == 0xC0) return 2; /* 2-byte sequence (110xxxxx) */
  if ((b & 0xF0) == 0xE0) return 3; /* 3-byte sequence (1110xxxx) */
  if ((b & 0xF8) == 0xF0) return 4; /* 4-byte sequence (11110xxx) */

  /* Continuation byte or invalid leading byte: treat as a single byte so
   * the scanner can recover and continue processing. */
  return 1;
}
