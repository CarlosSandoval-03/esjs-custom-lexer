/**
 * @file dfa.h
 * @brief Deterministic Finite Automaton (DFA) used as the core recognition
 *        engine of the EsJS lexer.
 *
 * The DFA is implemented as a 2-D transition table indexed by
 * (LexerState, CharClass). Every cell holds the next state to enter upon
 * consuming a character of that class. Cells left unset default to
 * STATE_ERROR, which halts scanning for that token.
 *
 * A parallel boolean array marks which states are *accepting*, i.e. which
 * states correspond to a complete, valid token. The scanner uses this
 * information together with the maximal-munch rule: it records the longest
 * prefix ending in an accepting state.
 *
 * Design decisions:
 *  - The table is initialized once at program startup via dfa_init(); no
 *    dynamic allocation is performed after that point.
 *  - Regex recognition enters through a dedicated entry state
 *    (STATE_REGEX_BODY) rather than STATE_START. This avoids conflating the
 *    `/` division operator with the start of a regex literal; the scanner
 *    chooses the appropriate entry point based on contextual hints.
 *  - Standalone `&` and `|` are intentionally non-accepting; only `&&` and
 *    `||` are valid, matching EsJS semantics.
 *  - Unicode escape sequences (\uXXXX and \u{X+}) are recognized directly
 *    in the automaton so identifiers containing them are tokenized correctly
 *    without a separate pre-processing pass.
 */
#ifndef ESJS_CUSTOM_LEXER_H
#define ESJS_CUSTOM_LEXER_H

#include "charclass.h"

/**
 * @brief Enumeration of all DFA states.
 *
 * States are organized into functional groups:
 *
 *  - STATE_START        : entry point; also used to skip whitespace and
 *                         newlines (self-loop transitions).
 *  - Identifier states  : STATE_IDENTIFIER and the Unicode-escape chain
 *                         STATE_ID_BACKSLASH ... STATE_ID_UNICODE_BRACE_CLOSE.
 *  - Number states      : STATE_NUMBER, STATE_DOT_NUMBER, STATE_FLOAT_NUMBER.
 *  - String states      : one body state and one escape state per quote style
 *                         (double, single, backtick).
 *  - Regex states       : STATE_REGEX_BODY / _ESCAPE and
 *                         STATE_REGEX_CLASS / _ESCAPE.
 *  - Operator prefix states: one state per operator character that may be
 *                         the start of a longer token (e.g. STATE_STAR leads
 *                         to STATE_STAR_STAR for `**`).
 *  - Longer ambiguous variants: STATE_STAR_STAR, STATE_EQUAL_EQUAL,
 *                         STATE_BANG_EQUAL, STATE_DOT_DOT.
 *  - Terminal states    : STATE_DONE (single-character tokens that are
 *                         fully determined in one step) and STATE_ERROR.
 *
 * @warning _STATE_ENUM_COUNT must remain the last entry; it is used to size
 *          the DFA transition table.
 */
typedef enum {
  STATE_START, /**< Initial state; skips whitespace via self-loop.          */

  /* ---- Basic lexeme states ---------------------------------------------- */
  STATE_IDENTIFIER,   /**< Inside an identifier (letter/digit/_ sequence). */
  STATE_NUMBER,       /**< Inside an integer literal.                       */
  STATE_DOT_NUMBER,   /**< After `N.` — accepting (e.g. `10.`).            */
  STATE_FLOAT_NUMBER, /**< After `N.M` — accepting float literal.          */

  /* ---- String literal states -------------------------------------------- */
  STATE_STRING_DOUBLE,         /**< Inside a `"..."` string body.           */
  STATE_STRING_DOUBLE_ESCAPE,  /**< After `\` inside a double-quoted string.*/
  STATE_STRING_SINGLE,         /**< Inside a `'...'` string body.           */
  STATE_STRING_SINGLE_ESCAPE,  /**< After `\` inside a single-quoted string.*/
  STATE_STRING_BACKTICK,       /**< Inside a `` `...` `` template literal.  */
  STATE_STRING_BACKTICK_ESCAPE,/**< After `\` inside a template literal.    */

  /* ---- Regex literal states --------------------------------------------- */
  STATE_REGEX_BODY,         /**< Inside the regex pattern `/...`.           */
  STATE_REGEX_BODY_ESCAPE,  /**< After `\` inside a regex pattern.          */
  STATE_REGEX_CLASS,        /**< Inside a regex character class `/[...]`.   */
  STATE_REGEX_CLASS_ESCAPE, /**< After `\` inside a regex character class.  */

  /* ---- Single-char operator prefix states (ambiguous; need lookahead) --- */
  STATE_PLUS,      /**< Seen `+`; may become `++` or `+=`.                  */
  STATE_MINUS,     /**< Seen `-`; may become `--` or `-=`.                  */
  STATE_STAR,      /**< Seen `*`; may become `**`, `**=`, or `*=`.          */
  STATE_SLASH,     /**< Seen `/`; may become `/=` or division operator.     */
  STATE_PERCENT,   /**< Seen `%`; may become `%=`.                          */
  STATE_EQUAL,     /**< Seen `=`; may become `==`, `===`, or `=>`.          */
  STATE_BANG,      /**< Seen `!`; may become `!=` or `!==`.                 */
  STATE_LESS,      /**< Seen `<`; may become `<=`.                          */
  STATE_GREATER,   /**< Seen `>`; may become `>=`.                          */
  STATE_AMPERSAND, /**< Seen `&`; non-accepting — only `&&` is valid.       */
  STATE_PIPE,      /**< Seen `|`; non-accepting — only `||` is valid.       */
  STATE_QUESTION,  /**< Seen `?`; may become `??`.                          */
  STATE_DOT,       /**< Seen `.`; may become `..` (partial) or `...`.       */

  /* ---- Two-character operator intermediate states ----------------------- */
  STATE_STAR_STAR,   /**< Seen `**`; may become `**=`.                      */
  STATE_EQUAL_EQUAL, /**< Seen `==`; may become `===`.                      */
  STATE_BANG_EQUAL,  /**< Seen `!=`; may become `!==`.                      */
  STATE_DOT_DOT,     /**< Seen `..`; must be followed by `.` to form `...`. */

  /* ---- Unicode escape sequence chain in identifiers -------------------- */
  STATE_ID_BACKSLASH,           /**< Seen `\` at identifier start or within.*/
  STATE_ID_UNICODE_U,           /**< Seen `\u`.                              */
  STATE_ID_UNICODE_HEX1,        /**< Seen `\uX` (1st hex digit).            */
  STATE_ID_UNICODE_HEX2,        /**< Seen `\uXX` (2nd hex digit).           */
  STATE_ID_UNICODE_HEX3,        /**< Seen `\uXXX` (3rd hex digit).          */
  STATE_ID_UNICODE_HEX4,        /**< Seen `\uXXXX` — accepting.             */
  STATE_ID_UNICODE_BRACE,       /**< Seen `\u{` — brace form started.       */
  STATE_ID_UNICODE_BRACE_HEX,   /**< Seen `\u{X+` — one or more hex digits. */
  STATE_ID_UNICODE_BRACE_CLOSE, /**< Seen `\u{X+}` — accepting.             */

  /* ---- Terminal states -------------------------------------------------- */
  STATE_DONE,  /**< Accepting: token is complete (used by single-char and
                    known multi-char tokens whose type needs no lookahead).  */
  STATE_ERROR, /**< Non-accepting: invalid input or undefined transition.   */

  /**
   * @internal Sentinel for table dimensioning. Must remain the last entry.
   */
  _STATE_ENUM_COUNT
} LexerState;

/**
 * @brief Constructs and populates the DFA transition table and accepting-state
 *        metadata.
 *
 * Must be called once before any lexer operation. In practice this happens
 * automatically inside lexer_init(). The function:
 *  1. Fills the entire table with STATE_ERROR as the default.
 *  2. Marks accepting states.
 *  3. Installs transition rules for each token category (layout, identifiers,
 *     Unicode escapes, numbers, strings, regex, operators).
 *
 * The function is idempotent: subsequent calls after the first are no-ops.
 *
 * @warning **Not thread-safe during initialization.** There is a TOCTOU race:
 *          two threads calling dfa_init() (or lexer_init()) simultaneously for
 *          the first time can both pass the `dfa_initialized` guard and write
 *          the tables concurrently, producing a partially corrupt automaton.
 *          Call dfa_init() (or one lexer_init()) from the main thread before
 *          spawning any worker threads. After initialization completes the
 *          tables are read-only and safe for concurrent access.
 */
void dfa_init(void);

/**
 * @brief Returns the DFA entry state used to scan a regex literal body.
 *
 * The regex body state (STATE_REGEX_BODY) is separated from STATE_START so
 * the scanner can choose between the division-operator path and the regex
 * path without encoding that decision in the automaton itself.
 *
 * @return LexerState The initial state for regex literal scanning.
 */
LexerState dfa_regex_entry_state(void);

/**
 * @brief Performs one transition in the DFA.
 *
 * Looks up @p current × @p input_class in the transition table and returns
 * the resulting state. Returns STATE_ERROR for any out-of-range arguments.
 *
 * @param current     Current state of the automaton.
 * @param input_class Character class of the consumed input symbol.
 * @return            The next state, or STATE_ERROR on invalid input.
 */
LexerState dfa_next_state(LexerState current, CharClass input_class);

/**
 * @brief Tests whether a state is an accepting state.
 *
 * An accepting state indicates that the input consumed so far forms a
 * complete, valid token. The scanner uses this to implement maximal munch:
 * it continues advancing and updates the last-accepted position whenever
 * it lands on an accepting state.
 *
 * @param state State to test.
 * @return      Non-zero (true) if the state is accepting; 0 otherwise.
 */
int dfa_is_accepting(LexerState state);

#endif  // ESJS_CUSTOM_LEXER_H