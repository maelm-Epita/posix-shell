#ifndef BUILTINS_H
#define BUILTINS_H

#include "shell/shell.h"

/**
 * @brief Builtin function signature.
 *
 * Builtins take an argv-like array and the current shell state.
 *
 * @param args Null-terminated array of arguments (argv style)
 * @param s Shell state
 * @return Exit status of the builtin
 */

typedef int (*builtin_function)(char **, struct shell_state *s);

/**
 * @brief Returns the builtin function matching a name.
 * @param name Builtin name (e.g. "echo", "exit")
 * @return The builtin function pointer, or NULL if not found
 */

builtin_function get_builtin(char *name);

/**
 * @brief Builtin that always succeeds.
 * @param args Arguments (unused)
 * @param s Shell state
 * @return Always 0
 */

int builtin_true(char **args, struct shell_state *s);

/**
 * @brief Builtin that always fails.
 * @param args Arguments (unused)
 * @param s Shell state
 * @return Always 1
 */

int builtin_false(char **args, struct shell_state *s);

/**
 * @brief Prints arguments to stdout.
 * @param args Arguments (argv style)
 * @param s Shell state
 * @return Exit status
 */

int builtin_echo(char **args, struct shell_state *s);

/**
 * @brief Exits the shell.
 * @param args Arguments (optional exit status)
 * @param s Shell state
 * @return Exit status (may not return)
 */

int builtin_exit(char **args, struct shell_state *s);

int builtin_cd(char **args, struct shell_state *s);

int builtin_export(char **args, struct shell_state *s);

int builtin_unset(char **args, struct shell_state *s);
int builtin_dot(char **args, struct shell_state *s);

int builtin_break(char **args, struct shell_state *s);
int builtin_continue(char **args, struct shell_state *s);

int builtin_alias(char **args, struct shell_state *s);
int builtin_unalias(char **args, struct shell_state *s);

#endif /* ! BUILTINS_H */
