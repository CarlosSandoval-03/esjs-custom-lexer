/**
 * @file charclass.h
 * @brief Character classification layer that abstracts raw byte values into
 *        the discrete input alphabet consumed by the DFA transition table.
 *
 * Rather than embedding concrete character comparisons inside the automaton,
 * the DFA operates on CharClass values. This keeps the transition table
 * compact and makes it straightforward to extend the alphabet (e.g. adding a
 * new operator character) without changing any DFA logic.
 *
 * UTF-8 multi-byte sequences are handled transparently: a leading byte
 * (0xC0-0xFF) is mapped to CHAR_LETTER so identifiers can contain non-ASCII
 * characters, while continuation bytes (0x80-0xBF) produce
 * CHAR_UTF8_CONTINUATION so the scanner can skip them without treating each
 * byte as an independent token character.
 *
 * @note Every CharClass value must have a corresponding case in charclass.c.
 *       Adding a new class without updating classify_char() leaves the new
 *       value unreachable and will silently break DFA rules that depend on it.
 */
#ifndef ESJS_CUSTOM_LEXER_CHARCLASS_H
#define ESJS_CUSTOM_LEXER_CHARCLASS_H

/**
 * @brief Discrete input-alphabet symbol fed into the DFA transition table.
 *
 * Each value represents an equivalence class of raw byte values that the DFA
 * treats identically. The enum is kept ordered from most-specific to
 * most-generic so that classify_char() can apply priority rules without
 * ambiguity (e.g. 'u' precedes other letters to enable Unicode-escape
 * lookahead).
 *
 * @warning Every value requires its own handling in charclass.c. The sentinel
 *          _CHAR_ENUM_COUNT must remain the last entry.
 */
typedef enum {
  CHAR_LETTER,    /**< ASCII alphabetic character [A-Za-z], excluding 'u'.  */
  CHAR_U_LOWER,   /**< Lowercase 'u', distinguished to recognize \uXXXX and
                       \u{XXXX} Unicode escape sequences in identifiers.    */
  CHAR_DIGIT,     /**< ASCII decimal digit [0-9].                           */
  CHAR_UNDERSCORE,/**< Underscore '_', valid identifier start and continue. */
  CHAR_WHITESPACE,/**< Horizontal whitespace: space, tab, CR, FF, VT.       */
  CHAR_NEWLINE,   /**< Line-feed '\n'; tracked separately for line counting. */
  CHAR_UTF8_CONTINUATION, /**< UTF-8 continuation byte (0x80-0xBF). These
                               bytes must be consumed without advancing the
                               logical column counter.                       */

  /* ---- Operator and punctuation characters ------------------------------ */
  CHAR_PLUS,         /**< `+`  */
  CHAR_MINUS,        /**< `-`  */
  CHAR_STAR,         /**< `*`  */
  CHAR_SLASH,        /**< `/`  */
  CHAR_PERCENT,      /**< `%`  */
  CHAR_EQUAL,        /**< `=`  */
  CHAR_BANG,         /**< `!`  */
  CHAR_LESS,         /**< `<`  */
  CHAR_GREATER,      /**< `>`  */
  CHAR_AMPERSAND,    /**< `&`  */
  CHAR_PIPE,         /**< `|`  */
  CHAR_CARET,        /**< `^`  */
  CHAR_TILDE,        /**< `~`  */
  CHAR_QUESTION,     /**< `?`  */
  CHAR_DOT,          /**< `.`  */
  CHAR_DOLLAR_SIGN,  /**< `$`  - valid identifier start character.         */

  /* ---- Delimiter characters --------------------------------------------- */
  CHAR_COMMA,      /**< `,`  */
  CHAR_SEMICOLON,  /**< `;`  */
  CHAR_COLON,      /**< `:`  */
  CHAR_LBRACE,     /**< `{`  - also used as delimiter in \u{XXXX} escapes. */
  CHAR_RBRACE,     /**< `}`  - closes \u{XXXX} Unicode escapes.            */
  CHAR_LBRACKET,   /**< `[`  - also opens regex character classes.         */
  CHAR_RBRACKET,   /**< `]`  - closes regex character classes.             */
  CHAR_LPAREN,     /**< `(`  */
  CHAR_RPAREN,     /**< `)`  */

  /* ---- String / regex delimiter characters ------------------------------ */
  CHAR_DOUBLE_QUOTE,  /**< `"` - opens/closes double-quoted string literals.*/
  CHAR_SINGLE_QUOTE,  /**< `'` - opens/closes single-quoted string literals.*/
  CHAR_BACKTICK,      /**< `` ` `` - opens/closes template string literals. */
  CHAR_BACKSLASH,     /**< `\` - escape sequence introducer in strings,
                           regex, and identifier Unicode escapes.            */

  /* ---- Catch-all / sentinel --------------------------------------------- */
  CHAR_EOF,   /**< End-of-file condition returned by buffer_get().          */
  CHAR_OTHER, /**< Any byte that does not match the classes above.          */

  /**
   * @internal Sentinel for table dimensioning. Must remain the last entry.
   */
  _CHAR_ENUM_COUNT
} CharClass;

/**
 * @brief Maps a raw byte (or EOF) to its CharClass equivalence class.
 *
 * UTF-8 handling:
 *  - Leading bytes 0xC0-0xFF are returned as CHAR_LETTER (identifier
 *    continuation; the leading byte starts an identifier character).
 *  - Continuation bytes 0x80-0xBF are returned as CHAR_UTF8_CONTINUATION.
 *
 * The character 'u' is returned as CHAR_U_LOWER (not CHAR_LETTER) so that
 * the DFA can distinguish the start of a \uXXXX escape sequence.
 *
 * @param c  Raw byte value as returned by fgetc() / buffer_get(), or EOF.
 * @return   The CharClass for @p c. Never returns a value outside the enum.
 */
CharClass classify_char(int c);

/**
 * @brief Determines the byte length of a UTF-8 encoded code point from its
 *        first byte.
 *
 * Uses the standard UTF-8 bit-prefix encoding to detect sequence width:
 *  - 0x00-0x7F : 1 byte  (ASCII)
 *  - 0xC0-0xDF : 2 bytes (110xxxxx leading byte)
 *  - 0xE0-0xEF : 3 bytes (1110xxxx leading byte)
 *  - 0xF0-0xF7 : 4 bytes (11110xxx leading byte)
 *
 * Invalid or continuation bytes (0x80-0xBF) are treated as length 1 to
 * allow the caller to recover gracefully.
 *
 * @param first_byte  The first byte of a UTF-8 sequence.
 * @return            The total number of bytes in the sequence (1-4).
 */
int utf8_char_length(int first_byte);

#endif  // ESJS_CUSTOM_LEXER_CHARCLASS_H
