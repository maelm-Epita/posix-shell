#include <stdlib.h>

#include "execution_private.h"
#include "expansion/expansion.h"
#include "matching/matching.h"
#include "utils/vector/vector_extra.h"

static char *expand_to_string(m_string ms, struct shell_state *state)
{
    vector *fields = expand(ms, state, NULL);
    if (fields == NULL)
        return NULL;

    char *res = ifs_join(fields, state, 0);
    vector_free_pointer(fields);
    return res;
}

int execute_case(struct ast_case *ast_case, struct shell_state *state)
{
    if (ast_case == NULL || state == NULL)
        return RET_SHELL_INTERNAL;

    if (!state->running)
        return state->last_exit_code;

    char *word = expand_to_string(ast_case->word, state);
    if (word == NULL)
        return RET_SHELL_INTERNAL;

    for (size_t i = 0; i < ast_case->branches->size; i++)
    {
        if (!state->running)
        {
            free(word);
            return state->last_exit_code;
        }
        struct ast_case_branch *branch =
            vector_get_pointer(ast_case->branches, i);
        for (size_t j = 0; j < branch->patterns->size; j++)
        {
            m_string pat_ms = vector_get_pointer(branch->patterns, j);
            char *pattern = expand_to_string(pat_ms, state);
            if (pattern == NULL)
            {
                free(word);
                return RET_SHELL_INTERNAL;
            }

            if (pattern_matches(word, pattern))
            {
                free(pattern);

                int status = 0;
                if (branch->list != NULL)
                    status = execute_list(branch->list, state);

                free(word);
                return status;
            }
            free(pattern);
        }
    }
    free(word);
    return 0;
}
