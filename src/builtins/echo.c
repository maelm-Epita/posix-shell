#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "builtins.h"

static void print_char(const char *str)
{
    while (*str)
    {
        if (*str == '\\' && *(str + 1))
        {
            str++;
            switch (*str)
            {
            case 'n':
                putchar('\n');
                break;
            case 't':
                putchar('\t');
                break;
            case '\\':
                putchar('\\');
                break;
            default:
                putchar('\\');
                putchar(*str);
                break;
            }
        }
        else
        {
            putchar(*str);
        }
        str++;
    }
}

static bool is_option(char *arg, bool *n_flag, bool *e_flag)
{
    if (!arg || arg[0] != '-' || strlen(arg) < 2)
        return false;

    for (size_t i = 1; i < strlen(arg); i++)
    {
        if (arg[i] != 'n' && arg[i] != 'e' && arg[i] != 'E')
            return false;
    }

    for (size_t i = 1; i < strlen(arg); i++)
    {
        if (arg[i] == 'n')
            *n_flag = true;
        else if (arg[i] == 'e')
            *e_flag = true;
        else if (arg[i] == 'E')
            *e_flag = false;
    }
    return true;
}

int builtin_echo(char **args, struct shell_state *s)
{
    (void)s;
    bool n_flag = false;
    bool e_flag = false;
    int i = 1;

    while (args[i] && is_option(args[i], &n_flag, &e_flag))
    {
        i++;
    }

    while (args[i])
    {
        if (e_flag)
            print_char(args[i]);
        else
            fputs(args[i], stdout);

        if (args[i + 1])
            putchar(' ');

        i++;
    }

    if (!n_flag)
        putchar('\n');

    fflush(stdout);

    return 0;
}
