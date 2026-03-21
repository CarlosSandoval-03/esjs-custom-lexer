#ifndef ESJS_CUSTOM_LEXER_H
#define ESJS_CUSTOM_LEXER_H

typedef enum {
    STATE_START,

    // Basic States
    STATE_IDENTIFIER,
    STATE_NUMBER,
    STATE_DOT_NUMER,    // This supports float numbers

    // String States
    STATE_STRING_DOUBLE,            // "..."
    STATE_STRING_DOUBLE_ESCAPE,
    STATE_STRING_SINGLE,            // '...'
    STATE_STRING_SINGLE_ESCAPE,
    STATE_STRING_BACKTICK,          // `...`
    STATE_STRING_BACKTICK_ESCAPE,

    // Ambiguous prefix (needs lookahead)
    STATE_PLUS,
    STATE_MINUS,
    STATE_STAR,         // *
    STATE_SLASH,
    STATE_PERCENT,
    STATE_EQUAL,
    STATE_BANG,         // !
    STATE_LESS,
    STATE_GREATER,
    STATE_AMPERSAND,    // &
    STATE_PIPE,         // |
    STATE_QUESTION,
    STATE_DOT,          // This state requires lookahead of 2 chars!

    // Longer Ambiguous Variants
    STATE_STAR_STAR,    // **
    STATE_EQUAL_EQUAL,  // ==
    STATE_BANG_EQUAL,   // !=

    // Final States
    STATE_DONE,
    STATE_ERROR,
} LexerState;

#endif // ESJS_CUSTOM_LEXER_H