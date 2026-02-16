#include "functions.h"

#include <stdlib.h>

#include "ast/ast.h"

void free_function(void *f)
{
    struct function *function = f;
    if (function == NULL)
        return;
    if (function->command != NULL)
        free_command(function->command);
    free(function);
}

struct function *create_function(struct ast_command *ast_command)
{
    struct function *function = malloc(sizeof(struct function));
    if (function == NULL)
        return NULL;
    function->command = ast_command;
    return function;
}
