#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

#include "string_metadata.h"
#include "utils/vector/vector.h"

/**
 * @brief Token types produced by the lexer and used by the parser.
 */

enum token_type
{
    /* Symbols */
    T_EOF,
    T_NL,

    /* Words (arguably ionumber should just be a word maybe */
    T_WORD, /* (string | command) */
    T_IONUMBER,

    /* Operators */
    T_LEFT_PAREN,
    T_RIGHT_PAREN,
    T_PIPE,
    T_DOUBLE_PIPE,
    T_SEMICOL,
    T_DOUBLE_SEMICOL,
    T_AND,
    T_DOUBLE_AND,

    /* Redirection operators */
    T_GT,
    T_LT,
    T_DOUBLE_GT,
    T_DOUBLE_LT,
    T_GT_AND,
    T_LT_AND,
    T_GT_PIPE,
    T_LT_GT,
    T_DOUBLE_LT_MINUS,

    /* Reserved keywords; Should never be lexed, only used in parsing stage */
    T_EXCLAMATION,
    T_LEFT_BRACK,
    T_RIGHT_BRACK,
    T_FOR,
    T_DO,
    T_DONE,
    T_WHILE,
    T_UNTIL,
    T_CASE,
    T_IN,
    T_ESAC,
    T_IF,
    T_THEN,
    T_FI,
    T_ELSE,
    T_ELIF
};

/**
 * @brief Token value (either an int or a string).
 */

union token_value
{
    int int_value;
    m_string string;
};

/**
 * @brief A single token (type + optional value).
 *
 * @param type Token type
 * @param value Token value (only valid for some types)
 */

struct token
{
    enum token_type type;
    union token_value value; /* some types should not have values */
};

/**
 * @brief Creates a token with an explicit value.
 * @param type Token type
 * @param val Token value
 * @return Newly allocated token, or NULL on failure
 */

struct token *create_token(enum token_type type, union token_value val);

/**
 * @brief Creates a token holding a string value.
 * @param type Token type
 * @param val Source buffer
 * @param n Number of characters to copy
 * @return Newly allocated token, or NULL on failure
 */

struct token *create_token_string(enum token_type type, char *val, size_t n);

struct token *token_copy(struct token *token);

/**
 * @brief Frees a token.
 * @param token Token to free
 */

void free_token(struct token *token);

/**
 * @brief Returns a printable name for a token type.
 * @param type Token type
 * @return String name of the type
 */

char *token_type_to_string(enum token_type type);

/**
 * @brief Checks if a token matches a given type.
 * @param tok Token to test
 * @param t Expected type
 * @return 1 if it matches, 0 otherwise
 */

int token_matches_type(struct token *tok, enum token_type t);

/**
 * @brief Tells whether a token type carries an int value.
 * @param t Token type
 * @return 1 if it has an int value, 0 otherwise
 */

int token_type_has_int_value(enum token_type t);

/**
 * @brief Tells whether a token type carries a string value.
 * @param t Token type
 * @return 1 if it has a string value, 0 otherwise
 */

int token_type_has_string_value(enum token_type t);

/**
 * @brief Checks if a token is a reserved keyword (when type is WORD).
 * @param tok Token to test
 * @return 1 if reserved keyword, 0 otherwise
 */

/* type is WORD and value is one of (for | while | until | case | if | in | do |
 * done | esac | fi | then | else | elif) */
int is_reserved_keyword(struct token *tok);
int is_reserved_keyword_str(char *str);

#endif /* TOKEN_H */
