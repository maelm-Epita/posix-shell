#include "execution_private.h"
#include "expansion/expansion.h"
#include "utils/vector/vector_extra.h"
#include "variables/variables.h"

int execute_assignments(vector *assignments, struct shell_state *state)
{
    for (size_t i = 0; i < assignments->size; i++)
    {
        m_string assignment = vector_get_pointer(assignments, i);
        vector *assignment_fields = expand(assignment, state, NULL);
        char *assignment_str = ifs_join(assignment_fields, state, 0);
        if (variable_from_string(assignment_str, state->variables) != 0)
            return -1;
        free(assignment_str);
        vector_free_pointer(assignment_fields);
    }
    return 0;
}
