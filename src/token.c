/**
 * @file token.c
 * @brief Token-type to string mapping used by the CLI output formatter.
 *
 * TOKEN_TYPE_NAMES is a designated-initializer array that maps each TokenType
 * enum value to the short string tag used in the lexer output format:
 *
 *   <tag,lexeme,line,column>   for value-bearing tokens
 *   <tag,line,column>          for punctuation and operator tokens
 *   <keyword,line,column>      for reserved-word tokens (tag is the word)
 *
 * Entries for TOKEN_EOF and TOKEN_ERROR are intentionally empty strings
 * because those conditions are handled specially by the CLI driver (they
 * either terminate iteration or print an error message rather than producing
 * a standard output line).
 *
 * Keyword tokens (TOKEN_KEYWORD) are also absent from this table because the
 * CLI driver prints the keyword text itself (the lexeme) as the tag, not a
 * generic "keyword" label.
 *
 * @note This table must be updated whenever a new TokenType is added to the
 *       enum in token.h. Failing to do so leaves the entry as NULL, which
 *       token_type_to_string() converts to an empty string.
 */
#include "../include/token.h"

/**
 * @brief Designated-initializer mapping from TokenType to output tag string.
 *
 * Array size is _TOKEN_TYPE_ENUM_COUNT so it covers every valid enum value.
 * Uninitialized slots default to NULL, which token_type_to_string() treats
 * as an empty string.
 */
static const char *TOKEN_TYPE_NAMES[] = {
    /* Literal value tokens */
    [TOKEN_IDENTIFIER] = "id",
    [TOKEN_NUMBER]     = "tkn_num",
    [TOKEN_STRING]     = "tkn_str",
    [TOKEN_REGEX]      = "tkn_reg",

    /* Arithmetic operators */
    [TOKEN_PLUS]   = "tkn_plus",
    [TOKEN_MINUS]  = "tkn_minus",
    [TOKEN_TIMES]  = "tkn_times",
    [TOKEN_DIVIDE] = "tkn_div",
    [TOKEN_POWER]  = "tkn_power",
    [TOKEN_MOD]    = "tkn_mod",

    /* Assignment operators */
    [TOKEN_ASSIGN]        = "tkn_assign",
    [TOKEN_MOD_ASSIGN]    = "tkn_mod_assign",
    [TOKEN_DIV_ASSIGN]    = "tkn_div_assign",
    [TOKEN_TIMES_ASSIGN]  = "tkn_times_assign",
    [TOKEN_MINUS_ASSIGN]  = "tkn_minus_assign",
    [TOKEN_PLUS_ASSIGN]   = "tkn_plus_assign",
    [TOKEN_POWER_ASSIGN]  = "tkn_power_assign",

    /* Equality operators */
    [TOKEN_EQUAL]            = "tkn_equal",
    [TOKEN_STRICT_EQUAL]     = "tkn_strict_equal",
    [TOKEN_NOT_EQUAL]        = "tkn_neq",
    [TOKEN_STRICT_NOT_EQUAL] = "tkn_strict_neq",

    /* Relational operators */
    [TOKEN_LESS]          = "tkn_less",
    [TOKEN_LESS_EQUAL]    = "tkn_leq",
    [TOKEN_GREATER]       = "tkn_greater",
    [TOKEN_GREATER_EQUAL] = "tkn_geq",

    /* Logical operators */
    [TOKEN_LOGICAL_AND] = "tkn_and",
    [TOKEN_LOGICAL_OR]  = "tkn_or",

    /* Unary and bitwise operators */
    [TOKEN_NOT]     = "tkn_not",
    [TOKEN_BIT_AND] = "tkn_bit_and",  /* TODO: verify tag name */
    [TOKEN_BIT_OR]  = "tkn_bit_or",
    [TOKEN_BIT_XOR] = "tkn_bit_xor",
    [TOKEN_BIT_NOT] = "tkn_bit_not",

    /* Punctuation */
    [TOKEN_SPREAD]    = "tkn_spread",
    [TOKEN_PERIOD]    = "tkn_period",
    [TOKEN_COMMA]     = "tkn_comma",
    [TOKEN_SEMICOLON] = "tkn_semicolon",
    [TOKEN_COLON]     = "tkn_colon",

    /* Grouping tokens */
    [TOKEN_OPENING_KEY] = "tkn_opening_key",
    [TOKEN_CLOSING_KEY] = "tkn_closing_key",
    [TOKEN_OPENING_BRA] = "tkn_opening_bra",
    [TOKEN_CLOSING_BRA] = "tkn_closing_bra",
    [TOKEN_OPENING_PAR] = "tkn_opening_par",
    [TOKEN_CLOSING_PAR] = "tkn_closing_par",

    /* Compound and special operators */
    [TOKEN_INCREMENT] = "tkn_increment",
    [TOKEN_DECREMENT] = "tkn_decrement",
    [TOKEN_ARROW]     = "tkn_arrow",
    [TOKEN_TERNARY]   = "tkn_ternary",
    [TOKEN_NULISH]    = "tkn_nulish",

    /* Sentinels: no output tag (handled specially by the CLI). */
    [TOKEN_EOF]   = "",
    [TOKEN_ERROR] = ""
};

/**
 * @brief Returns the output tag string for a given TokenType.
 *
 * Returns an empty string for out-of-range values, TOKEN_EOF, TOKEN_ERROR,
 * and any slot that was left uninitialized in TOKEN_TYPE_NAMES. The return
 * value is a pointer to static memory and must not be freed or modified.
 *
 * @param type The token type to look up.
 * @return     Null-terminated string tag, or "" if not found. Never NULL.
 */
const char *token_type_to_string(const TokenType type) {
  if (type < 0 || type >= _TOKEN_TYPE_ENUM_COUNT) {
    return "";
  }
  return TOKEN_TYPE_NAMES[type] ? TOKEN_TYPE_NAMES[type] : "";
}
