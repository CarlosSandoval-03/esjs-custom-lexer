#include "../include/scanner.h"

/**
 * @brief Returns true when the previous token can legally precede a regex
 * literal.
 */
int scanner_allows_regex_after(const TokenType previous_type) {
    switch (previous_type) {
        case TOKEN_EOF:
        case TOKEN_ERROR:
        case TOKEN_KEYWORD:
        case TOKEN_ASSIGN:
        case TOKEN_CONST:
        case TOKEN_LET:
        case TOKEN_VAR:
        case TOKEN_FUNCTION:
        case TOKEN_RETURN:
        case TOKEN_IF:
        case TOKEN_ELSE:
        case TOKEN_FOR:
        case TOKEN_WHILE:
        case TOKEN_DO:
        case TOKEN_SWITCH:
        case TOKEN_CASE:
        case TOKEN_DEFAULT:
        case TOKEN_BREAK:
        case TOKEN_CONTINUE:
        case TOKEN_TRY:
        case TOKEN_CATCH:
        case TOKEN_FINALLY:
        case TOKEN_THROW:
        case TOKEN_CLASS:
        case TOKEN_EXTENDS:
        case TOKEN_NEW:
        case TOKEN_THIS:
        case TOKEN_SUPER:
        case TOKEN_IMPORT:
        case TOKEN_EXPORT:
        case TOKEN_FROM:
        case TOKEN_AS:
        case TOKEN_ASYNC:
        case TOKEN_AWAIT:
        case TOKEN_YIELD:
        case TOKEN_IN:
        case TOKEN_OF:
        case TOKEN_INSTANCEOF:
        case TOKEN_TYPEOF:
        case TOKEN_VOID:
        case TOKEN_DELETE:
        case TOKEN_DEBUGGER:
        case TOKEN_WITH:
        case TOKEN_STATIC:
        case TOKEN_GET:
        case TOKEN_SET:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        case TOKEN_NULL:
        case TOKEN_UNDEFINED:
        case TOKEN_ENUM:
        case TOKEN_IMPLEMENTS:
        case TOKEN_INTERFACE:
        case TOKEN_PACKAGE:
        case TOKEN_PRIVATE:
        case TOKEN_PROTECTED:
        case TOKEN_PUBLIC:
        case TOKEN_MOD_ASSIGN:
        case TOKEN_DIV_ASSIGN:
        case TOKEN_TIMES_ASSIGN:
        case TOKEN_MINUS_ASSIGN:
        case TOKEN_PLUS_ASSIGN:
        case TOKEN_POWER_ASSIGN:
        case TOKEN_EQUAL:
        case TOKEN_STRICT_EQUAL:
        case TOKEN_NOT_EQUAL:
        case TOKEN_STRICT_NOT_EQUAL:
        case TOKEN_LESS:
        case TOKEN_LESS_EQUAL:
        case TOKEN_GREATER:
        case TOKEN_GREATER_EQUAL:
        case TOKEN_LOGICAL_AND:
        case TOKEN_LOGICAL_OR:
        case TOKEN_NOT:
        case TOKEN_TERNARY:
        case TOKEN_COLON:
        case TOKEN_COMMA:
        case TOKEN_SEMICOLON:
        case TOKEN_OPENING_KEY:
        case TOKEN_OPENING_BRA:
        case TOKEN_OPENING_PAR:
        case TOKEN_ARROW:
            return 1;
        default:
            return 0;
    }
}

/**
 * @brief Chooses the scanner context after a token has been emitted.
 */
ScannerContext scanner_context_after(const TokenType previous_type) {
    return scanner_allows_regex_after(previous_type)
               ? SCANNER_CONTEXT_EXPECT_REGEX
               : SCANNER_CONTEXT_DEFAULT;
}

/**
 * @brief Returns the DFA entry state for a given scanner context.
 */
LexerState scanner_entry_state_for_context(const ScannerContext context) {
    return context == SCANNER_CONTEXT_EXPECT_REGEX
               ? dfa_regex_entry_state()
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
        if (input_class == CHAR_LETTER && (unsigned char) c >= 0xC0) {
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
int scanner_match_longest_after(Scanner *sc, const TokenType previous_type,
                                ScannerContext *used_context,
                                size_t *consumed_length) {
    const ScannerContext default_context = SCANNER_CONTEXT_DEFAULT;

    if (!scanner_allows_regex_after(previous_type)) {
        if (used_context != NULL) {
            *used_context = default_context;
        }
        return scanner_match_longest(
            sc, scanner_entry_state_for_context(default_context), consumed_length);
    }

    Scanner default_probe = *sc;
    Scanner regex_probe = *sc;
    size_t regex_length = 0;
    size_t default_length = 0;
    int regex_ok = 0;

    if (scanner_peek(&regex_probe, 0) == '/') {
        scanner_next(&regex_probe);
        regex_ok = scanner_match_longest(&regex_probe, dfa_regex_entry_state(),
                                         &regex_length);
        if (regex_ok) {
            regex_length += 1;
        }
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
        const unsigned char ub = (unsigned char) c;
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

int scanner_match(Scanner *sc, const char expected) {
    if (scanner_peek(sc, 0) == expected) {
        scanner_next(sc);
        return 1;
    }
    return 0;
}

size_t scanner_position(const Scanner *sc) { return sc->pos; }

int scanner_line(const Scanner *sc) { return sc->line; }

int scanner_column(const Scanner *sc) { return sc->column; }