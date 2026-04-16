/**
 * @file lexer.c
 * @brief High-level lexer: whitespace/comment skipping, token-type resolution,
 *        and token structure population.
 *
 * The Lexer sits above the Scanner and is the component that external code
 * (the parser, the CLI driver) interacts with. Its responsibilities are:
 *
 *  1. Skipping non-token input: whitespace and comments are consumed silently
 *     before each token attempt. An unclosed block comment is treated as a
 *     lexical error and causes TOKEN_ERROR to be returned immediately.
 *
 *  2. Recording source position: line and column are captured before any
 *     characters are consumed so the resulting Token reflects the start of
 *     the lexeme, not the position after it.
 *
 *  3. Delegating matching: scanner_match_longest_after() drives the DFA and
 *     returns the raw lexeme bytes and the winning scanner context.
 *
 *  4. Resolving token type: the raw lexeme is inspected by
 *     lexer_token_type_from_lexeme() to produce the precise TokenType. This
 *     step handles keyword vs. identifier classification, operator
 *     disambiguation (length-based), and string/regex context.
 *
 *  5. Stripping delimiters: for TOKEN_STRING and TOKEN_REGEX the surrounding
 *     quotes/slashes are excluded from the lexeme stored in the Token so that
 *     consumers receive the content directly.
 *
 *  6. Error recovery: if no DFA match is found the current byte is consumed
 *     and a TOKEN_ERROR is returned. The lexer does not attempt to skip
 *     forward or synchronize; that decision belongs to the caller.
 */
#include "../include/lexer.h"

#include "../include/keywords.h"

/**
 * @brief Populates a Token structure with the supplied field values.
 *
 * @param token         Token to fill. Must not be NULL.
 * @param type          Resolved token type.
 * @param lexeme_start  Pointer into the buffer at the lexeme start.
 * @param lexeme_length Length of the lexeme in bytes.
 * @param line          1-based source line of the token start.
 * @param column        1-based source column of the token start.
 */
static void lexer_fill_token(Token *token, TokenType type,
                             const char *lexeme_start, size_t lexeme_length,
                             int line, int column) {
  token->type = type;
  token->lexeme_start = lexeme_start;
  token->lexeme_length = lexeme_length;
  token->line = line;
  token->column = column;
}

/**
 * @brief Consumes all whitespace and comment sequences at the current position.
 *
 * Three forms of skippable input are handled:
 *
 *  Whitespace: space, tab, carriage return, newline, form feed, vertical tab.
 *
 *  Line comment: "//" followed by all characters up to (but not including)
 *  the next newline or EOF. The newline itself is left to be consumed in the
 *  next iteration.
 *
 *  Block comment: slash-star followed by all characters up to and including
 *  the closing star-slash. Nested block comments are not supported. If EOF
 *  is reached before the closing delimiter, @p token is filled with a
 *  TOKEN_ERROR pointing to the opening slash-star and the function returns 0 to
 *  signal the error to the caller.
 *
 * @param lexer Active Lexer instance. Must not be NULL.
 * @param token Output token used exclusively when an unclosed block comment
 *              is detected. Must not be NULL.
 * @return      1 when skipping completed normally (caller should continue
 *              to scan a regular token); 0 when an unclosed block comment
 *              was found and @p token has been filled with TOKEN_ERROR.
 */
static int lexer_skip_layout_and_comments(Lexer *lexer, Token *token) {
  for (;;) {
    const int c = scanner_peek(&lexer->scanner, 0);
    if (c == EOF) {
      return 1;
    }

    /* Whitespace: consume and continue. */
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' ||
        c == '\v') {
      scanner_next(&lexer->scanner);
      continue;
    }

    /* Line comment: consume everything through end-of-line. */
    if (c == '/' && scanner_peek(&lexer->scanner, 1) == '/') {
      scanner_next(&lexer->scanner); /* consume first '/' */
      scanner_next(&lexer->scanner); /* consume second '/' */
      while (scanner_peek(&lexer->scanner, 0) != EOF &&
             scanner_peek(&lexer->scanner, 0) != '\n') {
        scanner_next(&lexer->scanner);
      }
      continue;
    }

    /* Block comment: consume everything through the closing star-slash delimiter.
     * Record the opening position so it can be reported if the comment is
     * not properly terminated before EOF. */
    if (c == '/' && scanner_peek(&lexer->scanner, 1) == '*') {
      const size_t comment_pos    = scanner_position(&lexer->scanner);
      const int    comment_line   = scanner_line(&lexer->scanner);
      const int    comment_column = scanner_column(&lexer->scanner);
      scanner_next(&lexer->scanner); /* consume '/' */
      scanner_next(&lexer->scanner); /* consume '*' */
      int closed = 0;
      while (scanner_peek(&lexer->scanner, 0) != EOF) {
        if (scanner_peek(&lexer->scanner, 0) == '*' &&
            scanner_peek(&lexer->scanner, 1) == '/') {
          scanner_next(&lexer->scanner); /* consume '*' */
          scanner_next(&lexer->scanner); /* consume '/' */
          closed = 1;
          break;
        }
        scanner_next(&lexer->scanner);
      }
      if (!closed) {
        /* An unclosed block comment is a lexical error. The token points to
         * the opening slash-star so the error message shows the correct position. */
        lexer_fill_token(token, TOKEN_ERROR,
                         lexer->scanner.buffer->data + comment_pos, 2,
                         comment_line, comment_column);
        return 0;
      }
      continue;
    }

    return 1; /* Current character is the start of a real token. */
  }
}

/**
 * @brief Determines the precise TokenType for a matched lexeme.
 *
 * The function inspects the first byte of @p lexeme together with @p length
 * and @p context to classify the token:
 *
 *  - SCANNER_CONTEXT_EXPECT_REGEX -> TOKEN_REGEX immediately (the scanner
 *    already verified that the lexeme is a valid regex literal).
 *  - Leading quote (`"`, `'`, `` ` ``) -> TOKEN_STRING.
 *  - Leading alphabetic character, `_`, `$`, `\`, or UTF-8 leading byte ->
 *    keyword_lookup() determines TOKEN_KEYWORD or TOKEN_IDENTIFIER.
 *  - Leading digit -> TOKEN_NUMBER.
 *  - `.` -> TOKEN_NUMBER when a digit follows (e.g. `.5`), TOKEN_SPREAD
 *    when length is 3 (`...`), or TOKEN_PERIOD for a single dot.
 *    Note: the number check is applied first so that `.5` is never confused
 *    with spread even though both have length > 1.
 *  - Punctuation characters `,`, `;`, `:`, `{`, `}`, `[`, `]`, `(`, `)` ->
 *    their respective single-character token types.
 *  - Operator characters: disambiguated by @p length and the second byte
 *    of the lexeme to distinguish e.g. `+` / `++` / `+=`.
 *
 * @param lexeme  Pointer to the first byte of the matched lexeme.
 * @param length  Lexeme length in bytes.
 * @param context Scanner context that produced the match.
 * @return        The most specific matching TokenType, or TOKEN_ERROR when
 *                no rule applies.
 */
static TokenType lexer_token_type_from_lexeme(const char *lexeme, size_t length,
                                              ScannerContext context) {
  const char first = lexeme[0];

  if (context == SCANNER_CONTEXT_EXPECT_REGEX) {
    return TOKEN_REGEX;
  }

  if (first == '"' || first == '\'' || first == '`') {
    return TOKEN_STRING;
  }

  /* Identifier or keyword: delegate to the keyword table. */
  if ((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') ||
      first == '_' || first == '$' || first == '\\' ||
      (unsigned char)first >= 0xC0) {
    return keyword_lookup(lexeme, length);
  }

  if (first >= '0' && first <= '9') {
    return TOKEN_NUMBER;
  }

  switch (first) {
    case '.':
      /* `.5` style float: digit immediately after the dot. */
      if (length > 1 && lexeme[1] >= '0' && lexeme[1] <= '9') {
        return TOKEN_NUMBER;
      }
      if (length == 3) {
        return TOKEN_SPREAD;         /* `...` */
      }
      return TOKEN_PERIOD;           /* single `.` */

    case ',': return TOKEN_COMMA;
    case ';': return TOKEN_SEMICOLON;
    case ':': return TOKEN_COLON;
    case '{': return TOKEN_OPENING_KEY;
    case '}': return TOKEN_CLOSING_KEY;
    case '[': return TOKEN_OPENING_BRA;
    case ']': return TOKEN_CLOSING_BRA;
    case '(': return TOKEN_OPENING_PAR;
    case ')': return TOKEN_CLOSING_PAR;

    case '+':
      if (length == 2 && lexeme[1] == '+') return TOKEN_INCREMENT;
      if (length == 2 && lexeme[1] == '=') return TOKEN_PLUS_ASSIGN;
      return TOKEN_PLUS;

    case '-':
      if (length == 2 && lexeme[1] == '-') return TOKEN_DECREMENT;
      if (length == 2 && lexeme[1] == '=') return TOKEN_MINUS_ASSIGN;
      return TOKEN_MINUS;

    case '*':
      if (length == 3) return TOKEN_POWER_ASSIGN;    /* `**=` */
      if (length == 2 && lexeme[1] == '*') return TOKEN_POWER;
      if (length == 2 && lexeme[1] == '=') return TOKEN_TIMES_ASSIGN;
      return TOKEN_TIMES;

    case '/':
      if (context == SCANNER_CONTEXT_EXPECT_REGEX && length > 1) {
        return TOKEN_REGEX;
      }
      return length == 2 ? TOKEN_DIV_ASSIGN : TOKEN_DIVIDE;

    case '%':
      return length == 2 ? TOKEN_MOD_ASSIGN : TOKEN_MOD;

    case '=':
      if (length == 3) return TOKEN_STRICT_EQUAL;    /* `===` */
      if (length == 2 && lexeme[1] == '>') return TOKEN_ARROW;
      return length == 2 ? TOKEN_EQUAL : TOKEN_ASSIGN;

    case '!':
      if (length == 3) return TOKEN_STRICT_NOT_EQUAL; /* `!==` */
      return length == 2 ? TOKEN_NOT_EQUAL : TOKEN_NOT;

    case '<':
      return length == 2 ? TOKEN_LESS_EQUAL : TOKEN_LESS;

    case '>':
      return length == 2 ? TOKEN_GREATER_EQUAL : TOKEN_GREATER;

    case '&':
      return length == 2 ? TOKEN_LOGICAL_AND : TOKEN_ERROR;

    case '|':
      return length == 2 ? TOKEN_LOGICAL_OR : TOKEN_ERROR;

    case '?':
      return length == 2 ? TOKEN_NULISH : TOKEN_TERNARY;

    default:
      return TOKEN_ERROR;
  }
}

/**
 * @brief Initializes the Lexer by delegating to scanner_init().
 *
 * @param lexer  Lexer to initialize. Must not be NULL.
 * @param buffer Initialized input buffer. Must remain live for the Lexer's
 *               lifetime.
 */
void lexer_init(Lexer *lexer, Buffer *buffer) {
  scanner_init(&lexer->scanner, buffer);
}

/**
 * @brief Produces the next token from the input stream.
 *
 * Processing steps:
 *  1. Skip whitespace and comments via lexer_skip_layout_and_comments().
 *     If an unclosed block comment is detected, that function fills @p token
 *     with TOKEN_ERROR and returns 0; lexer_next_token() propagates the error
 *     token immediately.
 *  2. Capture source position (start_pos, start_line, start_column).
 *  3. If the current byte is EOF, emit TOKEN_EOF and return.
 *  4. Call scanner_match_longest_after() to run the DFA and find the longest
 *     accepting prefix. The scanner is advanced by the match length.
 *  5. If no match is found (e.g. unrecognized character), consume one byte,
 *     emit TOKEN_ERROR, and return.
 *  6. Resolve the precise TokenType from the raw lexeme using
 *     lexer_token_type_from_lexeme().
 *  7. For TOKEN_STRING and TOKEN_REGEX, strip the surrounding delimiters:
 *     advance @p lexeme_start by 1 and reduce @p lexeme_length by 2.
 *  8. Fill the Token and return.
 *
 * @param lexer Active Lexer instance.
 * @param token Output Token. Must not be NULL.
 * @return      Always 1; end-of-input is signalled by TOKEN_EOF, lexical
 *              errors by TOKEN_ERROR.
 */
int lexer_next_token(Lexer *lexer, Token *token) {
  if (!lexer_skip_layout_and_comments(lexer, token)) {
    return 1; /* token already filled with TOKEN_ERROR by the skip function */
  }

  const size_t start_pos    = scanner_position(&lexer->scanner);
  const int    start_line   = scanner_line(&lexer->scanner);
  const int    start_column = scanner_column(&lexer->scanner);
  const int    current      = scanner_peek(&lexer->scanner, 0);

  if (current == EOF) {
    lexer_fill_token(token, TOKEN_EOF, NULL, 0, start_line, start_column);
    return 1;
  }

  Scanner        matched_scanner = lexer->scanner;
  size_t         matched_length  = 0;
  ScannerContext matched_context = SCANNER_CONTEXT_DEFAULT;

  const int matched = scanner_match_longest_after(
      &lexer->scanner, &matched_context, &matched_length);

  if (matched) {
    matched_scanner = lexer->scanner;
  }

  /* No valid match: consume the offending byte and report an error token. */
  if (!matched || matched_length == 0) {
    scanner_next(&lexer->scanner);
    lexer_fill_token(token, TOKEN_ERROR,
                     lexer->scanner.buffer->data + start_pos, 1, start_line,
                     start_column);
    return 1;
  }

  const char    *lexeme_start  = lexer->scanner.buffer->data + start_pos;
  const TokenType type         = lexer_token_type_from_lexeme(
      lexeme_start, matched_length, matched_context);
  size_t          lexeme_length = matched_length;

  /* Strip surrounding delimiters for string and regex tokens so that
   * consumers receive only the content (e.g. `hello` instead of `"hello"`). */
  if ((type == TOKEN_STRING || type == TOKEN_REGEX) && matched_length >= 2) {
    lexeme_start  += 1;
    lexeme_length -= 2;
  }

  lexer_fill_token(token, type, lexeme_start, lexeme_length, start_line,
                   start_column);
  lexer->scanner = matched_scanner;
  return 1;
}
