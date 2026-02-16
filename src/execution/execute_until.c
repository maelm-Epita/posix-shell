#include "execution_private.h"

int execute_until(struct ast_until *ast_until, struct shell_state *state)
{
    int status = 0;
    state->loop_depth++;

    while (execute_list(ast_until->until_list, state) != 0)
    {
        if (!state->running)
            return state->last_exit_code;
        status = execute_list(ast_until->do_list, state);

        if (state->continue_depth > 0)
        {
            state->continue_depth--;
            if (state->continue_depth == 0)
                continue;
            status = 0;
            break;
        }

        if (state->break_depth > 0)
        {
            state->break_depth--;
            if (state->break_depth == 0)
                break;
            status = 0;
            break;
        }
    }

    state->loop_depth--;
    return status;
}
