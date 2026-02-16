#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "utils/hash_map/hash_map.h"
#include "variables/variables.h"
static char *get_env_var(struct shell_state *state, char *key)
{
    struct variable *var = hash_map_get(state->variables, key);
    return var ? var->value : NULL;
}

static int cd_change_dir(char *target, char *oldpwd, char *current_cwd,
                         struct shell_state *s)
{
    if (chdir(target) != 0)
    {
        fprintf(stderr, "cd: %s: %s\n", target, strerror(errno));
        free(target);
        free(oldpwd);
        return 1;
    }

    struct variable *oldpwd_var = create_variable(oldpwd, 1);
    hash_map_insert(s->variables, strdup("OLDPWD"), oldpwd_var, free_variable);
    s->oldpwd_set = 1;

    if (getcwd(current_cwd, PATH_MAX))
    {
        struct variable *pwd_var = create_variable(strdup(current_cwd), 1);
        hash_map_insert(s->variables, strdup("PWD"), pwd_var, free_variable);
    }
    else
    {
        free(target);
        fprintf(stderr, "cd: getcwd failed\n");
    }
    return 0;
}

int builtin_cd(char **args, struct shell_state *s)
{
    char *target = NULL;
    int print_path = 0;

    if (args[1] == NULL)
    {
        target = get_env_var(s, "HOME");
        if (!target)
        {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }
    }
    else if (strcmp(args[1], "-") == 0)
    {
        target = get_env_var(s, "OLDPWD");
        if (!target)
        {
            fprintf(stderr, "cd: OLDPWD not set\n");
            return 1;
        }
        print_path = 1;
    }
    else if (args[2] == NULL)
    {
        target = args[1];
    }
    else
    {
        fprintf(stderr, "cd: too many arguments\n");
        return 1;
    }
    target = strdup(target);

    char current_cwd[PATH_MAX];
    char *oldpwd = get_env_var(s, "PWD");

    if (!oldpwd)
        oldpwd = getcwd(current_cwd, PATH_MAX) != NULL ? strdup(current_cwd)
                                                       : strdup("");
    else
        oldpwd = strdup(oldpwd);

    if (print_path)
        printf("%s\n", target);

    if (cd_change_dir(target, oldpwd, current_cwd, s) == 1)
        return 1;

    free(target);

    return 0;
}
