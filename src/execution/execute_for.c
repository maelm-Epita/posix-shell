#define _POSIX_C_SOURCE 200809L

#include <string.h>

#include "execution_private.h"
#include "expansion/expansion.h"
#include "utils/vector/vector_extra.h"
#include "variables/variables.h"

int execute_for(struct ast_for *ast_for, struct shell_state *state)
{
    int status = 0;
    char *key = m_string_get_raw(ast_for->element_identifier);
    if (key == NULL)
        return RET_SHELL_INTERNAL;
    vector *expanded_words = expand_vector(ast_for->words, state, NULL);
    if (expanded_words == NULL)
        return RET_SHELL_INTERNAL;

    state->loop_depth++;

    for (size_t i = 0; i < expanded_words->size; i++)
    {
        if (!state->running)
        {
            vector_free_pointer(expanded_words);
            return state->last_exit_code;
        }
        char *word = vector_get_pointer(expanded_words, i);
        word = strdup(word);
        struct variable *var = create_variable(word, 0);
        char *key_cpy = strdup(key);
        if (key_cpy == NULL)
            return RET_SHELL_INTERNAL;
        if (hash_map_insert(state->variables, key_cpy, var, free_variable)
            == -1)
            return RET_SHELL_INTERNAL;

        status = execute_list(ast_for->do_list, state);

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

    vector_free_pointer(expanded_words);
    return status;
}
