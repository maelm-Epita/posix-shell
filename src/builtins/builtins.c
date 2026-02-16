#include "builtins.h"

#include <stddef.h>
#include <string.h>

#define CTRL_BREAK_BASE 1000
#define CTRL_CONTINUE_BASE 2000

builtin_function get_builtin(char *name)
{
    if (name == NULL)
        return NULL;

    if (strcmp(name, "echo") == 0)
        return builtin_echo;
    else if (strcmp(name, "true") == 0)
        return builtin_true;
    else if (strcmp(name, "false") == 0)
        return builtin_false;
    else if (strcmp(name, "exit") == 0)
        return builtin_exit;
    else if (strcmp(name, "cd") == 0)
        return builtin_cd;
    else if (strcmp(name, "export") == 0)
        return builtin_export;
    else if (strcmp(name, "unset") == 0)
        return builtin_unset;
    else if (strcmp(name, ".") == 0)
        return builtin_dot;
    else if (strcmp(name, "break") == 0)
        return builtin_break;
    else if (strcmp(name, "continue") == 0)
        return builtin_continue;
    else if (strcmp(name, "alias") == 0)
        return builtin_alias;
    else if (strcmp(name, "unalias") == 0)
        return builtin_unalias;
    return NULL;
}

int builtin_true(char **args, struct shell_state *s)
{
    (void)args;
    (void)s;
    return 0;
}

int builtin_false(char **args, struct shell_state *s)
{
    (void)args;
    (void)s;
    return 1;
}
