#ifndef PARSER_H
#define PARSER_H

#include "ast/ast.h"
#include "shell/shell.h"

/**
 * @brief Parser return statuses.
 */

enum parser_status
{
    P_SUCCESS,
    P_SYNTAX_ERR,
    P_EOF,
    P_ERR
};

/**
 * @brief Parses the current input into an AST.
 * @param ast Output pointer updated with the parsed AST on success
 * @return Parser status (P_SUCCESS on success)
 */

enum parser_status parse_input(struct ast **ast, struct shell_state *state);

#endif /* ! PARSER_H */
