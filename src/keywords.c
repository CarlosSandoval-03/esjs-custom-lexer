#include "../include/keywords.h"

#include <string.h>

#define KEYWORD_LITERAL(value) value,
static const char *const KEYWORDS[] = {ESJS_KEYWORDS(KEYWORD_LITERAL)};
#undef KEYWORD_LITERAL

TokenType keyword_lookup(const char *lexeme, const size_t length) {
    for (size_t i = 0; i < sizeof(KEYWORDS) / sizeof(KEYWORDS[0]); i++) {
        if (strlen(KEYWORDS[i]) == length &&
            strncmp(KEYWORDS[i], lexeme, length) == 0) {
            return TOKEN_KEYWORD;
        }
    }

    return TOKEN_IDENTIFIER;
}