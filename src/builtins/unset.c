#include <stdio.h>
#include <string.h>

#include "builtins.h"
#include "functions/functions.h"
#include "variables/variables.h"

int builtin_unset(char **args, struct shell_state *s)
{
    int func_mode = 0;
    int i = 1;

    if (args[1] && args[1][0] == '-')
    {
        if (strcmp(args[1], "-f") == 0)
        {
            func_mode = 1;
            i++;
        }
        else if (strcmp(args[1], "-v") == 0)
        {
            func_mode = 0;
            i++;
        }
    }

    for (; args[i]; i++)
    {
        if (func_mode)
        {
            hash_map_remove(s->functions, args[i], free_function);
        }
        else
        {
            remove_variable(s->variables, args[i]);
        }
    }
    return 0;
}
