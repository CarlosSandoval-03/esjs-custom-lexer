/**
 * @file keywords.c
 * @brief Keyword lookup table construction and linear-search implementation.
 *
 * The reserved-word table is built at compile time from the ESJS_KEYWORDS
 * X-macro defined in keywords.h. The KEYWORD_LITERAL expansion produces an
 * array of null-terminated C string literals:
 *
 * @code
 *   #define KEYWORD_LITERAL(value) value,
 *   static const char *const KEYWORDS[] = { ESJS_KEYWORDS(KEYWORD_LITERAL) };
 *   #undef KEYWORD_LITERAL
 * @endcode
 *
 * Adding a new reserved word requires only a single change in the ESJS_KEYWORDS
 * macro in keywords.h; this file and any other consumer that uses the macro
 * will be updated automatically at the next compilation.
 *
 * Lookup complexity: O(n * k) where n is the number of keywords and k is the
 * average keyword length. For the current set size this is acceptable; a hash
 * table or trie could be substituted if profiling shows a bottleneck.
 */
#include "../include/keywords.h"

#include <string.h>

/**
 * @brief Expands ESJS_KEYWORDS into an array of string literal pointers.
 *
 * The X-macro pattern: each invocation of KEYWORD_LITERAL(value) emits
 * `value,` so the preprocessor builds the initializer list automatically.
 */
#define KEYWORD_LITERAL(value) value,
static const char *const KEYWORDS[] = {ESJS_KEYWORDS(KEYWORD_LITERAL)};
#undef KEYWORD_LITERAL

/**
 * @brief Performs a linear search for @p lexeme in the KEYWORDS table.
 *
 * Each candidate is compared by length first (cheap integer comparison) and
 * then by content via strncmp(). Because the lexeme is not necessarily
 * null-terminated, strlen() is used on the table entry (which is always a
 * null-terminated literal) and the comparison is bounded by @p length.
 *
 * @param lexeme Pointer to the first byte of the candidate. Need not be
 *               null-terminated.
 * @param length Length of the candidate in bytes.
 * @return       TOKEN_KEYWORD if a match is found; TOKEN_IDENTIFIER otherwise.
 */
TokenType keyword_lookup(const char *lexeme, const size_t length) {
  for (size_t i = 0; i < sizeof(KEYWORDS) / sizeof(KEYWORDS[0]); i++) {
    if (strlen(KEYWORDS[i]) == length &&
        strncmp(KEYWORDS[i], lexeme, length) == 0) {
      return TOKEN_KEYWORD;
    }
  }

  return TOKEN_IDENTIFIER;
}
