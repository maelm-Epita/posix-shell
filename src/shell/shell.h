#ifndef SHELL_H
#define SHELL_H

#include "io/io.h"
#include "sys/types.h"
#include "utils/vector/vector.h"
/*

Parse execution loop pseudocode:

create new envp from passed envp // this makes each subshell have its own env,
but initially inherit from its parent shell init io from passed source // main
shell can be stdio and subshells can be char stream while (!eof) get ast from
parser call execution on ast, passing envp to be updated return captured ouput

*/

#define DEFAULT_IFS " \n\t"

/**
 * @brief Common shell return codes.
 */

enum return_codes
{
    RET_COMMAND_LINE_GRAMMAR = 2,
    RET_EXPANSION_REDIRECTION = 125,
    RET_COMMAND_NOT_EXECUTABLE = 126,
    RET_COMMAND_NOT_FOUND = 127,
    RET_SIGNAL_TERMINATED = 129,
    RET_SHELL_INTERNAL = 256,
};

/**
 * @brief Shell runtime state.
 *
 * This structure stores variables, arguments and execution status.
 *
 * @param variables Shell variables map
 * @param args Shell arguments vector
 * @param last_exit_code Last command exit status
 * @param running Non-zero while the shell is running
 * @param shell_pid Shell process id
 * @param oldpwd_set Non-zero if OLDPWD is set
 */

struct shell_state
{
    struct hash_map *variables;
    struct hash_map *functions;
    struct hash_map *aliases;
    char *name;
    vector *args;
    int last_exit_code;
    int running;
    pid_t shell_pid;
    int oldpwd_set;
    int loop_depth;
    int break_depth;
    int continue_depth;
    int interactive;
};

/**
 ** @brief              Does the shell loop
 ** @param envp         The environment array
 ** @return             Returns 0 on success, -1 on failure
 */
int shell_loop(int argc, char **argv, char **envp, struct io_source source);
int exec_subshell_stdout_redirected(struct shell_state *state,
                                    struct io_source source, int *pipe);

#endif /* ! SHELL_H */
