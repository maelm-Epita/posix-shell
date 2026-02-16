#include <stdlib.h>

#include "builtins.h"
#include "utils/string/string_helpers.h"

int builtin_exit(char **args, struct shell_state *s)
{
    if (args[1] == NULL)
    {
        s->running = 0;
        return s->last_exit_code;
    }
    if (args[2] != NULL)
    {
        fprintf(stderr, "exit: too many arguments\n");
        return 2;
    }
    int r;
    if (string_to_int_plus_minus(args[1], &r) == -1)
    {
        fprintf(stderr, "exit: numeric argument required\n");
        return 2;
    }
    s->running = 0;
    s->last_exit_code = r;
    return (unsigned char)r;
}
