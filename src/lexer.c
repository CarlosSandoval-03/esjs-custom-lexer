#include "../include/lexer.h"

#include "../include/keywords.h"

static void lexer_skip_layout_and_comments(Lexer *lexer) {
  for (;;) {
    const int c = scanner_peek(&lexer->scanner, 0);
    if (c == EOF) {
      return;
    }

    if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' ||
        c == '\v') {
      scanner_next(&lexer->scanner);
      continue;
    }

    if (c == '/' && scanner_peek(&lexer->scanner, 1) == '/') {
      scanner_next(&lexer->scanner);
      scanner_next(&lexer->scanner);
      while (scanner_peek(&lexer->scanner, 0) != EOF &&
             scanner_peek(&lexer->scanner, 0) != '\n') {
        scanner_next(&lexer->scanner);
      }
      continue;
    }

    if (c == '/' && scanner_peek(&lexer->scanner, 1) == '*') {
      scanner_next(&lexer->scanner);
      scanner_next(&lexer->scanner);
      while (scanner_peek(&lexer->scanner, 0) != EOF) {
        if (scanner_peek(&lexer->scanner, 0) == '*' &&
            scanner_peek(&lexer->scanner, 1) == '/') {
          scanner_next(&lexer->scanner);
          scanner_next(&lexer->scanner);
          break;
        }
        scanner_next(&lexer->scanner);
      }
      continue;
    }

    return;
  }
}

static TokenType lexer_token_type_from_lexeme(const char *lexeme, size_t length,
                                              ScannerContext context) {
  const char first = lexeme[0];

  if (context == SCANNER_CONTEXT_EXPECT_REGEX) {
    return TOKEN_REGEX;
  }

  if (first == '"' || first == '\'' || first == '`') {
    return TOKEN_STRING;
  }

  if ((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z') ||
      first == '_' || first == '$') {
    return keyword_lookup(lexeme, length);
  }

  if (first >= '0' && first <= '9') {
    return TOKEN_NUMBER;
  }

  switch (first) {
    case '.':
      if (length == 3) {
        return TOKEN_SPREAD;
      }
      if (length > 1 && lexeme[1] >= '0' && lexeme[1] <= '9') {
        return TOKEN_NUMBER;
      }
      return TOKEN_PERIOD;

    case ',':
      return TOKEN_COMMA;
    case ';':
      return TOKEN_SEMICOLON;
    case ':':
      return TOKEN_COLON;
    case '{':
      return TOKEN_OPENING_KEY;
    case '}':
      return TOKEN_CLOSING_KEY;
    case '[':
      return TOKEN_OPENING_BRA;
    case ']':
      return TOKEN_CLOSING_BRA;
    case '(':
      return TOKEN_OPENING_PAR;
    case ')':
      return TOKEN_CLOSING_PAR;

    case '+':
      if (length == 2 && lexeme[1] == '+') {
        return TOKEN_INCREMENT;
      }
      if (length == 2 && lexeme[1] == '=') {
        return TOKEN_PLUS_ASSIGN;
      }
      return TOKEN_PLUS;
    case '-':
      if (length == 2 && lexeme[1] == '-') {
        return TOKEN_DECREMENT;
      }
      if (length == 2 && lexeme[1] == '=') {
        return TOKEN_MINUS_ASSIGN;
      }
      return TOKEN_MINUS;
    case '*':
      if (length == 3) {
        return TOKEN_POWER_ASSIGN;
      }
      if (length == 2 && lexeme[1] == '*') {
        return TOKEN_POWER;
      }
      if (length == 2 && lexeme[1] == '=') {
        return TOKEN_TIMES_ASSIGN;
      }
      return TOKEN_TIMES;
    case '/':
      if (context == SCANNER_CONTEXT_EXPECT_REGEX && length > 1) {
        return TOKEN_REGEX;
      }
      return length == 2 ? TOKEN_DIV_ASSIGN : TOKEN_DIVIDE;
    case '%':
      return length == 2 ? TOKEN_MOD_ASSIGN : TOKEN_MOD;
    case '=':
      if (length == 3) {
        return TOKEN_STRICT_EQUAL;
      }
      if (length == 2 && lexeme[1] == '>') {
        return TOKEN_ARROW;
      }
      return length == 2 ? TOKEN_EQUAL : TOKEN_ASSIGN;
    case '!':
      if (length == 3) {
        return TOKEN_STRICT_NOT_EQUAL;
      }
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

static void lexer_fill_token(Token *token, TokenType type,
                             const char *lexeme_start, size_t lexeme_length,
                             int line, int column) {
  token->type = type;
  token->lexeme_start = lexeme_start;
  token->lexeme_length = lexeme_length;
  token->line = line;
  token->column = column;
}

void lexer_init(Lexer *lexer, Buffer *buffer) {
  scanner_init(&lexer->scanner, buffer);
  lexer->previous_type = TOKEN_EOF;
  lexer->has_previous = 0;
}

int lexer_next_token(Lexer *lexer, Token *token) {
  lexer_skip_layout_and_comments(lexer);

  const size_t start_pos = scanner_position(&lexer->scanner);
  const int start_line = scanner_line(&lexer->scanner);
  const int start_column = scanner_column(&lexer->scanner);
  const int current = scanner_peek(&lexer->scanner, 0);

  if (current == EOF) {
    lexer_fill_token(token, TOKEN_EOF, NULL, 0, start_line, start_column);
    lexer->previous_type = TOKEN_EOF;
    lexer->has_previous = 1;
    return 1;
  }

  Scanner matched_scanner = lexer->scanner;
  size_t matched_length = 0;
  ScannerContext matched_context = SCANNER_CONTEXT_DEFAULT;

  const int matched = scanner_match_longest_after(
      &lexer->scanner, lexer->has_previous ? lexer->previous_type : TOKEN_EOF,
      &matched_context, &matched_length);

  if (matched) {
    matched_scanner = lexer->scanner;
  }

  if (!matched || matched_length == 0) {
    scanner_next(&lexer->scanner);
    lexer_fill_token(token, TOKEN_ERROR,
                     lexer->scanner.buffer->data + start_pos, 1, start_line,
                     start_column);
    lexer->previous_type = TOKEN_ERROR;
    lexer->has_previous = 1;
    return 1;
  }

  const char *lexeme_start = lexer->scanner.buffer->data + start_pos;
  const TokenType type = lexer_token_type_from_lexeme(
      lexeme_start, matched_length, matched_context);
  size_t lexeme_length = matched_length;

  if ((type == TOKEN_STRING || type == TOKEN_REGEX) && matched_length >= 2) {
    lexeme_start += 1;
    lexeme_length -= 2;
  }

  lexer_fill_token(token, type, lexeme_start, lexeme_length, start_line,
                   start_column);
  lexer->previous_type = type;
  lexer->has_previous = 1;
  lexer->scanner = matched_scanner;
  return 1;
}