#ifndef ESJS_CUSTOM_LEXER_SCANNER_H
#define ESJS_CUSTOM_LEXER_SCANNER_H

#include "buffer.h"
#include "dfa.h"
#include "token.h"

typedef enum {
    SCANNER_CONTEXT_DEFAULT,
    SCANNER_CONTEXT_EXPECT_REGEX
} ScannerContext;

typedef struct {
    Buffer *buffer;
    size_t pos;
    int line;
    int column;
} Scanner;

void scanner_init(Scanner * sc, Buffer * buffer);

int scanner_peek(const Scanner *sc, size_t k);

int scanner_next(Scanner *sc);

void scanner_advance(Scanner *sc, size_t n);

int scanner_match(Scanner *sc, char expected);

/**
 * @brief Returns the DFA entry state for the requested scanner context.
 *
 * @param context Scanner context hint.
 * @return LexerState Entry state for token scanning.
 */
LexerState scanner_entry_state_for_context(ScannerContext context);

/**
 * @brief Decides whether a regex literal is allowed after a previous token.
 *
 * This helper is useful for JavaScript-like lexing where '/' can mean either a
 * division operator or the start of a regex literal.
 *
 * @param previous_type Previously emitted token type.
 * @return int Non-zero when a regex literal is allowed.
 */
int scanner_allows_regex_after(TokenType previous_type);

/**
 * @brief Chooses the scanner context after a token has been emitted.
 *
 * @param previous_type Previously emitted token type.
 * @return ScannerContext Regex context when a regex literal is valid, default
 * otherwise.
 */
ScannerContext scanner_context_after(TokenType previous_type);

/**
 * @brief Runs the DFA from the current position and consumes the longest
 * matching lexeme.
 *
 * The scanner remembers the furthest accepting state, which implements the
 * maximal-munch rule.
 *
 * @param sc Scanner to advance.
 * @param entry_state DFA state used as the starting point.
 * @param consumed_length Output length in bytes of the matched lexeme.
 * @return int Non-zero when a match was found and consumed.
 */
int scanner_match_longest(Scanner *sc, LexerState entry_state,
                          size_t *consumed_length);

/**
 * @brief Convenience wrapper that combines context selection and maximal munch.
 *
 * When a regex literal is allowed after the previous token, this helper probes
 * both the regex path and the default path and keeps the longer match.
 *
 * @param sc Scanner to advance.
 * @param previous_type Previously emitted token type.
 * @param used_context Output context used for the winning match.
 * @param consumed_length Output length in bytes of the matched lexeme.
 * @return int Non-zero when a match was found and consumed.
 */
int scanner_match_longest_after(Scanner *sc, TokenType previous_type,
                                ScannerContext *used_context,
                                size_t *consumed_length);

size_t scanner_position(const Scanner *sc);

int scanner_line(const Scanner *sc);

int scanner_column(const Scanner *sc);

#endif // ESJS_CUSTOM_LEXER_SCANNER_H