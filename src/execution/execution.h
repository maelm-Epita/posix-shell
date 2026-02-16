#ifndef EXECUTION_H
#define EXECUTION_H

#include "ast/ast.h"
#include "shell/shell.h"
/*

Execution loop pseudocode:

Execute ast
If Need expansion (variable or subshell)
    call expansion, passing envp, update node to result
Update envp

*/

/**
 * @brief Executes the AST using the current shell state.
 * @param ast The AST to execute
 * @param state Shell state
 * @return Exit status of the execution
 */

int execute_ast(struct ast *ast, struct shell_state *state);

#endif /* ! EXECUTION_H */
