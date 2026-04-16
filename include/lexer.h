/**
 * @file lexer.h
 * @brief High-level lexer interface that orchestrates scanning, token-type
 *        resolution, and comment/whitespace skipping.
 *
 * The Lexer is the public-facing component of the pipeline. It wraps a
 * Scanner and exposes a single function, lexer_next_token(), that a parser
 * or other consumer calls repeatedly to retrieve one Token at a time.
 *
 * Responsibilities:
 *  - Skip leading whitespace and comments (line comments "//" and block
 *    comments of the form slash-star...star-slash).
 *  - Record the source location (line, column) of the token start before
 *    consuming any input.
 *  - Delegate to the Scanner for maximal-munch DFA matching.
 *  - Interpret the raw lexeme to determine the precise TokenType (keyword
 *    vs. identifier, specific operator, etc.).
 *  - Strip delimiters from string and regex lexemes stored in the Token.
 *  - Produce a TOKEN_ERROR token and advance one byte on unrecognized input.
 *  - Produce a TOKEN_EOF token and return non-zero when end-of-stream is
 *    reached (the caller should stop iterating after seeing TOKEN_EOF).
 */
#ifndef ESJS_CUSTOM_LEXER_LEXER_H
#define ESJS_CUSTOM_LEXER_LEXER_H

#include "scanner.h"

/**
 * @brief Lexer state: a thin wrapper around a Scanner.
 *
 * The Lexer itself is stateless beyond the Scanner it contains; all position
 * and buffer state is tracked inside Scanner and Buffer. Future extensions
 * (e.g. a context stack for template literal interpolation) would add fields
 * here.
 */
typedef struct {
  Scanner scanner; /**< Underlying character-level scanner and DFA driver.  */
} Lexer;

/**
 * @brief Initializes the Lexer over an input buffer.
 *
 * The Buffer must have been initialized via buffer_init() before this call.
 * The Lexer does not take ownership of @p buffer.
 *
 * @param lexer  Lexer instance to initialize. Must not be NULL.
 * @param buffer Input buffer to read from. Must remain live for the Lexer's
 *               lifetime.
 */
void lexer_init(Lexer *lexer, Buffer *buffer);

/**
 * @brief Scans and returns the next token from the input.
 *
 * The function advances through whitespace and comments before attempting to
 * match a token, so the returned token is never a whitespace pseudo-token.
 *
 * Token production rules:
 *  - If the current position is at EOF, fills @p token with TOKEN_EOF and
 *    returns 1. The caller should treat this as the termination signal.
 *  - If the DFA matches one or more bytes and the result is an accepting
 *    state, the token type is determined from the lexeme content and the
 *    scanner context (regex vs. default). String and regex tokens have their
 *    surrounding delimiters stripped.
 *  - If no valid match is found, the current byte is consumed, @p token is
 *    filled with TOKEN_ERROR, and 1 is returned. The caller may choose to
 *    abort or recover from the error.
 *
 * @param lexer Lexer instance. Must not be NULL.
 * @param token Output parameter filled with the recognized token. Must not
 *              be NULL.
 * @return      Always 1 (non-zero). The end-of-input condition is signalled
 *              via TOKEN_EOF, not a zero return value.
 */
int lexer_next_token(Lexer *lexer, Token *token);

#endif  // ESJS_CUSTOM_LEXER_LEXER_H
