/**
 * @file token.h
 * @brief Token type definitions and the Token structure used throughout the
 *        lexer pipeline.
 *
 * A token is the atomic unit produced by the lexer. Each token carries its
 * type, a pointer into the raw input buffer where the lexeme begins, the
 * lexeme length in bytes, and the source location (line and column) at which
 * the lexeme starts. Ownership of the buffer memory belongs to the Buffer
 * object; tokens are lightweight views over that memory.
 *
 * @note Every TokenType value must have a corresponding string representation
 *       registered in token.c's TOKEN_TYPE_NAMES table.
 */
#ifndef ESJS_CUSTOM_LEXER_TOKEN_H
#define ESJS_CUSTOM_LEXER_TOKEN_H

#include <stdio.h>

/**
 * @brief Exhaustive list of token categories recognized by the EsJS lexer.
 *
 * The enum is divided into logical groups:
 *  - Generic categories: identifiers, keywords, literals.
 *  - Language-level keyword tokens mapped to specific EsJS reserved words.
 *  - Literal value tokens: numbers, strings, regular expressions.
 *  - Control-flow sentinels: EOF and ERROR.
 *  - Arithmetic and assignment operators.
 *  - Relational and equality operators.
 *  - Logical and bitwise operators.
 *  - Punctuation and grouping tokens.
 *  - Compound operators: increment, decrement, arrow, ternary, nullish.
 *
 * @warning Each value requires its own representation entry in token.c.
 *          Adding a new token type without updating TOKEN_TYPE_NAMES will
 *          cause token_type_to_string() to return an empty string for it.
 */
typedef enum {
  TOKEN_IDENTIFIER, /**< User-defined name; not a reserved word. */
  TOKEN_KEYWORD,    /**< Reserved EsJS word matched by keyword_lookup(). */

  /* ---- Language keyword tokens ----------------------------------------- */
  TOKEN_CONST,     /**< `const`    - immutable binding declaration.         */
  TOKEN_LET,       /**< (reserved) - mutable binding (maps to `mut`).       */
  TOKEN_VAR,       /**< `var`      - function-scoped variable declaration.  */
  TOKEN_FUNCTION,  /**< `funcion`  - function declaration keyword.          */
  TOKEN_RETURN,    /**< `retornar` - return statement keyword.              */
  TOKEN_IF,        /**< `si`       - conditional branch keyword.            */
  TOKEN_ELSE,      /**< `sino`     - alternative branch keyword.            */
  TOKEN_FOR,       /**< `para`     - for-loop keyword.                      */
  TOKEN_WHILE,     /**< `mientras` - while-loop keyword.                    */
  TOKEN_DO,        /**< `hacer`    - do-while keyword.                      */
  TOKEN_SWITCH,    /**< `elegir`   - switch statement keyword.              */
  TOKEN_CASE,      /**< `caso`     - switch case keyword.                   */
  TOKEN_DEFAULT,   /**< `porDefecto` - default branch keyword.              */
  TOKEN_BREAK,     /**< `romper`   - break statement keyword.               */
  TOKEN_CONTINUE,  /**< `continuar` - continue statement keyword.           */
  TOKEN_TRY,       /**< `intentar` - try block keyword.                     */
  TOKEN_CATCH,     /**< `capturar` - catch clause keyword.                  */
  TOKEN_FINALLY,   /**< `finalmente` - finally clause keyword.              */
  TOKEN_THROW,     /**< `lanzar`   - throw statement keyword.               */
  TOKEN_CLASS,     /**< `clase`    - class declaration keyword.             */
  TOKEN_EXTENDS,   /**< `extiende` - class inheritance keyword.             */
  TOKEN_NEW,       /**< `crear`    - object instantiation keyword.          */
  TOKEN_THIS,      /**< (reserved) - current instance reference.            */
  TOKEN_SUPER,     /**< `super`    - parent class reference.                */
  TOKEN_IMPORT,    /**< `importar` - module import keyword.                 */
  TOKEN_EXPORT,    /**< `exportar` - module export keyword.                 */
  TOKEN_FROM,      /**< `desde`    - import source specifier.               */
  TOKEN_AS,        /**< (reserved) - alias specifier in imports/exports.    */
  TOKEN_ASYNC,     /**< `asincrono` - async function keyword.               */
  TOKEN_AWAIT,     /**< `esperar`  - await expression keyword.              */
  TOKEN_YIELD,     /**< `producir` - generator yield keyword.               */
  TOKEN_IN,        /**< `en`       - for-in loop keyword.                   */
  TOKEN_OF,        /**< `de`       - for-of loop keyword.                   */
  TOKEN_INSTANCEOF,/**< `instanciaDe` - instanceof operator keyword.        */
  TOKEN_TYPEOF,    /**< `tipoDe`   - typeof operator keyword.               */
  TOKEN_VOID,      /**< `vacio`    - void operator keyword.                 */
  TOKEN_DELETE,    /**< `eliminar` - delete operator keyword.               */
  TOKEN_DEBUGGER,  /**< `depurador` - debugger statement keyword.           */
  TOKEN_WITH,      /**< `con`      - with statement keyword.                */
  TOKEN_STATIC,    /**< (reserved) - static class member modifier.          */
  TOKEN_GET,       /**< (reserved) - getter accessor keyword.               */
  TOKEN_SET,       /**< (reserved) - setter accessor keyword.               */
  TOKEN_TRUE,      /**< `verdadero` - boolean true literal keyword.         */
  TOKEN_FALSE,     /**< `falso`    - boolean false literal keyword.         */
  TOKEN_NULL,      /**< `nulo`     - null literal keyword.                  */
  TOKEN_UNDEFINED, /**< `indefinido` - undefined literal keyword.           */
  TOKEN_ENUM,      /**< (reserved) - future reserved word.                  */
  TOKEN_IMPLEMENTS,/**< (reserved) - future reserved word.                  */
  TOKEN_INTERFACE, /**< (reserved) - future reserved word.                  */
  TOKEN_PACKAGE,   /**< (reserved) - future reserved word.                  */
  TOKEN_PRIVATE,   /**< (reserved) - future reserved word.                  */
  TOKEN_PROTECTED, /**< (reserved) - future reserved word.                  */
  TOKEN_PUBLIC,    /**< (reserved) - future reserved word.                  */

  /* ---- Literal value tokens --------------------------------------------- */
  TOKEN_NUMBER, /**< Integer or floating-point numeric literal.             */
  TOKEN_STRING, /**< String literal (double-quoted, single-quoted, or
                     backtick template). Delimiters are stripped from the
                     lexeme stored in the Token.                             */
  TOKEN_REGEX,  /**< Regular expression literal `/pattern/flags`. The
                     enclosing slashes are stripped from the stored lexeme.  */

  /* ---- Sentinels -------------------------------------------------------- */
  TOKEN_EOF,   /**< End-of-file sentinel; no more tokens will be produced. */
  TOKEN_ERROR, /**< Invalid or unrecognized input; signals a lexical error. */

  /* ---- Arithmetic operators --------------------------------------------- */
  TOKEN_PLUS,    /**< `+`  - addition or unary plus.                        */
  TOKEN_MINUS,   /**< `-`  - subtraction or unary minus.                    */
  TOKEN_TIMES,   /**< `*`  - multiplication.                                */
  TOKEN_DIVIDE,  /**< `/`  - division.                                      */
  TOKEN_POWER,   /**< `**` - exponentiation.                                */
  TOKEN_MOD,     /**< `%`  - modulo (remainder).                            */

  /* ---- Assignment operators --------------------------------------------- */
  TOKEN_ASSIGN,        /**< `=`   - simple assignment.                      */
  TOKEN_MOD_ASSIGN,    /**< `%=`  - modulo-assignment.                      */
  TOKEN_DIV_ASSIGN,    /**< `/=`  - division-assignment.                    */
  TOKEN_TIMES_ASSIGN,  /**< `*=`  - multiplication-assignment.              */
  TOKEN_MINUS_ASSIGN,  /**< `-=`  - subtraction-assignment.                 */
  TOKEN_PLUS_ASSIGN,   /**< `+=`  - addition-assignment.                    */
  TOKEN_POWER_ASSIGN,  /**< `**=` - exponentiation-assignment.              */

  /* ---- Equality operators ----------------------------------------------- */
  TOKEN_EQUAL,             /**< `==`  - abstract equality.                  */
  TOKEN_STRICT_EQUAL,      /**< `===` - strict equality.                    */
  TOKEN_NOT_EQUAL,         /**< `!=`  - abstract inequality.                */
  TOKEN_STRICT_NOT_EQUAL,  /**< `!==` - strict inequality.                  */

  /* ---- Relational operators --------------------------------------------- */
  TOKEN_LESS,           /**< `<`  - less-than.                              */
  TOKEN_LESS_EQUAL,     /**< `<=` - less-than-or-equal.                     */
  TOKEN_GREATER,        /**< `>`  - greater-than.                           */
  TOKEN_GREATER_EQUAL,  /**< `>=` - greater-than-or-equal.                  */

  /* ---- Logical operators ------------------------------------------------ */
  TOKEN_LOGICAL_AND,  /**< `&&` - logical AND; standalone `&` is an error. */
  TOKEN_LOGICAL_OR,   /**< `||` - logical OR;  standalone `|` is an error. */

  /* ---- Bitwise and unary operators -------------------------------------- */
  TOKEN_NOT,      /**< `!`  - logical NOT.                                  */
  TOKEN_BIT_AND,  /**< `&`  - bitwise AND (currently produces TOKEN_ERROR). */
  TOKEN_BIT_OR,   /**< `|`  - bitwise OR  (currently produces TOKEN_ERROR). */
  TOKEN_BIT_XOR,  /**< `^`  - bitwise XOR.                                  */
  TOKEN_BIT_NOT,  /**< `~`  - bitwise NOT (one's complement).               */

  /* ---- Punctuation ------------------------------------------------------ */
  TOKEN_SPREAD,     /**< `...` - spread / rest operator.                    */
  TOKEN_PERIOD,     /**< `.`   - member access operator.                    */
  TOKEN_COMMA,      /**< `,`   - argument / element separator.              */
  TOKEN_SEMICOLON,  /**< `;`   - statement terminator.                      */
  TOKEN_COLON,      /**< `:`   - object property separator / label.         */

  /* ---- Grouping tokens -------------------------------------------------- */
  TOKEN_OPENING_KEY,  /**< `{` - open brace.                                */
  TOKEN_CLOSING_KEY,  /**< `}` - close brace.                               */
  TOKEN_OPENING_BRA,  /**< `[` - open bracket.                              */
  TOKEN_CLOSING_BRA,  /**< `]` - close bracket.                             */
  TOKEN_OPENING_PAR,  /**< `(` - open parenthesis.                          */
  TOKEN_CLOSING_PAR,  /**< `)` - close parenthesis.                         */

  /* ---- Compound / special operators ------------------------------------- */
  TOKEN_INCREMENT,  /**< `++` - pre/post increment.                         */
  TOKEN_DECREMENT,  /**< `--` - pre/post decrement.                         */

  TOKEN_ARROW,    /**< `=>`  - arrow function fat-arrow.                    */
  TOKEN_TERNARY,  /**< `?`   - ternary conditional operator.                */
  TOKEN_NULISH,   /**< `??`  - nullish coalescing operator.                 */

  /**
   * @internal Sentinel used to track the total number of enum values.
   *           Do not assign semantic meaning to this entry.
   */
  _TOKEN_TYPE_ENUM_COUNT
} TokenType;

/**
 * @brief Lightweight, non-owning view of a single lexeme in the input buffer.
 *
 * The lexer does not copy lexeme text; instead it stores a pointer directly
 * into the Buffer's internal data array together with the length of the match.
 * The Buffer must remain live and unmodified for as long as any Token
 * referencing it is in use.
 *
 * For TOKEN_STRING and TOKEN_REGEX the delimiters (quotes or slashes) are
 * excluded: @c lexeme_start points to the first content character and
 * @c lexeme_length counts only the content bytes.
 *
 * @warning **`lexeme_start` is invalidated by buffer reallocation.** The
 *          Buffer grows its internal heap array on demand. If more tokens are
 *          read after a Token is stored, a subsequent realloc() may move the
 *          array to a new address, leaving `lexeme_start` as a dangling
 *          pointer. Copy the lexeme bytes (e.g. with strndup or memcpy) before
 *          calling lexer_next_token() again if you need to retain the text.
 */
typedef struct {
  TokenType   type;          /**< Semantic category of this token.          */
  const char *lexeme_start;  /**< Pointer into Buffer::data at match start. */
  size_t      lexeme_length; /**< Length of the lexeme in bytes.            */
  int         line;          /**< 1-based source line of the first character. */
  int         column;        /**< 1-based source column of the first character
                                  (UTF-8 code-point count, not byte offset). */
} Token;

/**
 * @brief Maps a TokenType to its canonical string name used in output.
 *
 * Returns an empty string for TOKEN_EOF, TOKEN_ERROR, and any value that
 * falls outside the valid range or has no registered name.
 *
 * @param type The token type to convert.
 * @return Pointer to a static, null-terminated string. Never NULL.
 */
const char *token_type_to_string(TokenType type);

#endif  // ESJS_CUSTOM_LEXER_TOKEN_H