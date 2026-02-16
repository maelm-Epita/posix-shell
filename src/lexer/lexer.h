#ifndef LEXER_H
#define LEXER_H

#include "token/token.h"

struct lex_word_state
{
    int escaping;
    int escaped_last;
    int in_paren;
    int in_sub_brack;
    enum quote_type type;
    vector *segments;
    vector *acc;
};

/**
 ** @brief                      Different possible states of lexing status
 ** @param LEXER_SUCCESS        Lexing succeeded
 ** @param LEXER_SYNTAX_ERR     A syntax error was encountered in the input,
 * this stops the shell loop if source is not stdin
 ** @param LEXER_INTERNAL_ERR   An error was encountered during parsing,
 * probably due to allocation
 */
enum lexer_state
{
    LEXER_SUCCESS, // lexer succeeded getting the next token
    LEXER_SYNTAX_ERR,
    LEXER_INTERNAL_ERR,
};

int is_separator(char c);

/**
 ** @brief              Looks count tokens ahead of the peeked token
 ** @param out          The pointer to update with the value of the peeked token
 ** @param count        The number of tokens to look forward
 ** @return             The status of the lexer
 */
enum lexer_state lexer_lookahead(struct token **out, size_t count);

/**
 ** @brief              Peeks the head of the unconsumed token queue (generates
 * it if the queue is empty)
 ** @param out          The pointer to update with the value of the peeked token
 ** @return             The status of the lexer
 */
enum lexer_state lexer_peek(struct token **out);

/**
 ** @brief              Consumes tokens until it finds a newline token
 ** @param void
 ** @return             The status of the lexer
 */
enum lexer_state lexer_clear_line(void);

/**
 ** @brief              Eats (consumes) the head of the token queue if its type
 * matches tok_type
 ** @param rule         The current rule from which this function is called
 ** @param tok_type     The expected type of the token to consume
 ** @return             Returns 0 if it consumed the head successfully, -1 if it
 * did not (meaning the head token was not of type tok_type)
 ** @effects            Prints an error message to stderr if the head token was
 * not of type tok_type
 */
int lexer_eat(char *rule, enum token_type tok_type);

/**
 ** @brief              Eats (consumes) the head of the token queue if its type
 * matches any type in __va_args__
 ** @param rule         The current rule from which this function is called
 ** @param n            The number of token_types to match against
 ** @param __va_args__  The token types to match against
 ** @return             Returns 0 if it consumed the head successfully, -1 if it
 * did not
 ** @effects            Prints an error message to stderr if the head token did
 * not match any type
 ** @safety             This should only be called with token_type arguments as
 * variadic arguments, it also crashes via assert if n > 64
 */
int lexer_eat_one_of_n(char *rule, size_t n, ...);

void lexer_reset(void);

int lexer_queue_push_front(struct token *tok);

enum lexer_state lex_next_token(struct token **out);

#endif /* ! LEXER_H */
