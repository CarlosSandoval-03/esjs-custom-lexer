/**
 * @file main.c
 * @brief Command-line driver for the EsJS lexer.
 *
 * Reads EsJS source code from standard input, tokenizes it using the lexer
 * pipeline, and prints each token to standard output in one of the following
 * formats:
 *
 *   Keyword token:
 *     <keyword_text,line,column>
 *     Example: <funcion,1,1>
 *
 *   Value-bearing token (identifier, number, string, regex):
 *     <tag,lexeme,line,column>
 *     Example: <id,miVariable,1,9>
 *              <tkn_num,42,2,5>
 *              <tkn_str,hola mundo,3,1>
 *
 *   Operator / punctuation token:
 *     <tag,line,column>
 *     Example: <tkn_plus,1,6>
 *
 *   Lexical error:
 *     >>> Error lexico (linea: L, posicion: C)
 *     Processing stops immediately after the first lexical error.
 *
 * Exit codes:
 *   0 - Input was tokenized successfully (no lexical errors).
 *   0 - Processing was halted by a lexical error (the error message was
 *       already printed; the caller may inspect stderr for details).
 *
 * Usage:
 * @code
 *   echo "var x = 42;" | ./esjs_custom_lexer
 * @endcode
 */
#include <stdio.h>

#include "../include/lexer.h"

/**
 * @brief Formats and prints a single token to stdout.
 *
 * Output format depends on the token type:
 *  - TOKEN_ERROR: prints a Spanish-language lexical error message and returns
 *    -1 to signal the caller that processing should stop.
 *  - TOKEN_KEYWORD: prints `<lexeme,line,column>` (the keyword text is used
 *    directly as the tag, without a generic "keyword" prefix).
 *  - TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_STRING, TOKEN_REGEX: prints
 *    `<tag,lexeme,line,column>`.
 *  - All other tokens: prints `<tag,line,column>` (no lexeme field).
 *
 * @param token The token to print. Must not be NULL.
 * @return      0 on success; -1 when the token is TOKEN_ERROR.
 */
static int print_token(const Token *token) {
  const char *name = token_type_to_string(token->type);

  if (token->type == TOKEN_ERROR) {
    printf(">>> Error lexico (linea: %d, posicion: %d)\n", token->line,
           token->column);
    return -1;
  }

  /* Keyword: the reserved word text is the tag. */
  if (token->type == TOKEN_KEYWORD) {
    printf("<%.*s,%d,%d>\n", (int)token->lexeme_length, token->lexeme_start,
           token->line, token->column);
    return 0;
  }

  /* Value-bearing tokens: include the lexeme in the output. */
  if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_NUMBER ||
      token->type == TOKEN_STRING || token->type == TOKEN_REGEX) {
    printf("<%s,%.*s,%d,%d>\n", name, (int)token->lexeme_length,
           token->lexeme_start, token->line, token->column);
    return 0;
  }

  /* Punctuation and operator tokens: tag and position only. */
  printf("<%s,%d,%d>\n", name, token->line, token->column);
  return 0;
}

/**
 * @brief Entry point: initializes the pipeline and drives the token loop.
 *
 * Initialization order:
 *  1. buffer_init() - creates a lazy buffer over stdin.
 *  2. lexer_init()  - wires the lexer to the buffer (calls dfa_init() internally).
 *
 * Token loop:
 *  Calls lexer_next_token() repeatedly until TOKEN_EOF is encountered or
 *  print_token() returns -1 (lexical error). The buffer is destroyed before
 *  returning to release heap memory.
 *
 * @return 0 in all cases; error conditions are reported via stdout.
 */
int main(void) {
  Buffer buffer;
  Lexer  lexer;
  Token  token;

  buffer_init(&buffer, stdin);
  lexer_init(&lexer, &buffer);

  while (lexer_next_token(&lexer, &token)) {
    if (token.type == TOKEN_EOF) {
      break;
    }

    /* Stop analysis on the first lexical error. */
    const int res = print_token(&token);
    if (res < 0) break;
  }

  buffer_destroy(&buffer);
  return 0;
}