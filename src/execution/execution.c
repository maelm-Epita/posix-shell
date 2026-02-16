#include "execution.h"

#include "execution_private.h"

/*
When to fork:
- on each command in a pipeline if there are commands after
- when executing as background
- right before using execvp unless i've already forked and there is literally
nothing after the command

When not to fork:
- when using a builtin
- when it is the last simple command and i've already forked

Consequences for redirection:
- need to restore file descriptors ONLY if using a builtin
*/

int execute_ast(struct ast *ast, struct shell_state *state)
{
    if (ast == NULL)
        return 0;

    return execute_list(ast->list, state);
}
