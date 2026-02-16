#define _POSIX_C_SOURCE 200809L
#include <string.h>

#include "execution_private.h"
#include "functions/functions.h"
#include "utils/hash_map/hash_map.h"

int execute_funcdec(struct ast_funcdec *ast_funcdec, struct shell_state *state)
{
    struct ast_command *func_command = ast_funcdec->command;
    /* The ownership of the ast_command for the function transfers to the
     * hashmap thus we remove the command reference from the ast_funcdec here
     * this prevents it from being freed by free_ast
     */

    /*
     * The command can be null if it is a funcdec inside of a function;
     * The ast will be executed and ast_funcdec->command will be set to null
     * Thus the next time funcdec is executed inside the outer function;
     * It will be null; we want to skip it
     */
    if (ast_funcdec->command == NULL)
        return 0;

    ast_funcdec->command = NULL;
    struct function *func = create_function(func_command);
    char *key = strdup(ast_funcdec->identifier);
    if (hash_map_insert(state->functions, key, func, free_function) == -1)
    {
        ast_funcdec->command = func_command;
        free(key);
        return RET_SHELL_INTERNAL;
    }
    return 0;
}
