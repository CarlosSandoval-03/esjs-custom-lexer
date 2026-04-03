#include "../include/scanner.h"

/**
 * @brief Returns the DFA entry state for a given scanner context.
 */
LexerState scanner_entry_state_for_context(const ScannerContext context) {
  return context == SCANNER_CONTEXT_EXPECT_REGEX ? dfa_regex_entry_state()
                                                 : STATE_START;
}

/**
 * @brief Runs the DFA from the current position and consumes the longest
 * accepting match.
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

    // If this was the start of a UTF-8 sequence, skip remaining bytes
    // UTF-8 leading bytes are classified as CHAR_LETTER
    if (input_class == CHAR_LETTER && (unsigned char)c >= 0xC0) {
      // This is a UTF-8 leading byte; skip its continuation bytes
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
 * @brief Convenience wrapper that combines regex-context selection and
 * maximal munch.
 */
int scanner_match_longest_after(Scanner *sc, ScannerContext *used_context,
                                size_t *consumed_length) {
  const ScannerContext default_context = SCANNER_CONTEXT_DEFAULT;

  // When the current character is not '/' there is no ambiguity: use the
  // default context directly.
  if (scanner_peek(sc, 0) != '/') {
    if (used_context != NULL) {
      *used_context = default_context;
    }
    return scanner_match_longest(
        sc, scanner_entry_state_for_context(default_context), consumed_length);
  }

  // Current character is '/': always probe both the regex and the default
  // interpretations and apply the longest-match (maximal munch) principle.
  Scanner default_probe = *sc;
  Scanner regex_probe = *sc;
  size_t regex_length = 0;
  size_t default_length = 0;
  int regex_ok = 0;

  scanner_next(&regex_probe);
  regex_ok = scanner_match_longest(&regex_probe, dfa_regex_entry_state(),
                                   &regex_length);
  if (regex_ok) {
    regex_length += 1;  // account for the leading '/'
  }

  const int default_ok = scanner_match_longest(
      &default_probe, scanner_entry_state_for_context(default_context),
      &default_length);

  if (!regex_ok && !default_ok) {
    return 0;
  }

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

void scanner_init(Scanner *sc, Buffer *buffer) {
  sc->buffer = buffer;
  sc->pos = 0;
  sc->line = 1;
  sc->column = 1;
}

int scanner_peek(const Scanner *sc, const size_t k) {
  return buffer_get(sc->buffer, sc->pos + k);
}

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
    // Only increment column for ASCII characters and UTF-8 leading bytes
    // Skip UTF-8 continuation bytes (0x80-0xBF)
    const unsigned char ub = (unsigned char)c;
    if (ub < 0x80 || ub >= 0xC0) {
      sc->column++;
    }
  }
  return c;
}

void scanner_advance(Scanner *sc, size_t n) {
  while (n-- > 0) {
    if (scanner_next(sc) == EOF) {
      break;
    }
  }
}

size_t scanner_position(const Scanner *sc) { return sc->pos; }

int scanner_line(const Scanner *sc) { return sc->line; }

int scanner_column(const Scanner *sc) { return sc->column; }