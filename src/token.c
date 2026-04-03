#include "../include/token.h"

static const char *TOKEN_TYPE_NAMES[] = {
    [TOKEN_IDENTIFIER] = "id",
    [TOKEN_NUMBER] = "tkn_num",
    [TOKEN_STRING] = "tkn_str",
    [TOKEN_REGEX] = "tkn_reg",

    [TOKEN_PLUS] = "tkn_plus",
    [TOKEN_MINUS] = "tkn_minus",
    [TOKEN_TIMES] = "tkn_times",
    [TOKEN_DIVIDE] = "tkn_div",
    [TOKEN_POWER] = "tkn_power",
    [TOKEN_MOD] = "tkn_mod",

    [TOKEN_ASSIGN] = "tkn_assign",
    [TOKEN_MOD_ASSIGN] = "tkn_mod_assign",
    [TOKEN_DIV_ASSIGN] = "tkn_div_assign",
    [TOKEN_TIMES_ASSIGN] = "tkn_times_assign",
    [TOKEN_MINUS_ASSIGN] = "tkn_minus_assign",
    [TOKEN_PLUS_ASSIGN] = "tkn_plus_assign",
    [TOKEN_POWER_ASSIGN] = "tkn_power_assign",

    [TOKEN_EQUAL] = "tkn_equal",
    [TOKEN_STRICT_EQUAL] = "tkn_strict_equal",
    [TOKEN_NOT_EQUAL] = "tkn_neq",
    [TOKEN_STRICT_NOT_EQUAL] = "tkn_strict_neq",

    [TOKEN_LESS] = "tkn_less",
    [TOKEN_LESS_EQUAL] = "tkn_leq",
    [TOKEN_GREATER] = "tkn_greater",
    [TOKEN_GREATER_EQUAL] = "tkn_geq",

    [TOKEN_LOGICAL_AND] = "tkn_and",
    [TOKEN_LOGICAL_OR] = "tkn_or",

    [TOKEN_NOT] = "tkn_not",
    // TODO: Name not defined, verify
    [TOKEN_BIT_AND] = "tkn_bit_and",
    [TOKEN_BIT_OR] = "tkn_bit_or",
    [TOKEN_BIT_XOR] = "tkn_bit_xor",
    [TOKEN_BIT_NOT] = "tkn_bit_not",

    [TOKEN_SPREAD] = "tkn_spread",
    [TOKEN_PERIOD] = "tkn_period",
    [TOKEN_COMMA] = "tkn_comma",
    [TOKEN_SEMICOLON] = "tkn_semicolon",
    [TOKEN_COLON] = "tkn_colon",

    [TOKEN_OPENING_KEY] = "tkn_opening_key",
    [TOKEN_CLOSING_KEY] = "tkn_closing_key",
    [TOKEN_OPENING_BRA] = "tkn_opening_bra",
    [TOKEN_CLOSING_BRA] = "tkn_closing_bra",
    [TOKEN_OPENING_PAR] = "tkn_opening_par",
    [TOKEN_CLOSING_PAR] = "tkn_closing_par",

    [TOKEN_INCREMENT] = "tkn_increment",
    [TOKEN_DECREMENT] = "tkn_decrement",

    [TOKEN_ARROW] = "tkn_arrow",
    [TOKEN_TERNARY] = "tkn_ternary",
    [TOKEN_NULISH] = "tkn_nulish",

    [TOKEN_EOF] = "",
    [TOKEN_ERROR] = ""};

const char *token_type_to_string(const TokenType type) {
  if (type < 0 || type >= _TOKEN_TYPE_ENUM_COUNT) {
    return "";
  }
  return TOKEN_TYPE_NAMES[type] ? TOKEN_TYPE_NAMES[type] : "";
}