#ifndef PARSER_HELPERS_H
#define PARSER_HELPERS_H

#include "token/token.h"

/**
 * @brief Peeks the next token and returns P_ERR on lexer failure.
 */

#define PEEK(tok)                                                              \
    {                                                                          \
        int s = lexer_peek(tok);                                               \
        if (s != LEXER_SUCCESS)                                                \
            return s == LEXER_SYNTAX_ERR ? P_SYNTAX_ERR : P_ERR;               \
    }

/**
 * @brief Eats one token of a given type, returns P_SYNTAX_ERR on mismatch.
 */

#define EAT(rule, type)                                                        \
    {                                                                          \
        if (lexer_eat(rule, type) != 0)                                        \
            return P_SYNTAX_ERR;                                               \
    }

/**
 * @brief Eats one token among N types, returns P_SYNTAX_ERR on mismatch.
 */

#define EAT_ONE_OF_N(rule, n, ...)                                             \
    {                                                                          \
        if (lexer_eat_one_of_n(rule, n, __VA_ARGS__) != 0)                     \
            return P_SYNTAX_ERR;                                               \
    }

/**
 * @brief Looks ahead count tokens, returns P_ERR on lexer failure.
 */

#define LOOKAHEAD(tok, count)                                                  \
    {                                                                          \
        int s = lexer_lookahead(tok, count);                                   \
        if (s != LEXER_SUCCESS)                                                \
            return s == LEXER_SYNTAX_ERR ? P_SYNTAX_ERR : P_ERR;               \
    }

/**
 * @brief Consumes all consecutive newline tokens.
 */

#define EAT_NEWLINES()                                                         \
    {                                                                          \
        PEEK(&peeked);                                                         \
        while (peeked->type == T_NL)                                           \
        {                                                                      \
            EAT("case item", T_NL);                                            \
            PEEK(&peeked);                                                     \
        }                                                                      \
    }

/**
 * @brief Checks if the token represents an assignment
 * @param tok The token to check
 * @return Returns 0 if token does not represent a valid assignment, non 0 if it
 * does
 */
int is_assignment(struct token *tok);

/*-------------------------------------------------------------------------------------------*/

/**
 * @brief Checks if a token is in the FIRST set of shell_command.
 * @param tok Token to test
 * @return 1 if in FIRST set, 0 otherwise
 */

/* shell command's first set is: '{' '(' 'for' 'while' 'until' 'case' 'if' */
int is_in_shell_command_first(struct token *tok);

/*-------------------------------------------------------------------------------------------*/

/**
 * @brief Checks if a token is in the FIRST set of redirection.
 * @param tok Token to test
 * @return 1 if in FIRST set, 0 otherwise
 */

/* redirection's first set is IONUMBER '>' '<' '>>' '>&' '<&' '>|' '<>' '<<'
 * '<<-' */
int is_in_redirection_first(struct token *tok);

/*-------------------------------------------------------------------------------------------*/

/**
 * @brief Checks if a token is in the FIRST set of prefix.
 * @param tok Token to test
 * @return 1 if in FIRST set, 0 otherwise
 */

/* prefix's first set is ASSIGNMENT_WORD IONUMBER '>' '<' '>>' '>&' '<&' '>|'
 * '<>' '<<' '<<-' */
int is_in_prefix_first(struct token *tok);

/*-------------------------------------------------------------------------------------------*/

/**
 * @brief Checks if a token is in the FIRST set of element.
 * @param tok Token to test
 * @return 1 if in FIRST set, 0 otherwise
 */

/* element's first set is WORD IONUMBER '>' '<' '>>' '>&' '<&' '>|' '<>' '<<'
 * '<<-' */
int is_in_element_first(struct token *tok);

/*-------------------------------------------------------------------------------------------*/

/**
 * @brief Checks if a token is in the FIRST set of and_or.
 * @param tok Token to test
 * @return 1 if in FIRST set, 0 otherwise
 */

/* and or's first set is '!' ASSIGNMENT_WORD IONUMBER '>' '<' '>>' '>&' '<&'
 * '>|' '<>' '<<' '<<-' WORD '{' '(' 'for' 'while' 'until' 'case' 'if' */
int is_in_and_or_first(struct token *tok);

/*-------------------------------------------------------------------------------------------*/

/**
 * @brief Checks if a token is in the FIRST set of case_item/case_clause.
 * @param tok Token to test
 * @return 1 if in FIRST set, 0 otherwise
 */

/* case item first set is '(' WORD */
/* case clause first set is '(' WORD */
int is_in_case_item_clause_first(struct token *tok);

/*-------------------------------------------------------------------------------------------*/

/**
 * @brief Checks if a token is in the FIRST set of else_clause.
 * @param tok Token to test
 * @return 1 if in FIRST set, 0 otherwise
 */

/* else clause first set is 'else' 'elif' */
int is_in_else_clause_first(struct token *tok);

/*-------------------------------------------------------------------------------------------*/

/**
 * @brief Checks if a token is in the FIRST set of compound_list.
 * @param tok Token to test
 * @return 1 if in FIRST set, 0 otherwise
 */

/* and or's first set is '\n' '!' ASSIGNMENT_WORD IONUMBER '>' '<' '>>' '>&'
 * '<&' '>|' '<>' '<<' '<<-' WORD '{' '(' 'for' 'while' 'until' 'case' 'if' */
int is_in_compound_list_first(struct token *tok);

/*-------------------------------------------------------------------------------------------*/

/**
 * @brief Detects if a function declaration can be predicted from two tokens.
 * @param first First token
 * @param second Second token
 * @return 1 if it looks like a funcdec, 0 otherwise
 */

/* function declaration rule can be predicted if first token is WORD and second
 * token is '(' */
int is_funcdec(struct token *first, struct token *second);

/*-------------------------------------------------------------------------------------------*/

#endif /* PARSER_HELPERS_H */
