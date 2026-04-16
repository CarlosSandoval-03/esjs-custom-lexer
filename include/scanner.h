/**
 * @file scanner.h
 * @brief Low-level DFA runner and position tracker that sits between the
 *        Buffer and the Lexer.
 *
 * The Scanner maintains a cursor (byte position, line, column) over the
 * Buffer and drives the DFA to consume the longest matching lexeme from the
 * current position. It provides:
 *
 *  - Character-level access: scanner_peek() and scanner_next().
 *  - Bulk advance: scanner_advance().
 *  - Maximal-munch matching: scanner_match_longest() and the higher-level
 *    scanner_match_longest_after() which handles the regex/division
 *    ambiguity.
 *
 * Context handling:
 *  The `/` character is ambiguous: it can be the start of a division
 *  operator (`/`, `/=`) or a regex literal (`/pattern/flags`). The Scanner
 *  resolves this by probing both interpretations when the current character
 *  is `/` and keeping whichever produces the longer match (maximal-munch
 *  principle). The winning context is reported back to the Lexer so it can
 *  assign the correct token type.
 *
 * UTF-8 awareness:
 *  scanner_next() counts only ASCII bytes and UTF-8 leading bytes toward the
 *  column counter, skipping continuation bytes (0x80-0xBF), so the column
 *  number accurately reflects code-point positions rather than byte offsets.
 */
#ifndef ESJS_CUSTOM_LEXER_SCANNER_H
#define ESJS_CUSTOM_LEXER_SCANNER_H

#include "buffer.h"
#include "dfa.h"
#include "token.h"

/**
 * @brief Hint passed to the scanner to bias the DFA entry point.
 *
 * SCANNER_CONTEXT_DEFAULT uses STATE_START and recognizes division operators.
 * SCANNER_CONTEXT_EXPECT_REGEX uses STATE_REGEX_BODY and recognizes regex
 * literals. The context is chosen externally by scanner_match_longest_after()
 * based on which interpretation yields the longer match.
 */
typedef enum {
  SCANNER_CONTEXT_DEFAULT,      /**< Normal scanning; `/` is a division op. */
  SCANNER_CONTEXT_EXPECT_REGEX  /**< Regex context; `/` opens a regex body. */
} ScannerContext;

/**
 * @brief Cursor state over the input Buffer.
 *
 * The Scanner owns no memory itself; it holds a pointer to a Buffer and
 * tracks the current read position together with the associated source
 * coordinates. Copying a Scanner (struct assignment) creates an independent
 * cursor that can be advanced separately — this is exploited by
 * scanner_match_longest_after() to probe both the default and regex
 * interpretations without consuming input until the winner is chosen.
 */
typedef struct {
  Buffer *buffer; /**< Input buffer being scanned. Not owned by the Scanner. */
  size_t  pos;    /**< Next byte index to read from Buffer::data.            */
  int     line;   /**< Current 1-based line number.                          */
  int     column; /**< Current 1-based column number (code-point count).     */
} Scanner;

/**
 * @brief Initializes a Scanner to position 0 in @p buffer.
 *
 * Line and column are both set to 1 (standard source-coordinate convention).
 *
 * @param sc     Scanner to initialize. Must not be NULL.
 * @param buffer Buffer to scan. Must remain live for the Scanner's lifetime.
 */
void scanner_init(Scanner *sc, Buffer *buffer);

/**
 * @brief Non-destructively reads the byte at @p k positions ahead of the
 *        current position.
 *
 * @c scanner_peek(sc, 0) returns the next byte to be consumed.
 * Does not advance @c pos, @c line, or @c column.
 *
 * @param sc Const scanner (position is unchanged).
 * @param k  Look-ahead distance in bytes (0 = current byte).
 * @return   The byte at position @c sc->pos + k, or EOF.
 */
int scanner_peek(const Scanner *sc, size_t k);

/**
 * @brief Consumes and returns the current byte, advancing the cursor.
 *
 * Updates @c pos by 1. Updates @c line and @c column according to the
 * consumed byte:
 *  - `\n` increments @c line and resets @c column to 1.
 *  - ASCII bytes and UTF-8 leading bytes (>= 0xC0) increment @c column.
 *  - UTF-8 continuation bytes (0x80-0xBF) do not increment @c column.
 *
 * @param sc Scanner to advance. Must not be NULL.
 * @return   The consumed byte, or EOF if already at end-of-stream.
 */
int scanner_next(Scanner *sc);

/**
 * @brief Advances the scanner by exactly @p n bytes, consuming each one.
 *
 * Stops early if EOF is reached before @p n bytes have been consumed.
 *
 * @param sc Scanner to advance. Must not be NULL.
 * @param n  Number of bytes to consume.
 */
void scanner_advance(Scanner *sc, size_t n);

/**
 * @brief Maps a ScannerContext to the corresponding DFA entry state.
 *
 * @param context Scanner context hint.
 * @return        STATE_START for SCANNER_CONTEXT_DEFAULT, or the regex entry
 *                state returned by dfa_regex_entry_state() for
 *                SCANNER_CONTEXT_EXPECT_REGEX.
 */
LexerState scanner_entry_state_for_context(ScannerContext context);

/**
 * @brief Drives the DFA from @p entry_state and consumes the longest
 *        accepting prefix starting at the current scanner position.
 *
 * Implements the maximal-munch rule: the function runs the DFA one character
 * at a time, recording the length of the input at each accepting state. When
 * the DFA reaches STATE_ERROR or the end of available input, it commits the
 * last recorded accepting length and advances the scanner by that amount.
 *
 * UTF-8 multi-byte sequences are consumed atomically: when a leading byte
 * (>= 0xC0) is encountered the remaining continuation bytes are skipped
 * without an additional DFA step.
 *
 * @param sc              Scanner to advance on success. Must not be NULL.
 * @param entry_state     DFA state to start from.
 * @param consumed_length Output: number of bytes consumed. May be NULL.
 * @return                Non-zero if at least one byte was consumed and the
 *                        match ended in an accepting state; 0 otherwise.
 */
int scanner_match_longest(Scanner *sc, LexerState entry_state,
                          size_t *consumed_length);

/**
 * @brief Resolves the regex/division ambiguity and returns the longest match.
 *
 * When the current character is not `/`, delegates directly to
 * scanner_match_longest() with SCANNER_CONTEXT_DEFAULT.
 *
 * When the current character is `/`, both interpretations are probed using
 * independent Scanner copies so that neither path consumes input until the
 * winner is known:
 *  1. The regex path consumes the leading `/` manually, then runs the DFA
 *     from the regex entry state. The total regex length includes the leading
 *     slash.
 *  2. The default path runs the DFA from STATE_START over the `/`.
 * The longer of the two matches wins (maximal munch). On a tie the default
 * (division) interpretation is preferred.
 *
 * @param sc              Scanner to advance on success. Must not be NULL.
 * @param used_context    Output: context that produced the winning match.
 *                        May be NULL.
 * @param consumed_length Output: number of bytes consumed. May be NULL.
 * @return                Non-zero if a match was found; 0 on failure.
 */
int scanner_match_longest_after(Scanner *sc, ScannerContext *used_context,
                                size_t *consumed_length);

/**
 * @brief Returns the current byte offset in the Buffer.
 * @param sc Const scanner.
 * @return   Current value of @c sc->pos.
 */
size_t scanner_position(const Scanner *sc);

/**
 * @brief Returns the current 1-based line number.
 * @param sc Const scanner.
 * @return   Current value of @c sc->line.
 */
int scanner_line(const Scanner *sc);

/**
 * @brief Returns the current 1-based column number.
 * @param sc Const scanner.
 * @return   Current value of @c sc->column.
 */
int scanner_column(const Scanner *sc);

#endif  // ESJS_CUSTOM_LEXER_SCANNER_H
