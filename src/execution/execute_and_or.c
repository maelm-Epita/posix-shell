#include <unistd.h>

#include "execution_private.h"
#include "utils/vector/vector_extra.h"

int execute_and_or(struct ast_and_or *ast_and_or, struct shell_state *state)
{
    // TODO
    if (ast_and_or->background)
    {
        pid_t pid = fork();
        if (pid == -1)
        {
            return RET_SHELL_INTERNAL;
        }
        else if (pid == 0)
        {
        }
        else
        {
            return 0;
        }
    }

    int status =
        execute_pipeline(vector_get_pointer(ast_and_or->pipelines, 0), state);
    enum and_or_type *type = vector_try_get(ast_and_or->connection_types, 0);

    for (size_t i = 1; i < ast_and_or->pipelines->size; i++)
    {
        if (!state->running)
            break;
        if ((*type == AND_OR_AND && status == 0)
            || (*type == AND_OR_OR && status != 0))
        {
            status = execute_pipeline(
                vector_get_pointer(ast_and_or->pipelines, i), state);
        }
        type = vector_try_get(ast_and_or->connection_types, i);
        // Means we were on the last and_or
        if (type == NULL)
            break;
    }

    return status;
}
