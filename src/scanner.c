/**
 * @file scanner.c
 * @brief Scanner implementation: DFA execution engine and cursor management.
 *
 * The scanner provides three layers of functionality:
 *
 *  1. Character-level access (scanner_peek, scanner_next, scanner_advance):
 *     Thin wrappers over the Buffer that update line/column tracking and
 *     handle UTF-8 continuation bytes correctly.
 *
 *  2. Maximal-munch matching (scanner_match_longest):
 *     Drives the DFA from a caller-supplied entry state, recording the
 *     furthest accepting position found. Commits that position to the
 *     scanner cursor if a match exists.
 *
 *  3. Ambiguity resolution (scanner_match_longest_after):
 *     When the current character is `/`, both the division-operator path and
 *     the regex path are probed independently using shallow Scanner copies
 *     (struct value semantics). The longer match wins and the real scanner
 *     cursor is updated accordingly.
 */
#include "../include/scanner.h"

/**
 * @brief Maps a ScannerContext hint to the corresponding DFA entry state.
 *
 * @param context SCANNER_CONTEXT_DEFAULT or SCANNER_CONTEXT_EXPECT_REGEX.
 * @return        STATE_START for the default context; the regex entry state
 *                (STATE_REGEX_BODY) for the regex context.
 */
LexerState scanner_entry_state_for_context(const ScannerContext context) {
  return context == SCANNER_CONTEXT_EXPECT_REGEX ? dfa_regex_entry_state()
                                                 : STATE_START;
}

/**
 * @brief Drives the DFA from @p entry_state to find the longest accepting
 *        prefix at the current scanner position (maximal-munch rule).
 *
 * Algorithm:
 *   offset     <- 0             (lookahead distance from sc->pos)
 *   best_length <- 0            (length of best accepting match so far)
 *   state      <- entry_state
 *
 *   loop:
 *     c          <- peek(sc, offset)
 *     next_state <- dfa_next_state(state, classify_char(c))
 *     if next_state == STATE_ERROR: break
 *     offset++
 *     if c was a UTF-8 leading byte: skip its continuation bytes (offset += utf8_len - 1)
 *     state <- next_state
 *     if dfa_is_accepting(state): best_length <- offset
 *
 *   if best_length > 0: advance scanner by best_length; report success
 *   else:               report failure (no match)
 *
 * UTF-8 handling: leading bytes (>= 0xC0) are classified as CHAR_LETTER, which
 * causes one DFA step. The remaining continuation bytes are skipped by
 * incrementing @p offset without an additional DFA step. This ensures that a
 * multi-byte code point consumes exactly one logical DFA transition while
 * correctly accounting for all physical bytes.
 *
 * @param sc              Scanner to advance on a successful match.
 * @param entry_state     DFA starting state.
 * @param consumed_length Set to the number of bytes consumed on success. May
 *                        be NULL.
 * @return                Non-zero on success; 0 if no accepting match found.
 */
int scanner_match_longest(Scanner *sc, const LexerState entry_state,
                          size_t *consumed_length) {
  size_t offset = 0;
  size_t best_length = 0;
  LexerState state = entry_state;

  for (;;) {
    const int c = scanner_peek(sc, offset);
    const CharClass input_class = classify_char(c);
    const LexerState next_state = dfa_next_state(state, input_class);

    if (next_state == STATE_ERROR) {
      break;
    }

    offset++;

    /* UTF-8 multi-byte sequence: the leading byte triggered the DFA step;
     * skip the remaining continuation bytes without additional DFA steps. */
    if (input_class == CHAR_LETTER && (unsigned char)c >= 0xC0) {
      const int utf8_len = utf8_char_length(c);
      for (int i = 1; i < utf8_len; i++) {
        offset++;
      }
    }

    state = next_state;

    if (dfa_is_accepting(state)) {
      best_length = offset;
    }
  }

  if (best_length == 0) {
    return 0;
  }

  scanner_advance(sc, best_length);
  if (consumed_length != NULL) {
    *consumed_length = best_length;
  }
  return 1;
}

/**
 * @brief Combines context selection and maximal-munch matching, resolving
 *        the `/` ambiguity between division operator and regex literal.
 *
 * When the current character is not `/`:
 *   Delegates immediately to scanner_match_longest() with the default context
 *   (STATE_START entry). No ambiguity exists.
 *
 * When the current character is `/`:
 *   Two independent Scanner probes are created by value copy:
 *     - Regex probe:   manually consumes the leading `/`, then runs the DFA
 *                      from STATE_REGEX_BODY. The total regex length includes
 *                      the consumed leading slash.
 *     - Default probe: runs the DFA from STATE_START over the `/` character,
 *                      which can match `/` (TOKEN_DIVIDE) or `/=`
 *                      (TOKEN_DIV_ASSIGN).
 *
 *   The probe with the longer successful match wins. If both match with equal
 *   length, the default (division) interpretation is preferred. The real
 *   scanner @p sc is updated to the winner's cursor state.
 *
 * @param sc              Scanner to advance on success.
 * @param used_context    Output: winning context. May be NULL.
 * @param consumed_length Output: bytes consumed. May be NULL.
 * @return                Non-zero on success; 0 if neither path matched.
 */
int scanner_match_longest_after(Scanner *sc, ScannerContext *used_context,
                                size_t *consumed_length) {
  const ScannerContext default_context = SCANNER_CONTEXT_DEFAULT;

  /* No ambiguity: current character is not '/'. */
  if (scanner_peek(sc, 0) != '/') {
    if (used_context != NULL) {
      *used_context = default_context;
    }
    return scanner_match_longest(
        sc, scanner_entry_state_for_context(default_context), consumed_length);
  }

  /* Current character is '/': probe both interpretations in parallel using
   * independent cursor copies. */
  Scanner default_probe = *sc;
  Scanner regex_probe = *sc;
  size_t regex_length = 0;
  size_t default_length = 0;
  int regex_ok = 0;

  /* Regex probe: consume the opening '/' manually, then run from the regex
   * entry state. The +1 accounts for the consumed leading slash. */
  scanner_next(&regex_probe);
  regex_ok = scanner_match_longest(&regex_probe, dfa_regex_entry_state(),
                                   &regex_length);
  if (regex_ok) {
    regex_length += 1;
  }

  /* Default probe: run from STATE_START (recognizes '/' or '/='). */
  const int default_ok = scanner_match_longest(
      &default_probe, scanner_entry_state_for_context(default_context),
      &default_length);

  if (!regex_ok && !default_ok) {
    return 0;
  }

  /* Prefer the longer match; on a tie prefer the default (division) path. */
  if (regex_ok && (!default_ok || regex_length > default_length)) {
    *sc = regex_probe;
    if (used_context != NULL) {
      *used_context = SCANNER_CONTEXT_EXPECT_REGEX;
    }
    if (consumed_length != NULL) {
      *consumed_length = regex_length;
    }
    return 1;
  }

  *sc = default_probe;
  if (used_context != NULL) {
    *used_context = default_context;
  }
  if (consumed_length != NULL) {
    *consumed_length = default_length;
  }
  return 1;
}

/**
 * @brief Initializes the scanner to position 0 in the given buffer.
 *
 * @param sc     Scanner to initialize.
 * @param buffer Backing buffer. Must have been initialized via buffer_init().
 */
void scanner_init(Scanner *sc, Buffer *buffer) {
  sc->buffer = buffer;
  sc->pos = 0;
  sc->line = 1;
  sc->column = 1;
}

/**
 * @brief Returns the byte at @p k positions ahead of the current cursor,
 *        without advancing the scanner.
 *
 * @param sc Const scanner (position unchanged).
 * @param k  Lookahead offset in bytes (0 = next byte to consume).
 * @return   The byte at sc->pos + k, or EOF.
 */
int scanner_peek(const Scanner *sc, const size_t k) {
  return buffer_get(sc->buffer, sc->pos + k);
}

/**
 * @brief Consumes and returns the byte at the current cursor position.
 *
 * Advances @c sc->pos by 1. Updates source location:
 *  - Newline: increments @c line, resets @c column to 1.
 *  - ASCII or UTF-8 leading byte (>= 0xC0): increments @c column.
 *  - UTF-8 continuation byte (0x80-0xBF): @c column is unchanged, because
 *    continuation bytes are not code-point boundaries.
 *
 * @param sc Scanner to advance.
 * @return   The consumed byte (as int), or EOF.
 */
int scanner_next(Scanner *sc) {
  const int c = buffer_get(sc->buffer, sc->pos);
  if (c == EOF) {
    return EOF;
  }

  sc->pos++;
  if (c == '\n') {
    sc->line++;
    sc->column = 1;
  } else {
    /* Increment the column counter only for code-point-starting bytes. */
    const unsigned char ub = (unsigned char)c;
    if (ub < 0x80 || ub >= 0xC0) {
      sc->column++;
    }
  }
  return c;
}

/**
 * @brief Advances the scanner by @p n bytes by calling scanner_next() @p n
 *        times.
 *
 * Stops early if EOF is reached before @p n bytes are consumed.
 *
 * @param sc Scanner to advance.
 * @param n  Number of bytes to consume.
 */
void scanner_advance(Scanner *sc, size_t n) {
  while (n-- > 0) {
    if (scanner_next(sc) == EOF) {
      break;
    }
  }
}

/** @return Current byte offset (sc->pos). */
size_t scanner_position(const Scanner *sc) { return sc->pos; }

/** @return Current 1-based line number (sc->line). */
int scanner_line(const Scanner *sc) { return sc->line; }

/** @return Current 1-based column number (sc->column). */
int scanner_column(const Scanner *sc) { return sc->column; }
