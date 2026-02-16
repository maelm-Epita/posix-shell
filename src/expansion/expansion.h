#ifndef EXPANSION_H
#define EXPANSION_H

#include "shell/shell.h"
#include "token/string_metadata.h"

/*

Expansion loop pseudocode:

If string:
    return expanded from envp
If subshell:
    call parser execute loop with envp and value to expand as char stream source
    return captured output

*/

/**
 * @brief Expands a string using the current shell state.
 * @param str The string to expand
 * @param state Shell state
 * @return A vector of expanded fields
 */

vector *expand(m_string str, struct shell_state *state, int *is_empty_command);

/**
 * @brief Expands each string in a vector.
 * @param strings Vector of strings to expand
 * @param state Shell state
 * @return A vector containing the expanded fields
 */

vector *expand_vector(vector *strings, struct shell_state *state,
                      int *is_empty_command);

/**
 * @brief Joins fields using IFS rules.
 * @param fields Vector of fields to join
 * @param state Shell state (provides IFS)
 * @param offset Starting index in the fields vector
 * @return A newly allocated joined string
 */

char *ifs_join(vector *fields, struct shell_state *state, size_t offset);

#endif /* EXPANSION_H */
