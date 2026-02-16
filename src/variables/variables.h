#ifndef VARIABLES_H
#define VARIABLES_H

#include "utils/hash_map/hash_map.h"

/**
 * @brief Represents a shell variable.
 *
 * @param value Variable value
 * @param exported Non-zero if exported to the environment
 */
struct variable
{
    char *value;
    int exported;
};

/**
 * @brief Creates a variable object.
 * @param value Variable value
 * @param exported Non-zero if exported
 * @return Newly allocated variable, or NULL on failure
 */
struct variable *create_variable(char *value, int exported);

/**
 * @brief Frees a variable object (hash_map compatible).
 * @param variable Pointer to the variable to free
 */
void free_variable(void *variable);

/**
 * @brief Adds or updates a variable in the map.
 * @param key Variable name
 * @param value Variable value
 * @param exported Non-zero if exported
 * @param variables Variables map
 * @return 0 on success, non-zero on error
 */
int add_variable(char *key, char *value, int exported,
                 struct hash_map *variables);

/**
 * @brief Parses an assignment string and stores it as a variable.
 * @param string Assignment string (e.g. NAME=VALUE)
 * @param variables Variables map
 * @return 0 on success, non-zero on error
 */
int variable_from_string(char *string, struct hash_map *variables);

/**
 * @brief Imports envp entries into the variables map.
 * @param envp Environment array
 * @param variables Variables map
 * @return 0 on success, non-zero on error
 */
int envp_to_variables(char **envp, struct hash_map *variables);

/**
 * @brief Exports variables to a newly allocated envp array.
 * @param envp Output envp array
 * @param variables Variables map
 * @return 0 on success, non-zero on error
 */
int variables_to_envp(char ***envp, struct hash_map *variables);

/**
 * @brief Frees an envp array.
 * @param envp Env array to free
 */
void free_envp(char **envp);

int add_variable(char *key, char *value, int exported,
                 struct hash_map *variables);

int remove_variable(struct hash_map *variables, char *key);

#endif /* ! VARIABLES_H */
