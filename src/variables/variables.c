#include "variables.h"

#include <stdio.h>
#include <string.h>

void free_variable(void *variable)
{
    if (variable == NULL)
        return;
    struct variable *var = variable;
    if (var->value != NULL)
        free(var->value);
    free(var);
}

struct variable *create_variable(char *value, int exported)
{
    struct variable *var = malloc(sizeof(struct variable));
    if (var == NULL)
        return NULL;
    var->exported = exported;
    var->value = value;
    return var;
}

int add_variable(char *key, char *value, int exported,
                 struct hash_map *variables)
{
    struct variable *var = create_variable(value, exported);
    if (var == NULL)
        return -1;
    return hash_map_insert(variables, key, var, free_variable);
}

int variable_from_string(char *string, struct hash_map *variables)
{
    char *from_del = strchr(string, '=');
    size_t key_size = from_del - string;
    size_t value_size = strchr(from_del, 0) - (from_del + 1);
    char *key = malloc(sizeof(char) * (key_size + 1));
    char *value = malloc(sizeof(char) * (value_size + 1));
    if (key == NULL || value == NULL)
        return -1;
    key[key_size] = 0;
    value[value_size] = 0;
    memcpy(key, string, key_size);
    memcpy(value, (string) + (key_size + 1), value_size);
    struct variable *var = create_variable(value, 1);
    if (hash_map_insert(variables, key, var, free_variable) != 0)
        return -1;
    return 0;
}

int envp_to_variables(char **envp, struct hash_map *variables)
{
    while (*envp != NULL)
    {
        if (variable_from_string(*envp, variables) != 0)
            return -1;
        envp++;
    }
    return 0;
}

int variables_to_envp(char ***envp, struct hash_map *variables)
{
    *envp = malloc(sizeof(char *) * (variables->element_count + 1));
    if (*envp == NULL)
        return -1;
    (*envp)[variables->element_count] = NULL;
    struct pair_list *el = variables->insert_order_head;
    for (size_t i = 0; el != NULL; i++)
    {
        struct variable *var = el->value;
        if (var->exported)
        {
            size_t len = strlen(el->key) + strlen(var->value) + 1;
            (*envp)[i] = malloc(sizeof(char) * (len + 1));
            if ((*envp)[i] == NULL)
                goto error;
            if (sprintf((*envp)[i], "%s=%s", el->key, var->value) == -1)
                goto error;
        }
        el = el->insert_order_next;
    }
    return variables->element_count;

error:
    free_envp(*envp);
    return -1;
}

void free_envp(char **envp)
{
    for (size_t i = 0; envp[i] != NULL; i++)
    {
        free(envp[i]);
    }
    free(envp);
}

int remove_variable(struct hash_map *variables, char *key)
{
    return hash_map_remove(variables, key, free_variable);
}
