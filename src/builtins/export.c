#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "utils/hash_map/hash_map.h"
#include "variables/variables.h"

static int is_valid_name(char *name)
{
    if (!name || !*name || isdigit(name[0]))
        return 0;

    for (size_t i = 0; name[i]; i++)
    {
        if (!isalnum(name[i]) && name[i] != '_')
            return 0;
    }
    return 1;
}

static int do_export(struct shell_state *state, char *key, char *val)
{
    struct variable *var = hash_map_get(state->variables, key);
    if (var)
    {
        if (val)
        {
            free(var->value);
            var->value = strdup(val);
            if (!var->value)
                return -1;
        }
        var->exported = 1;
        return 0;
    }

    char *value_to_store = val ? strdup(val) : strdup("");
    if (!value_to_store)
        return -1;

    var = create_variable(value_to_store, 1);
    if (!var)
    {
        free(value_to_store);
        return -1;
    }

    char *k = strdup(key);
    if (!k)
    {
        free_variable(var);
        return -1;
    }

    if (hash_map_insert(state->variables, k, var, free_variable) == -1)
    {
        free(k);
        free_variable(var);
        return -1;
    }
    return 0;
}

int builtin_export(char **args, struct shell_state *s)
{
    int status = 0;

    if (!args[1])
        return 0;

    for (size_t i = 1; args[i]; i++)
    {
        char *arg = args[i];
        char *eq = strchr(arg, '=');

        if (eq)
        {
            *eq = '\0';
            char *name = arg;
            char *val = eq + 1;

            if (is_valid_name(name))
            {
                if (do_export(s, name, val) == -1)
                    status = 1;
            }
            else
            {
                fprintf(stderr, "export: `%s=%s': not a valid identifier\n",
                        name, val);
                status = 1;
            }
            *eq = '=';
        }
        else
        {
            if (is_valid_name(arg))
            {
                if (do_export(s, arg, NULL) == -1)
                    status = 1;
            }
            else
            {
                fprintf(stderr, "export: `%s': not a valid identifier\n", arg);
                status = 1;
            }
        }
    }
    return status;
}
