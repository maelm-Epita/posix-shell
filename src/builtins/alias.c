#include <string.h>

#include "builtins.h"
#include "token/aliases.h"
#include "utils/hash_map/hash_map.h"

static int get_alias_key_val(char *arg, char **key, char **val)
{
    size_t key_len = 0;
    while (arg[key_len] != 0)
    {
        if (arg[key_len] == '=')
            break;
        key_len++;
    }
    *key = calloc(key_len + 1, sizeof(char));
    if (*key == NULL)
        return -1;
    memcpy(*key, arg, key_len);
    if (arg[key_len] == 0)
        *val = NULL;
    else
        *val = arg + key_len + 1;
    return 0;
}

static int print_single_alias(char *name, struct shell_state *s)
{
    struct alias *a = hash_map_get(s->aliases, name);
    if (a != NULL)
    {
        printf("%s='%s'\n", name, a->raw_input);
        return 0;
    }
    else
    {
        printf("alias: %s: not found\n", name);
        return 1;
    }
}

static int print_aliases(struct shell_state *s)
{
    struct pair_list *head = s->aliases->insert_order_head;
    while (head != NULL)
    {
        struct alias *alias = head->value;
        printf("%s='%s'\n", head->key, alias->raw_input);
        head = head->insert_order_next;
    }
    return 0;
}

static int alias_argument(char *arg, struct shell_state *s)
{
    char *key;
    char *val;
    if (get_alias_key_val(arg, &key, &val) == -1)
        return -1;
    if (val == NULL)
    {
        int r = print_single_alias(key, s);
        free(key);
        return r;
    }
    struct alias *alias = create_alias(val);
    if (hash_map_insert(s->aliases, key, alias, free_alias) == -1)
    {
        free(key);
        return -1;
    }
    return 0;
}

int builtin_alias(char **args, struct shell_state *s)
{
    if (args[1] == NULL)
    {
        return print_aliases(s);
    }
    int status = 0;
    for (size_t i = 1; args[i] != NULL; i++)
    {
        if (alias_argument(args[i], s) != 0)
            status = 1;
    }
    return status;
}

int builtin_unalias(char **args, struct shell_state *s)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "unalias: usage: unalias [-a] name [name ...]\n");
        return 2;
    }
    if (strcmp(args[1], "-a") == 0)
    {
        hash_map_free(s->aliases, free_alias);
        s->aliases = hash_map_init();
        if (s->aliases == NULL)
            return -1;
        return 0;
    }
    int status = 0;
    for (size_t i = 1; args[i] != NULL; i++)
    {
        if (!hash_map_remove(s->aliases, args[i], free_alias))
        {
            fprintf(stderr, "unalias: %s: not found\n", args[i]);
            status = 1;
        }
    }
    return status;
}
