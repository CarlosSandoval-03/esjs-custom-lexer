#ifndef ESJS_CUSTOM_LEXER_H
#define ESJS_CUSTOM_LEXER_H

#include "charclass.h"

typedef enum {
  STATE_START,

  // Basic States
  STATE_IDENTIFIER,
  STATE_NUMBER,
  // This supports float numbers
  STATE_DOT_NUMBER,
  STATE_FLOAT_NUMBER,

  // String States
  STATE_STRING_DOUBLE,  // "..."
  STATE_STRING_DOUBLE_ESCAPE,
  STATE_STRING_SINGLE,  // '...'
  STATE_STRING_SINGLE_ESCAPE,
  STATE_STRING_BACKTICK,  // `...`
  STATE_STRING_BACKTICK_ESCAPE,

  // Regex States
  STATE_REGEX_BODY,  // /.../
  STATE_REGEX_BODY_ESCAPE,
  STATE_REGEX_CLASS,  // /[...]/
  STATE_REGEX_CLASS_ESCAPE,

  // Ambiguous prefix (needs lookahead)
  STATE_PLUS,
  STATE_MINUS,
  STATE_STAR,  // *
  STATE_SLASH,
  STATE_PERCENT,
  STATE_EQUAL,
  STATE_BANG,  // !
  STATE_LESS,
  STATE_GREATER,
  STATE_AMPERSAND,  // &
  STATE_PIPE,       // |
  STATE_QUESTION,
  STATE_DOT,

  // Longer ambiguous variants
  STATE_STAR_STAR,    // **
  STATE_EQUAL_EQUAL,  // ==
  STATE_BANG_EQUAL,   // !=
  STATE_DOT_DOT,      // ..

  // Unicode escape sequence in identifiers: \uXXXX or \u{XXXX}
  STATE_ID_BACKSLASH,            // seen '\'
  STATE_ID_UNICODE_U,            // seen '\u'
  STATE_ID_UNICODE_HEX1,         // seen '\uX'
  STATE_ID_UNICODE_HEX2,         // seen '\uXX'
  STATE_ID_UNICODE_HEX3,         // seen '\uXXX'
  STATE_ID_UNICODE_HEX4,         // seen '\uXXXX' — accepting
  STATE_ID_UNICODE_BRACE,        // seen '\u{'
  STATE_ID_UNICODE_BRACE_HEX,    // seen '\u{X+'
  STATE_ID_UNICODE_BRACE_CLOSE,  // seen '\u{X+}' — accepting

  // Aux final states
  STATE_DONE,
  STATE_ERROR,

  _STATE_ENUM_COUNT  // // This value is used to keep track of the number of
                     // elements
} LexerState;

/**
 * @brief Initializes the DFA transition table and accepting-state metadata.
 */
void dfa_init(void);

/**
 * @brief Returns the DFA state that should be used as the entry point for a
 * regex literal scan.
 *
 * This keeps regex support separate from the division operator path.
 *
 * @return LexerState Initial regex state.
 */
LexerState dfa_regex_entry_state(void);

/**
 * @brief Returns the next DFA state based on current state and input class.
 *
 * @param current Current state in the automaton.
 * @param input_class Character class for the consumed input.
 * @return LexerState Next state, or STATE_ERROR on invalid transitions/inputs.
 */
LexerState dfa_next_state(LexerState current, CharClass input_class);

/**
 * @brief Indicates whether a state is accepting.
 *
 * @param state State to inspect.
 * @return int Non-zero when the state is accepting, 0 otherwise.
 */
int dfa_is_accepting(LexerState state);

#endif  // ESJS_CUSTOM_LEXER_H