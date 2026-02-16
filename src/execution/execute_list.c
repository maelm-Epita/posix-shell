#include "execution_private.h"
#include "utils/vector/vector_extra.h"

int execute_list(struct ast_list *ast_list, struct shell_state *state)
{
    int status = 0;
    for (size_t i = 0; i < ast_list->list->size; i++)
    {
        if (!state->running)
            return state->last_exit_code;
        status = execute_and_or(vector_get_pointer(ast_list->list, i), state);
        state->last_exit_code = status;
        if (state->break_depth > 0 || state->continue_depth > 0)
            break;
    }
    return status;
}
