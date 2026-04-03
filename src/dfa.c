#include "../include/dfa.h"

static LexerState dfa_table[_STATE_ENUM_COUNT][_CHAR_ENUM_COUNT];
static int dfa_accepting_state[_STATE_ENUM_COUNT];

/**
 * @brief Assigns a default transition value to the complete DFA table.
 */
static void dfa_set_all(LexerState value) {
  for (int s = 0; s < _STATE_ENUM_COUNT; s++) {
    for (int c = 0; c < _CHAR_ENUM_COUNT; c++) {
      dfa_table[s][c] = value;
    }
  }
}

/**
 * @brief Enables states that may represent a valid lexeme end.
 */
static void dfa_init_accepting_states(void) {
  for (int i = 0; i < _STATE_ENUM_COUNT; i++) {
    dfa_accepting_state[i] = 0;
  }

  // Valid final states
  dfa_accepting_state[STATE_IDENTIFIER] = 1;
  dfa_accepting_state[STATE_NUMBER] = 1;
  dfa_accepting_state[STATE_DOT_NUMBER] = 1;  // Supports: 10.
  dfa_accepting_state[STATE_FLOAT_NUMBER] = 1;

  dfa_accepting_state[STATE_DONE] = 1;

  dfa_accepting_state[STATE_PLUS] = 1;
  dfa_accepting_state[STATE_MINUS] = 1;
  dfa_accepting_state[STATE_STAR] = 1;
  dfa_accepting_state[STATE_SLASH] = 1;
  dfa_accepting_state[STATE_PERCENT] = 1;
  dfa_accepting_state[STATE_EQUAL] = 1;
  dfa_accepting_state[STATE_BANG] = 1;
  dfa_accepting_state[STATE_LESS] = 1;
  dfa_accepting_state[STATE_GREATER] = 1;
  dfa_accepting_state[STATE_AMPERSAND] = 0;
  dfa_accepting_state[STATE_PIPE] = 0;
  // STATE_AMPERSAND and STATE_PIPE stay non-accepting so standalone '&' and
  // '|' are rejected. Only '&&' and '||' are supported.
  dfa_accepting_state[STATE_QUESTION] = 1;
  dfa_accepting_state[STATE_DOT] = 1;

  dfa_accepting_state[STATE_STAR_STAR] = 1;
  dfa_accepting_state[STATE_EQUAL_EQUAL] = 1;
  dfa_accepting_state[STATE_BANG_EQUAL] = 1;

  dfa_accepting_state[STATE_ID_UNICODE_HEX4] = 1;         // \uXXXX complete
  dfa_accepting_state[STATE_ID_UNICODE_BRACE_CLOSE] = 1;  // \u{X+} complete
}

/**
 * @brief Configures identifier recognition:
 *   [$A-Za-z_\uXXXX\u{X+}][$A-Za-z0-9_\uXXXX\u{X+}]*
 * with UTF-8 and Unicode escape sequence support.
 */
static void dfa_set_identifier_rules(void) {
  // ID start: letter (including 'u'), underscore, $, UTF-8 multi-byte
  dfa_table[STATE_START][CHAR_LETTER] = STATE_IDENTIFIER;
  dfa_table[STATE_START][CHAR_U_LOWER] = STATE_IDENTIFIER;
  dfa_table[STATE_START][CHAR_UNDERSCORE] = STATE_IDENTIFIER;
  dfa_table[STATE_START][CHAR_DOLLAR_SIGN] = STATE_IDENTIFIER;
  dfa_table[STATE_START][CHAR_UTF8_CONTINUATION] = STATE_IDENTIFIER;
  // ID content
  dfa_table[STATE_IDENTIFIER][CHAR_LETTER] = STATE_IDENTIFIER;
  dfa_table[STATE_IDENTIFIER][CHAR_U_LOWER] = STATE_IDENTIFIER;
  dfa_table[STATE_IDENTIFIER][CHAR_UNDERSCORE] = STATE_IDENTIFIER;
  dfa_table[STATE_IDENTIFIER][CHAR_DOLLAR_SIGN] = STATE_IDENTIFIER;
  dfa_table[STATE_IDENTIFIER][CHAR_DIGIT] = STATE_IDENTIFIER;
  dfa_table[STATE_IDENTIFIER][CHAR_UTF8_CONTINUATION] = STATE_IDENTIFIER;
}

/**
 * @brief Configures Unicode escape sequences (\uXXXX and \u{XXXX}) as valid
 * identifier characters, matching the ECMAScript specification.
 *
 * The 4-hex-digit form accepts any letter or digit in each position; strict
 * hex validation (0-9, a-f, A-F) is left to semantic analysis.
 */
static void dfa_set_unicode_escape_rules(void) {
  // '\' starts a unicode escape from the beginning of an identifier or from
  // within one (and from the accepting unicode states themselves).
  dfa_table[STATE_START][CHAR_BACKSLASH] = STATE_ID_BACKSLASH;
  dfa_table[STATE_IDENTIFIER][CHAR_BACKSLASH] = STATE_ID_BACKSLASH;
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_BACKSLASH] = STATE_ID_BACKSLASH;
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_BACKSLASH] = STATE_ID_BACKSLASH;

  // After '\': only 'u' is valid (all other chars stay at STATE_ERROR default)
  dfa_table[STATE_ID_BACKSLASH][CHAR_U_LOWER] = STATE_ID_UNICODE_U;

  // After '\u': '{' for brace form, or first hex digit for fixed form
  dfa_table[STATE_ID_UNICODE_U][CHAR_LBRACE] = STATE_ID_UNICODE_BRACE;
  dfa_table[STATE_ID_UNICODE_U][CHAR_DIGIT] = STATE_ID_UNICODE_HEX1;
  dfa_table[STATE_ID_UNICODE_U][CHAR_LETTER] = STATE_ID_UNICODE_HEX1;
  dfa_table[STATE_ID_UNICODE_U][CHAR_U_LOWER] = STATE_ID_UNICODE_HEX1;

  // Fixed form \uXXXX: consume exactly 4 (hex) characters
  dfa_table[STATE_ID_UNICODE_HEX1][CHAR_DIGIT] = STATE_ID_UNICODE_HEX2;
  dfa_table[STATE_ID_UNICODE_HEX1][CHAR_LETTER] = STATE_ID_UNICODE_HEX2;
  dfa_table[STATE_ID_UNICODE_HEX1][CHAR_U_LOWER] = STATE_ID_UNICODE_HEX2;

  dfa_table[STATE_ID_UNICODE_HEX2][CHAR_DIGIT] = STATE_ID_UNICODE_HEX3;
  dfa_table[STATE_ID_UNICODE_HEX2][CHAR_LETTER] = STATE_ID_UNICODE_HEX3;
  dfa_table[STATE_ID_UNICODE_HEX2][CHAR_U_LOWER] = STATE_ID_UNICODE_HEX3;

  dfa_table[STATE_ID_UNICODE_HEX3][CHAR_DIGIT] = STATE_ID_UNICODE_HEX4;
  dfa_table[STATE_ID_UNICODE_HEX3][CHAR_LETTER] = STATE_ID_UNICODE_HEX4;
  dfa_table[STATE_ID_UNICODE_HEX3][CHAR_U_LOWER] = STATE_ID_UNICODE_HEX4;

  // After \uXXXX: continue as identifier or start another escape
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_LETTER] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_U_LOWER] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_UNDERSCORE] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_DOLLAR_SIGN] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_DIGIT] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_HEX4][CHAR_UTF8_CONTINUATION] = STATE_IDENTIFIER;

  // Brace form \u{X+}: one or more hex digits then '}'
  dfa_table[STATE_ID_UNICODE_BRACE][CHAR_DIGIT] = STATE_ID_UNICODE_BRACE_HEX;
  dfa_table[STATE_ID_UNICODE_BRACE][CHAR_LETTER] = STATE_ID_UNICODE_BRACE_HEX;
  dfa_table[STATE_ID_UNICODE_BRACE][CHAR_U_LOWER] = STATE_ID_UNICODE_BRACE_HEX;

  dfa_table[STATE_ID_UNICODE_BRACE_HEX][CHAR_DIGIT] =
      STATE_ID_UNICODE_BRACE_HEX;
  dfa_table[STATE_ID_UNICODE_BRACE_HEX][CHAR_LETTER] =
      STATE_ID_UNICODE_BRACE_HEX;
  dfa_table[STATE_ID_UNICODE_BRACE_HEX][CHAR_U_LOWER] =
      STATE_ID_UNICODE_BRACE_HEX;
  dfa_table[STATE_ID_UNICODE_BRACE_HEX][CHAR_RBRACE] =
      STATE_ID_UNICODE_BRACE_CLOSE;

  // After \u{X+}: continue as identifier or start another escape
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_LETTER] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_U_LOWER] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_UNDERSCORE] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_DOLLAR_SIGN] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_DIGIT] = STATE_IDENTIFIER;
  dfa_table[STATE_ID_UNICODE_BRACE_CLOSE][CHAR_UTF8_CONTINUATION] =
      STATE_IDENTIFIER;
}

/**
 * @brief Configures integer and decimal number transitions.
 */
static void dfa_set_number_rules(void) {
  // Number start
  dfa_table[STATE_START][CHAR_DIGIT] = STATE_NUMBER;
  // Number content (int)
  dfa_table[STATE_NUMBER][CHAR_DIGIT] = STATE_NUMBER;
  // Float number
  dfa_table[STATE_NUMBER][CHAR_DOT] = STATE_DOT_NUMBER;
  dfa_table[STATE_DOT_NUMBER][CHAR_DIGIT] = STATE_FLOAT_NUMBER;
  dfa_table[STATE_FLOAT_NUMBER][CHAR_DIGIT] = STATE_FLOAT_NUMBER;

  // Invalid numbers
  dfa_table[STATE_DOT_NUMBER][CHAR_DOT] = STATE_ERROR;
  dfa_table[STATE_FLOAT_NUMBER][CHAR_DOT] = STATE_ERROR;
}

/**
 * @brief Sets DFA transitions for quoted and template string literals.
 */
static void dfa_set_string_rules(void) {
  // String start
  dfa_table[STATE_START][CHAR_DOUBLE_QUOTE] = STATE_STRING_DOUBLE;
  dfa_table[STATE_START][CHAR_SINGLE_QUOTE] = STATE_STRING_SINGLE;
  dfa_table[STATE_START][CHAR_BACKTICK] = STATE_STRING_BACKTICK;

  // By default, strings continue consuming characters until a delimiter changes
  // state.
  for (int ctype = 0; ctype < _CHAR_ENUM_COUNT; ctype++) {
    dfa_table[STATE_STRING_DOUBLE][ctype] = STATE_STRING_DOUBLE;
    dfa_table[STATE_STRING_SINGLE][ctype] = STATE_STRING_SINGLE;
    dfa_table[STATE_STRING_BACKTICK][ctype] = STATE_STRING_BACKTICK;

    dfa_table[STATE_STRING_DOUBLE_ESCAPE][ctype] = STATE_STRING_DOUBLE;
    dfa_table[STATE_STRING_SINGLE_ESCAPE][ctype] = STATE_STRING_SINGLE;
    dfa_table[STATE_STRING_BACKTICK_ESCAPE][ctype] = STATE_STRING_BACKTICK;
  }

  // Closing delimiters
  dfa_table[STATE_STRING_DOUBLE][CHAR_DOUBLE_QUOTE] = STATE_DONE;
  dfa_table[STATE_STRING_SINGLE][CHAR_SINGLE_QUOTE] = STATE_DONE;
  dfa_table[STATE_STRING_BACKTICK][CHAR_BACKTICK] = STATE_DONE;

  // Escape handling
  dfa_table[STATE_STRING_DOUBLE][CHAR_BACKSLASH] = STATE_STRING_DOUBLE_ESCAPE;
  dfa_table[STATE_STRING_SINGLE][CHAR_BACKSLASH] = STATE_STRING_SINGLE_ESCAPE;
  dfa_table[STATE_STRING_BACKTICK][CHAR_BACKSLASH] =
      STATE_STRING_BACKTICK_ESCAPE;

  // Invalid literal termination
  dfa_table[STATE_STRING_DOUBLE][CHAR_EOF] = STATE_ERROR;
  dfa_table[STATE_STRING_SINGLE][CHAR_EOF] = STATE_ERROR;
  dfa_table[STATE_STRING_BACKTICK][CHAR_EOF] = STATE_ERROR;

  dfa_table[STATE_STRING_DOUBLE][CHAR_NEWLINE] = STATE_ERROR;
  dfa_table[STATE_STRING_SINGLE][CHAR_NEWLINE] = STATE_ERROR;

  dfa_table[STATE_STRING_DOUBLE_ESCAPE][CHAR_EOF] = STATE_ERROR;
  dfa_table[STATE_STRING_SINGLE_ESCAPE][CHAR_EOF] = STATE_ERROR;
  dfa_table[STATE_STRING_BACKTICK_ESCAPE][CHAR_EOF] = STATE_ERROR;
}

/**
 * @brief Sets DFA transitions for regex literals delimited by slashes.
 *
 * Regex scanning is intentionally separated from the division operator path so
 * the caller can choose the correct entry state based on context.
 */
static void dfa_set_regex_rules(void) {
  for (int ctype = 0; ctype < _CHAR_ENUM_COUNT; ctype++) {
    dfa_table[STATE_REGEX_BODY][ctype] = STATE_REGEX_BODY;
    dfa_table[STATE_REGEX_CLASS][ctype] = STATE_REGEX_CLASS;

    dfa_table[STATE_REGEX_BODY_ESCAPE][ctype] = STATE_REGEX_BODY;
    dfa_table[STATE_REGEX_CLASS_ESCAPE][ctype] = STATE_REGEX_CLASS;
  }

  dfa_table[STATE_REGEX_BODY][CHAR_SLASH] = STATE_DONE;
  dfa_table[STATE_REGEX_BODY][CHAR_BACKSLASH] = STATE_REGEX_BODY_ESCAPE;
  dfa_table[STATE_REGEX_BODY][CHAR_LBRACKET] = STATE_REGEX_CLASS;

  dfa_table[STATE_REGEX_CLASS][CHAR_RBRACKET] = STATE_REGEX_BODY;
  dfa_table[STATE_REGEX_CLASS][CHAR_BACKSLASH] = STATE_REGEX_CLASS_ESCAPE;

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
 * @brief Configures operators and punctuation transitions, including
 * lookahead-based variants.
 */
static void dfa_set_operator_rules(void) {
  // Operator start
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

  // Delimiters and single-char punctuators are terminal in one step.
  dfa_table[STATE_START][CHAR_COMMA] = STATE_DONE;
  dfa_table[STATE_START][CHAR_SEMICOLON] = STATE_DONE;
  dfa_table[STATE_START][CHAR_COLON] = STATE_DONE;
  dfa_table[STATE_START][CHAR_LBRACE] = STATE_DONE;
  dfa_table[STATE_START][CHAR_RBRACE] = STATE_DONE;
  dfa_table[STATE_START][CHAR_LBRACKET] = STATE_DONE;
  dfa_table[STATE_START][CHAR_RBRACKET] = STATE_DONE;
  dfa_table[STATE_START][CHAR_LPAREN] = STATE_DONE;
  dfa_table[STATE_START][CHAR_RPAREN] = STATE_DONE;

  // Operator ambiguous
  dfa_table[STATE_PLUS][CHAR_PLUS] = STATE_DONE;   // ++
  dfa_table[STATE_PLUS][CHAR_EQUAL] = STATE_DONE;  // +=

  dfa_table[STATE_MINUS][CHAR_MINUS] = STATE_DONE;  // --
  dfa_table[STATE_MINUS][CHAR_EQUAL] = STATE_DONE;  // -=

  dfa_table[STATE_STAR][CHAR_STAR] = STATE_STAR_STAR;
  dfa_table[STATE_STAR][CHAR_EQUAL] = STATE_DONE;  // *=

  dfa_table[STATE_STAR_STAR][CHAR_EQUAL] = STATE_DONE;  // **=

  dfa_table[STATE_SLASH][CHAR_EQUAL] = STATE_DONE;  // /=

  dfa_table[STATE_PERCENT][CHAR_EQUAL] = STATE_DONE;  // %=

  dfa_table[STATE_EQUAL][CHAR_EQUAL] = STATE_EQUAL_EQUAL;
  dfa_table[STATE_EQUAL][CHAR_GREATER] = STATE_DONE;  // =>

  dfa_table[STATE_EQUAL_EQUAL][CHAR_EQUAL] = STATE_DONE;  // ===

  dfa_table[STATE_BANG][CHAR_EQUAL] = STATE_BANG_EQUAL;
  dfa_table[STATE_BANG_EQUAL][CHAR_EQUAL] = STATE_DONE;  // !==

  dfa_table[STATE_LESS][CHAR_EQUAL] = STATE_DONE;     // <=
  dfa_table[STATE_GREATER][CHAR_EQUAL] = STATE_DONE;  // >=

  dfa_table[STATE_AMPERSAND][CHAR_AMPERSAND] = STATE_DONE;  // &&

  dfa_table[STATE_PIPE][CHAR_PIPE] = STATE_DONE;  // ||

  dfa_table[STATE_QUESTION][CHAR_QUESTION] = STATE_DONE;  // ??

  dfa_table[STATE_DOT][CHAR_DOT] = STATE_DOT_DOT;   // ..
  dfa_table[STATE_DOT_DOT][CHAR_DOT] = STATE_DONE;  // ...
}

/**
 * @brief Handles ignorable layout at start state.
 */
static void dfa_set_layout_rules(void) {
  dfa_table[STATE_START][CHAR_WHITESPACE] = STATE_START;
  dfa_table[STATE_START][CHAR_NEWLINE] = STATE_START;
}

void dfa_init(void) {
  // Behavior no defined causes error and is not accepting state
  dfa_set_all(STATE_ERROR);
  dfa_init_accepting_states();

  dfa_set_layout_rules();
  dfa_set_identifier_rules();
  dfa_set_unicode_escape_rules();
  dfa_set_number_rules();
  dfa_set_string_rules();
  dfa_set_regex_rules();
  dfa_set_operator_rules();
}

LexerState dfa_regex_entry_state(void) { return STATE_REGEX_BODY; }

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

int dfa_is_accepting(const LexerState state) {
  if (state < 0 || state >= _STATE_ENUM_COUNT) {
    return 0;
  }
  return dfa_accepting_state[state];
}