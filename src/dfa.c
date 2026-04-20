/**
 * @file dfa.c
 * @brief DFA transition table construction and query implementation.
 *
 * The automaton is represented as two static arrays:
 *
 *   dfa_table[_STATE_ENUM_COUNT][_CHAR_ENUM_COUNT]
 *     A 2-D array where dfa_table[s][c] holds the next state to enter when
 *     the automaton is in state s and the current input symbol belongs to
 *     character class c.
 *
 *   dfa_accepting_state[_STATE_ENUM_COUNT]
 *     A boolean array where a non-zero value at index s means that state s
 *     is an accepting state (i.e. the input consumed so far forms a valid
 *     token and could legally terminate there).
 *
 * Initialization pattern:
 *   dfa_init() first fills every cell of dfa_table with STATE_ERROR and clears
 *   every entry in dfa_accepting_state. Individual rule-setting helpers then
 *   overwrite only the cells relevant to each lexical category. Any transition
 *   not explicitly set therefore remains STATE_ERROR, so undefined input is
 *   always rejected.
 *
 * Rule organization:
 *   Rules are grouped into static helpers by category (layout, identifiers,
 *   Unicode escapes, numbers, strings, regex, operators). This makes it easy
 *   to add a new token kind without risking accidental interference with
 *   existing transitions.
 */
#include "../include/dfa.h"

/** 2-D transition table: dfa_table[current_state][char_class] -> next_state. */
static LexerState dfa_table[_STATE_ENUM_COUNT][_CHAR_ENUM_COUNT];

/** Guards against redundant re-initialization when used as a library. */
static int dfa_initialized = 0;

/**
 * Accepting-state flags: dfa_accepting_state[state] != 0 when the state
 * represents the end of a valid token.
 */
static int dfa_accepting_state[_STATE_ENUM_COUNT];

/**
 * @brief Sets every cell in the transition table to @p value.
 *
 * Used at the start of dfa_init() to establish STATE_ERROR as the universal
 * default so that only explicitly configured transitions are reachable.
 *
 * @param value LexerState to write into every table cell.
 */
static void dfa_set_all(LexerState value) {
  for (int s = 0; s < _STATE_ENUM_COUNT; s++) {
    for (int c = 0; c < _CHAR_ENUM_COUNT; c++) {
      dfa_table[s][c] = value;
    }
  }
}

/**
 * @brief Marks the states that represent valid token endings.
 *
 * All entries are first cleared to 0 (non-accepting). Individual states are
 * then set to 1. Notable design choices:
 *  - STATE_AMPERSAND and STATE_PIPE are left non-accepting: standalone `&`
 *    and `|` are lexical errors in EsJS; only `&&` and `||` are legal.
 *  - STATE_DOT_NUMBER is intentionally non-accepting: trailing-dot floats
 *    like `10.` are not valid number tokens; only `10.5` form is accepted.
 *  - The Unicode escape terminal states (STATE_ID_UNICODE_HEX4 and
 *    STATE_ID_UNICODE_BRACE_CLOSE) are accepting so that an identifier
 *    consisting solely of a Unicode escape is recognized.
 */
static void dfa_init_accepting_states(void) {
  for (int i = 0; i < _STATE_ENUM_COUNT; i++) {
    dfa_accepting_state[i] = 0;
  }

  dfa_accepting_state[STATE_IDENTIFIER] = 1;
  dfa_accepting_state[STATE_NUMBER] = 1;
  /* STATE_DOT_NUMBER is non-accepting: `10.` is not a valid number literal;
   * a digit must follow the decimal point (e.g. `10.5`). */
  dfa_accepting_state[STATE_FLOAT_NUMBER] = 1;

  dfa_accepting_state[STATE_DONE] = 1;

  /* Single-character operator states are accepting except & and |. */
  dfa_accepting_state[STATE_PLUS] = 1;
  dfa_accepting_state[STATE_MINUS] = 1;
  dfa_accepting_state[STATE_STAR] = 1;
  dfa_accepting_state[STATE_SLASH] = 1;
  dfa_accepting_state[STATE_PERCENT] = 1;
  dfa_accepting_state[STATE_EQUAL] = 1;
  dfa_accepting_state[STATE_BANG] = 1;
  dfa_accepting_state[STATE_LESS] = 1;
  dfa_accepting_state[STATE_GREATER] = 1;
  dfa_accepting_state[STATE_AMPERSAND] = 0; /* `&` alone is invalid. */
  dfa_accepting_state[STATE_PIPE] = 0;      /* `|` alone is invalid. */
  dfa_accepting_state[STATE_QUESTION] = 1;
  dfa_accepting_state[STATE_DOT] = 1;

  dfa_accepting_state[STATE_STAR_STAR] = 1;
  dfa_accepting_state[STATE_EQUAL_EQUAL] = 1;
  dfa_accepting_state[STATE_BANG_EQUAL] = 1;

  /* Unicode escape complete states. */
  dfa_accepting_state[STATE_ID_UNICODE_HEX4] = 1;
  dfa_accepting_state[STATE_ID_UNICODE_BRACE_CLOSE] = 1;
}

/**
 * @brief Installs whitespace and newline self-loops on STATE_START.
 *
 * Whitespace and newlines at the start state are consumed silently. This
 * means the DFA skips leading layout without producing a whitespace token.
 * The Lexer still needs its own skip loop for comments, but simple blanks are
 * handled here at the DFA level.
 */
static void dfa_set_layout_rules(void) {
  dfa_table[STATE_START][CHAR_WHITESPACE] = STATE_START;
  dfa_table[STATE_START][CHAR_NEWLINE] = STATE_START;
}

/**
 * @brief Configures identifier recognition transitions.
 *
 * EsJS identifier syntax (aligned with ECMAScript):
 *   IdentifierStart ::= [$A-Za-z_] | UTF8LeadingByte | UnicodeEscape
 *   IdentifierPart  ::= IdentifierStart | [0-9]
 *
 * The `$` character and UTF-8 leading bytes are valid identifier starts and
 * continuations. Digits are valid only as continuation characters.
 */
static void dfa_set_identifier_rules(void) {
  /* Identifier start characters from STATE_START. */
  dfa_table[STATE_START][CHAR_LETTER] = STATE_IDENTIFIER;
  dfa_table[STATE_START][CHAR_U_LOWER] = STATE_IDENTIFIER;
  dfa_table[STATE_START][CHAR_UNDERSCORE] = STATE_IDENTIFIER;
  dfa_table[STATE_START][CHAR_DOLLAR_SIGN] = STATE_IDENTIFIER;
  dfa_table[STATE_START][CHAR_UTF8_CONTINUATION] = STATE_IDENTIFIER;

  /* Identifier continuation characters. */
  dfa_table[STATE_IDENTIFIER][CHAR_LETTER] = STATE_IDENTIFIER;
  dfa_table[STATE_IDENTIFIER][CHAR_U_LOWER] = STATE_IDENTIFIER;
  dfa_table[STATE_IDENTIFIER][CHAR_UNDERSCORE] = STATE_IDENTIFIER;
  dfa_table[STATE_IDENTIFIER][CHAR_DOLLAR_SIGN] = STATE_IDENTIFIER;
  dfa_table[STATE_IDENTIFIER][CHAR_DIGIT] = STATE_IDENTIFIER;
  dfa_table[STATE_IDENTIFIER][CHAR_UTF8_CONTINUATION] = STATE_IDENTIFIER;
}

/**
 * @brief Configures Unicode escape sequence recognition inside identifiers.
 *
 * ECMAScript allows identifiers to contain Unicode escapes in two forms:
 *   Fixed:  \uXXXX       (exactly 4 hex digits)
 *   Brace:  \u{X...X}   (one or more hex digits)
 *
 * Both forms are recognized at the DFA level. Strict hex-digit validation
 * (i.e. rejecting 'g'-'z') is intentionally deferred to semantic analysis;
 * the DFA accepts any letter or digit in the hex positions to keep the
 * table simple.
 *
 * Escape sequences may appear:
 *  - At the very start of an identifier (from STATE_START via `\`).
 *  - After any regular identifier character (from STATE_IDENTIFIER via `\`).
 *  - After a complete prior escape sequence (from STATE_ID_UNICODE_HEX4 or
 *    STATE_ID_UNICODE_BRACE_CLOSE via `\`).
 */
static void dfa_set_unicode_escape_rules(void) {
  /* Entry points: `\` is valid at identifier start or within an identifier. */
  dfa_table[STATE_START][CHAR_BACKSLASH] = STATE_ID_BACKSLASH;
  dfa_table[STATE_IDENTIFIER][CHAR_BACKSLASH] = STATE_ID_BACKSLASH;
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_BACKSLASH] = STATE_ID_BACKSLASH;
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_BACKSLASH] = STATE_ID_BACKSLASH;

  /* After `\`: only `u` is valid to start a Unicode escape. */
  dfa_table[STATE_ID_BACKSLASH][CHAR_U_LOWER] = STATE_ID_UNICODE_U;

  /* After `\u`: branch on `{` (brace form) or first hex digit (fixed form). */
  dfa_table[STATE_ID_UNICODE_U][CHAR_LBRACE] = STATE_ID_UNICODE_BRACE;
  dfa_table[STATE_ID_UNICODE_U][CHAR_DIGIT] = STATE_ID_UNICODE_HEX1;
  dfa_table[STATE_ID_UNICODE_U][CHAR_LETTER] = STATE_ID_UNICODE_HEX1;
  dfa_table[STATE_ID_UNICODE_U][CHAR_U_LOWER] = STATE_ID_UNICODE_HEX1;

  /* Fixed form \uXXXX: consume exactly 4 characters in the hex positions.
   * CHAR_U_LOWER is treated as a valid hex digit here ('u' != valid hex,
   * but strict validation is left to a later phase). */
  dfa_table[STATE_ID_UNICODE_HEX1][CHAR_DIGIT] = STATE_ID_UNICODE_HEX2;
  dfa_table[STATE_ID_UNICODE_HEX1][CHAR_LETTER] = STATE_ID_UNICODE_HEX2;
  dfa_table[STATE_ID_UNICODE_HEX1][CHAR_U_LOWER] = STATE_ID_UNICODE_HEX2;

  dfa_table[STATE_ID_UNICODE_HEX2][CHAR_DIGIT] = STATE_ID_UNICODE_HEX3;
  dfa_table[STATE_ID_UNICODE_HEX2][CHAR_LETTER] = STATE_ID_UNICODE_HEX3;
  dfa_table[STATE_ID_UNICODE_HEX2][CHAR_U_LOWER] = STATE_ID_UNICODE_HEX3;

  dfa_table[STATE_ID_UNICODE_HEX3][CHAR_DIGIT] = STATE_ID_UNICODE_HEX4;
  dfa_table[STATE_ID_UNICODE_HEX3][CHAR_LETTER] = STATE_ID_UNICODE_HEX4;
  dfa_table[STATE_ID_UNICODE_HEX3][CHAR_U_LOWER] = STATE_ID_UNICODE_HEX4;

  /* After \uXXXX (STATE_ID_UNICODE_HEX4): continue as identifier content or
   * start another escape. */
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_LETTER] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_U_LOWER] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_UNDERSCORE] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_DOLLAR_SIGN] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_DIGIT] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_UTF8_CONTINUATION] = STATE_IDENTIFIER;

  /* Brace form \u{X+}: one or more hex digits followed by `}`. */
  dfa_table[STATE_ID_UNICODE_BRACE][CHAR_DIGIT] = STATE_ID_UNICODE_BRACE_HEX;
  dfa_table[STATE_ID_UNICODE_BRACE][CHAR_LETTER] = STATE_ID_UNICODE_BRACE_HEX;
  dfa_table[STATE_ID_UNICODE_BRACE][CHAR_U_LOWER] = STATE_ID_UNICODE_BRACE_HEX;

  /* STATE_ID_UNICODE_BRACE_HEX loops on additional hex digits. */
  dfa_table[STATE_ID_UNICODE_BRACE_HEX][CHAR_DIGIT] =
      STATE_ID_UNICODE_BRACE_HEX;
  dfa_table[STATE_ID_UNICODE_BRACE_HEX][CHAR_LETTER] =
      STATE_ID_UNICODE_BRACE_HEX;
  dfa_table[STATE_ID_UNICODE_BRACE_HEX][CHAR_U_LOWER] =
      STATE_ID_UNICODE_BRACE_HEX;
  dfa_table[STATE_ID_UNICODE_BRACE_HEX][CHAR_RBRACE] =
      STATE_ID_UNICODE_BRACE_CLOSE;

  /* After \u{X+} (STATE_ID_UNICODE_BRACE_CLOSE): continue as identifier. */
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_LETTER] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_U_LOWER] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_UNDERSCORE] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_DOLLAR_SIGN] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_DIGIT] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_UTF8_CONTINUATION] =
      STATE_IDENTIFIER;
}

/**
 * @brief Configures numeric literal recognition transitions.
 *
 * Recognized forms:
 *   Integer : [0-9]+
 *   Float   : [0-9]+ '.' [0-9]*  (trailing dot accepted, e.g. `10.`)
 *   Float   : [0-9]+ '.' [0-9]+
 *
 * Consecutive dots after the decimal point (e.g. `1..`) and a second dot
 * in the fractional part (e.g. `1.2.3`) are mapped to STATE_ERROR.
 */
static void dfa_set_number_rules(void) {
  dfa_table[STATE_START][CHAR_DIGIT] = STATE_NUMBER;

  /* Integer continuation. */
  dfa_table[STATE_NUMBER][CHAR_DIGIT] = STATE_NUMBER;

  /* Decimal point: transition from integer to trailing-dot float. */
  dfa_table[STATE_NUMBER][CHAR_DOT] = STATE_DOT_NUMBER;

  /* Fractional digits after the dot. */
  dfa_table[STATE_DOT_NUMBER][CHAR_DIGIT] = STATE_FLOAT_NUMBER;
  dfa_table[STATE_FLOAT_NUMBER][CHAR_DIGIT] = STATE_FLOAT_NUMBER;

  /* A second dot is invalid in any numeric context. */
  dfa_table[STATE_DOT_NUMBER][CHAR_DOT] = STATE_ERROR;
  dfa_table[STATE_FLOAT_NUMBER][CHAR_DOT] = STATE_ERROR;
}

/**
 * @brief Configures string literal recognition for all three quote styles.
 *
 * String body states accept every character class by default (loop
 * transitions). Specific overrides are then applied for:
 *  - The closing delimiter: transitions to STATE_DONE.
 *  - The backslash escape introducer: transitions to the corresponding
 *    escape state (which unconditionally returns to the body state after
 *    one character, implementing a single-step escape).
 *  - EOF and (for non-template strings) NEWLINE: transition to STATE_ERROR
 *    because an unterminated string is a lexical error.
 *
 * Template literals (backtick strings) allow embedded newlines, so
 * CHAR_NEWLINE does not trigger STATE_ERROR for STATE_STRING_BACKTICK.
 */
static void dfa_set_string_rules(void) {
  /* Entry transitions from STATE_START. */
  dfa_table[STATE_START][CHAR_DOUBLE_QUOTE] = STATE_STRING_DOUBLE;
  dfa_table[STATE_START][CHAR_SINGLE_QUOTE] = STATE_STRING_SINGLE;
  dfa_table[STATE_START][CHAR_BACKTICK] = STATE_STRING_BACKTICK;

  /* Body states: accept all character classes by default (loop). */
  for (int ctype = 0; ctype < _CHAR_ENUM_COUNT; ctype++) {
    dfa_table[STATE_STRING_DOUBLE][ctype] = STATE_STRING_DOUBLE;
    dfa_table[STATE_STRING_SINGLE][ctype] = STATE_STRING_SINGLE;
    dfa_table[STATE_STRING_BACKTICK][ctype] = STATE_STRING_BACKTICK;

    /* After an escape character: consume one character and return to body. */
    dfa_table[STATE_STRING_DOUBLE_ESCAPE][ctype] = STATE_STRING_DOUBLE;
    dfa_table[STATE_STRING_SINGLE_ESCAPE][ctype] = STATE_STRING_SINGLE;
    dfa_table[STATE_STRING_BACKTICK_ESCAPE][ctype] = STATE_STRING_BACKTICK;
  }

  /* Closing delimiters terminate the string token. */
  dfa_table[STATE_STRING_DOUBLE][CHAR_DOUBLE_QUOTE] = STATE_DONE;
  dfa_table[STATE_STRING_SINGLE][CHAR_SINGLE_QUOTE] = STATE_DONE;
  dfa_table[STATE_STRING_BACKTICK][CHAR_BACKTICK] = STATE_DONE;

  /* Escape introducer: enter the single-step escape sub-state. */
  dfa_table[STATE_STRING_DOUBLE][CHAR_BACKSLASH] = STATE_STRING_DOUBLE_ESCAPE;
  dfa_table[STATE_STRING_SINGLE][CHAR_BACKSLASH] = STATE_STRING_SINGLE_ESCAPE;
  dfa_table[STATE_STRING_BACKTICK][CHAR_BACKSLASH] =
      STATE_STRING_BACKTICK_ESCAPE;

  /* EOF inside any string is a lexical error. */
  dfa_table[STATE_STRING_DOUBLE][CHAR_EOF] = STATE_ERROR;
  dfa_table[STATE_STRING_SINGLE][CHAR_EOF] = STATE_ERROR;
  dfa_table[STATE_STRING_BACKTICK][CHAR_EOF] = STATE_ERROR;

  /* Newline inside single/double-quoted strings is a lexical error.
   * Template literals (backtick) permit embedded newlines. */
  dfa_table[STATE_STRING_DOUBLE][CHAR_NEWLINE] = STATE_ERROR;
  dfa_table[STATE_STRING_SINGLE][CHAR_NEWLINE] = STATE_ERROR;

  /* EOF inside escape states is also an error. */
  dfa_table[STATE_STRING_DOUBLE_ESCAPE][CHAR_EOF] = STATE_ERROR;
  dfa_table[STATE_STRING_SINGLE_ESCAPE][CHAR_EOF] = STATE_ERROR;
  dfa_table[STATE_STRING_BACKTICK_ESCAPE][CHAR_EOF] = STATE_ERROR;
}

/**
 * @brief Configures regex literal recognition transitions.
 *
 * A regex literal is delimited by unescaped forward slashes: `/pattern/flags`.
 * This function installs rules starting from STATE_REGEX_BODY, which is entered
 * externally (not from STATE_START) to avoid conflating the opening `/` with the
 * division operator.
 *
 * Sub-states:
 *  - STATE_REGEX_BODY        : main pattern body; accepts all chars by default.
 *  - STATE_REGEX_BODY_ESCAPE : after `\` in the body; next char is literal.
 *  - STATE_REGEX_CLASS       : inside `[...]` character class; `/` is literal.
 *  - STATE_REGEX_CLASS_ESCAPE: after `\` inside a character class.
 *
 * Character classes inside a regex (`[...]`) allow unescaped `/`, so the
 * closing delimiter is only recognized outside the class context.
 *
 * Newline and EOF are errors in all regex sub-states because regex literals
 * cannot span lines.
 */
static void dfa_set_regex_rules(void) {
  /* Body and class states: accept all character classes by default. */
  for (int ctype = 0; ctype < _CHAR_ENUM_COUNT; ctype++) {
    dfa_table[STATE_REGEX_BODY][ctype] = STATE_REGEX_BODY;
    dfa_table[STATE_REGEX_CLASS][ctype] = STATE_REGEX_CLASS;

    /* After an escape: consume one character and return to the enclosing state. */
    dfa_table[STATE_REGEX_BODY_ESCAPE][ctype] = STATE_REGEX_BODY;
    dfa_table[STATE_REGEX_CLASS_ESCAPE][ctype] = STATE_REGEX_CLASS;
  }

  /* Closing `/` terminates the regex body. */
  dfa_table[STATE_REGEX_BODY][CHAR_SLASH] = STATE_DONE;

  /* Escape introducer in the body. */
  dfa_table[STATE_REGEX_BODY][CHAR_BACKSLASH] = STATE_REGEX_BODY_ESCAPE;

  /* `[` opens a character class; inside it `/` is not a terminator. */
  dfa_table[STATE_REGEX_BODY][CHAR_LBRACKET] = STATE_REGEX_CLASS;

  /* `]` closes the character class and returns to body scanning. */
  dfa_table[STATE_REGEX_CLASS][CHAR_RBRACKET] = STATE_REGEX_BODY;

  /* Escape introducer inside a character class. */
  dfa_table[STATE_REGEX_CLASS][CHAR_BACKSLASH] = STATE_REGEX_CLASS_ESCAPE;

  /* Newline and EOF are hard errors in every regex sub-state. */
  dfa_table[STATE_REGEX_BODY][CHAR_NEWLINE] = STATE_ERROR;
  dfa_table[STATE_REGEX_BODY][CHAR_EOF] = STATE_ERROR;
  dfa_table[STATE_REGEX_CLASS][CHAR_NEWLINE] = STATE_ERROR;
  dfa_table[STATE_REGEX_CLASS][CHAR_EOF] = STATE_ERROR;
  dfa_table[STATE_REGEX_BODY_ESCAPE][CHAR_NEWLINE] = STATE_ERROR;
  dfa_table[STATE_REGEX_BODY_ESCAPE][CHAR_EOF] = STATE_ERROR;
  dfa_table[STATE_REGEX_CLASS_ESCAPE][CHAR_NEWLINE] = STATE_ERROR;
  dfa_table[STATE_REGEX_CLASS_ESCAPE][CHAR_EOF] = STATE_ERROR;
}

/**
 * @brief Configures operator and punctuation recognition transitions.
 *
 * Single-character punctuators (, ; : { } [ ] ( )) go directly to STATE_DONE
 * in one step because no lookahead is needed.
 *
 * Multi-character operators are handled via intermediate states. For each
 * operator prefix that could be extended, the DFA has an intermediate state
 * that is itself accepting (the prefix is a valid shorter token) and has
 * transitions to longer variants:
 *
 *   +  -> STATE_PLUS  (accepts as TOKEN_PLUS)
 *          + CHAR_PLUS  -> STATE_DONE (++)
 *          + CHAR_EQUAL -> STATE_DONE (+=)
 *
 *   *  -> STATE_STAR  (accepts as TOKEN_TIMES)
 *          + CHAR_STAR  -> STATE_STAR_STAR (**)
 *                         + CHAR_EQUAL -> STATE_DONE (**=)
 *          + CHAR_EQUAL -> STATE_DONE (*=)
 *
 *   =  -> STATE_EQUAL (accepts as TOKEN_ASSIGN)
 *          + CHAR_EQUAL  -> STATE_EQUAL_EQUAL (==)
 *                           + CHAR_EQUAL -> STATE_DONE (===)
 *          + CHAR_GREATER -> STATE_DONE (=>)
 *
 *   !  -> STATE_BANG  (accepts as TOKEN_NOT)
 *          + CHAR_EQUAL  -> STATE_BANG_EQUAL (!=)
 *                           + CHAR_EQUAL -> STATE_DONE (!==)
 *
 *   &  -> STATE_AMPERSAND  (non-accepting — standalone & is an error)
 *          + CHAR_AMPERSAND -> STATE_DONE (&&)
 *
 *   |  -> STATE_PIPE  (non-accepting — standalone | is an error)
 *          + CHAR_PIPE -> STATE_DONE (||)
 *
 *   .  -> STATE_DOT  (accepts as TOKEN_PERIOD)
 *          + CHAR_DOT -> STATE_DOT_DOT (partial)
 *                        + CHAR_DOT -> STATE_DONE (...)
 */
static void dfa_set_operator_rules(void) {
  /* Single-step operators: enter an ambiguous prefix state. */
  dfa_table[STATE_START][CHAR_PLUS] = STATE_PLUS;
  dfa_table[STATE_START][CHAR_MINUS] = STATE_MINUS;
  dfa_table[STATE_START][CHAR_STAR] = STATE_STAR;
  dfa_table[STATE_START][CHAR_SLASH] = STATE_SLASH;
  dfa_table[STATE_START][CHAR_PERCENT] = STATE_PERCENT;
  dfa_table[STATE_START][CHAR_EQUAL] = STATE_EQUAL;
  dfa_table[STATE_START][CHAR_BANG] = STATE_BANG;
  dfa_table[STATE_START][CHAR_LESS] = STATE_LESS;
  dfa_table[STATE_START][CHAR_GREATER] = STATE_GREATER;
  dfa_table[STATE_START][CHAR_AMPERSAND] = STATE_AMPERSAND;
  dfa_table[STATE_START][CHAR_PIPE] = STATE_PIPE;
  dfa_table[STATE_START][CHAR_QUESTION] = STATE_QUESTION;
  dfa_table[STATE_START][CHAR_DOT] = STATE_DOT;

  /* Single-character punctuators: terminate immediately. */
  dfa_table[STATE_START][CHAR_COMMA] = STATE_DONE;
  dfa_table[STATE_START][CHAR_SEMICOLON] = STATE_DONE;
  dfa_table[STATE_START][CHAR_COLON] = STATE_DONE;
  dfa_table[STATE_START][CHAR_LBRACE] = STATE_DONE;
  dfa_table[STATE_START][CHAR_RBRACE] = STATE_DONE;
  dfa_table[STATE_START][CHAR_LBRACKET] = STATE_DONE;
  dfa_table[STATE_START][CHAR_RBRACKET] = STATE_DONE;
  dfa_table[STATE_START][CHAR_LPAREN] = STATE_DONE;
  dfa_table[STATE_START][CHAR_RPAREN] = STATE_DONE;

  /* Multi-character operator extensions. */
  dfa_table[STATE_PLUS][CHAR_PLUS] = STATE_DONE;    /* ++ */
  dfa_table[STATE_PLUS][CHAR_EQUAL] = STATE_DONE;   /* += */

  dfa_table[STATE_MINUS][CHAR_MINUS] = STATE_DONE;  /* -- */
  dfa_table[STATE_MINUS][CHAR_EQUAL] = STATE_DONE;  /* -= */

  dfa_table[STATE_STAR][CHAR_STAR] = STATE_STAR_STAR;
  dfa_table[STATE_STAR][CHAR_EQUAL] = STATE_DONE;   /* *= */

  dfa_table[STATE_STAR_STAR][CHAR_EQUAL] = STATE_DONE; /* **= */

  dfa_table[STATE_SLASH][CHAR_EQUAL] = STATE_DONE;  /* /= */

  dfa_table[STATE_PERCENT][CHAR_EQUAL] = STATE_DONE; /* %= */

  dfa_table[STATE_EQUAL][CHAR_EQUAL] = STATE_EQUAL_EQUAL;
  dfa_table[STATE_EQUAL][CHAR_GREATER] = STATE_DONE; /* => */

  dfa_table[STATE_EQUAL_EQUAL][CHAR_EQUAL] = STATE_DONE; /* === */

  dfa_table[STATE_BANG][CHAR_EQUAL] = STATE_BANG_EQUAL;
  dfa_table[STATE_BANG_EQUAL][CHAR_EQUAL] = STATE_DONE; /* !== */

  dfa_table[STATE_LESS][CHAR_EQUAL] = STATE_DONE;    /* <= */
  dfa_table[STATE_GREATER][CHAR_EQUAL] = STATE_DONE; /* >= */

  dfa_table[STATE_AMPERSAND][CHAR_AMPERSAND] = STATE_DONE; /* && */

  dfa_table[STATE_PIPE][CHAR_PIPE] = STATE_DONE;     /* || */

  dfa_table[STATE_QUESTION][CHAR_QUESTION] = STATE_DONE; /* ?? */

  dfa_table[STATE_DOT][CHAR_DOT] = STATE_DOT_DOT;
  dfa_table[STATE_DOT_DOT][CHAR_DOT] = STATE_DONE;  /* ... */
}

/**
 * @brief Builds the complete DFA by invoking all rule-setting helpers.
 *
 * Call this once at program startup before creating any Lexer or Scanner
 * instance. The function is not re-entrant and not thread-safe during
 * execution, but the resulting tables are read-only afterward.
 */
void dfa_init(void) {
  if (dfa_initialized) return;
  dfa_initialized = 1;

  dfa_set_all(STATE_ERROR);        /* Default: every transition is an error. */
  dfa_init_accepting_states();

  dfa_set_layout_rules();
  dfa_set_identifier_rules();
  dfa_set_unicode_escape_rules();
  dfa_set_number_rules();
  dfa_set_string_rules();
  dfa_set_regex_rules();
  dfa_set_operator_rules();
}

/**
 * @brief Returns the DFA entry state for scanning a regex literal body.
 *
 * The scanner manually consumes the opening `/` before calling
 * scanner_match_longest() with this entry state, so the state machine starts
 * inside the regex body rather than having to parse the leading delimiter.
 *
 * @return STATE_REGEX_BODY.
 */
LexerState dfa_regex_entry_state(void) { return STATE_REGEX_BODY; }

/**
 * @brief Performs a single DFA transition lookup.
 *
 * @param current     Current automaton state.
 * @param input_class Character class of the consumed input.
 * @return            Next state, or STATE_ERROR for out-of-range arguments.
 */
LexerState dfa_next_state(const LexerState current,
                          const CharClass input_class) {
  if (current < 0 || current >= _STATE_ENUM_COUNT) {
    return STATE_ERROR;
  }
  if (input_class < 0 || input_class > CHAR_OTHER) {
    return STATE_ERROR;
  }
  return dfa_table[current][input_class];
}

/**
 * @brief Tests whether @p state is an accepting state.
 *
 * @param state State to test.
 * @return      Non-zero if accepting; 0 if non-accepting or out of range.
 */
int dfa_is_accepting(const LexerState state) {
  if (state < 0 || state >= _STATE_ENUM_COUNT) {
    return 0;
  }
  return dfa_accepting_state[state];
}