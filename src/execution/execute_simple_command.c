#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtins/builtins.h"
#include "execution_private.h"
#include "expansion/expansion.h"
#include "functions/functions.h"
#include "utils/vector/vector_extra.h"
#include "variables/variables.h"

extern char **environ;

static int execute_builtin(builtin_function builtin, vector *argv,
                           vector *redirections, struct shell_state *state)
{
    if (execute_redirections(redirections, state) == -1)
        return RET_SHELL_INTERNAL;
    int status = builtin(argv->data, state);
    if (revert_redirections(redirections) == -1)
        return RET_SHELL_INTERNAL;
    vector_free_pointer(argv);
    return status;
}

static int execute_function(struct function *func, vector *argv,
                            vector *redirections, struct shell_state *state)
{
    if (execute_redirections(redirections, state) == -1)
        return RET_SHELL_INTERNAL;
    vector *old_args = state->args;

    vector *args_stripped = vector_init(sizeof(char *));
    if (argv->size > 2)
    {
        char **argv_data = vector_get(argv, 1);
        if (args_stripped == NULL
            || vector_append_n(args_stripped, argv_data, argv->size - 2) == -1)
            return RET_SHELL_INTERNAL;
    }
    state->args = args_stripped;
    if (state->args == NULL)
        return RET_SHELL_INTERNAL;

    int status = execute_command(func->command, state);
    state->args = old_args;
    if (revert_redirections(redirections) == -1)
        return RET_SHELL_INTERNAL;
    vector_free(args_stripped);
    vector_free_pointer(argv);
    return status;
}

static void
execute_simple_command_child(struct ast_simple_command *ast_simple_command,
                             vector *redirections, struct shell_state *state,
                             vector *argv)
{
    if (execute_redirections(redirections, state) == -1)
        exit(RET_SHELL_INTERNAL);

    if (execute_assignments(ast_simple_command->assignments, state) == -1)
        exit(RET_SHELL_INTERNAL);

    char **new_env = NULL;
    if (variables_to_envp(&new_env, state->variables) != -1 && new_env != NULL)
    {
        environ = new_env;
    }

    char *name = vector_get_pointer(argv, 0);

    execvp(name, argv->data);

    if (errno == ENOENT)
    {
        fprintf(stderr, "42sh: command not found: %s\n", name);
        vector_free_pointer(argv);
        exit(RET_COMMAND_NOT_FOUND);
    }
    else
    {
        fprintf(stderr, "42sh: permission denied: %s\n", name);
        vector_free_pointer(argv);
        exit(RET_COMMAND_NOT_EXECUTABLE);
    }
}

int execute_simple_command(struct ast_simple_command *ast_simple_command,
                           vector *redirections, struct shell_state *state)
{
    if (ast_simple_command->arguments->size == 0)
        return execute_assignments(ast_simple_command->assignments, state);

    int empty_command = 0;
    vector *argv =
        expand_vector(ast_simple_command->arguments, state, &empty_command);
    if (empty_command)
    {
        vector_free_pointer(argv);
        return state->last_exit_code;
    }

    if (argv == NULL)
        return -1;

    char *term = NULL;
    if (vector_append(argv, &term) != 0)
        return -1;

    char *name = vector_get_pointer(argv, 0);
    struct function *function = hash_map_get(state->functions, name);

    if (function != NULL)
        return execute_function(function, argv, redirections, state);

    builtin_function builtin = get_builtin(name);

    if (builtin != NULL)
        return execute_builtin(builtin, argv, redirections, state);

    pid_t pid = fork();
    if (pid == -1)
    {
        return RET_SHELL_INTERNAL;
    }
    else if (pid == 0)
    {
        execute_simple_command_child(ast_simple_command, redirections, state,
                                     argv);
        return RET_SHELL_INTERNAL;
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
        vector_free_pointer(argv);
        status =
            WIFEXITED(status) ? WEXITSTATUS(status) : RET_SIGNAL_TERMINATED;
        state->last_exit_code = status;
        return status;
    }
}
