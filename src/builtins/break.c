#include <stdio.h>

#include "builtins.h"
#include "utils/string/string_helpers.h"

int builtin_break(char **args, struct shell_state *s)
{
    int n = 1;

    if (args[1] != NULL)
    {
        if (args[2] != NULL)
        {
            fprintf(stderr, "break: too many arguments\n");
            return 1;
        }
        n = string_to_int(args[1]);
        if (n < 1)
        {
            fprintf(stderr, "break: %s: numeric argument required\n", args[1]);
            return 1;
        }
    }

    if (s->loop_depth <= 0)
    {
        fprintf(stderr, "break: not in a loop\n");
        return 1;
    }
    if (n > s->loop_depth)
        n = s->loop_depth;

    s->continue_depth = 0;
    s->break_depth = n;
    return 0;
}
