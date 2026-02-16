#ifndef STRING_HELPERS_H
#define STRING_HELPERS_H

/**
 ** @brief              Converts a string to a positive integer
 ** @param string       The string to convert
 ** @return             Returns The positive integer converted from string on
 * success; -1 on failure
 */
int string_to_int(char *string);

/**
 * @brief Converts a string to an integer with optional '+' or '-' sign.
 * @param string The string to convert
 * @param out Output integer updated on success
 * @return 0 on success, non-zero on failure
 */
int string_to_int_plus_minus(char *string, int *out);

#endif /* ! STRING_HELPERS_H */
