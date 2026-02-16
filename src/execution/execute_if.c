#include "execution_private.h"
#include "utils/vector/vector_extra.h"

int execute_if(struct ast_if *ast_if, struct shell_state *state)
{
    int status = 0;
    for (size_t i = 0; i < ast_if->then_lists->size; i++)
    {
        if (!state->running)
            return state->last_exit_code;
        struct ast_list *condition =
            vector_try_get_pointer(ast_if->if_lists, i);
        if (condition == NULL || execute_list(condition, state) == 0)
        {
            if (!state->running)
                return state->last_exit_code;
            return execute_list(vector_get_pointer(ast_if->then_lists, i),
                                state);
        }
    }
    return status;
}
