#define _POSIX_C_SOURCE 200809L

#include "shell.h"

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "execution/execution.h"
#include "functions/functions.h"
#include "io/io.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "token/aliases.h"
#include "utils/hash_map/hash_map.h"
#include "utils/vector/vector_extra.h"
#include "variables/variables.h"

#ifdef RELEASE_BUILD
static inline void print_prompt(void)
{
    fputs("$ ", stdout);
    fflush(stdout);
}

static inline int is_interactive(struct io_source source)
{
    return source.source_type == IO_FILE && source.source_stream.file == stdin;
}
#endif /* ! RELEASE_BUILD */

static void init_shell_name(int *argc, char ***argv, struct shell_state *s)
{
    if (*argc == 0)
    {
        s->name = "42sh";
        return;
    }
    s->name = (*argv)[0];
    (*argv)++;
    (*argc)--;
    return;
}

static void shell_state_free(struct shell_state *state)
{
    vector_free(state->args);
    hash_map_free(state->variables, free_variable);
    hash_map_free(state->functions, free_function);
    hash_map_free(state->aliases, free_alias);
    free(state);
}

static struct shell_state *shell_state_init(int argc, char **argv, char **envp)
{
    struct shell_state *s = malloc(sizeof(struct shell_state));
    if (s == NULL)
        return NULL;

    init_shell_name(&argc, &argv, s);
    s->running = 1;
    s->variables = hash_map_init();
    s->functions = hash_map_init();
    s->aliases = hash_map_init();
    s->args = vector_init(sizeof(char *));
    if (s->args == NULL || vector_append_n(s->args, argv, argc) == -1)
        return NULL;
    s->last_exit_code = 0;
    s->shell_pid = getpid();
    s->interactive = 0;
    srand(time(NULL));

    if (envp_to_variables(envp, s->variables) != 0)
        return NULL;
    if (!hash_map_get(s->variables, "PWD"))
    {
        char *cwd = malloc(sizeof(char) * PATH_MAX);
        if (getcwd(cwd, PATH_MAX))
        {
            struct variable *var = create_variable(cwd, 1);
            char *key = strdup("PWD");
            if (hash_map_insert(s->variables, key, var, free_variable) == -1)
                return NULL;
        }
    }
    if (!hash_map_get(s->variables, "IFS"))
    {
        char *val = strdup(DEFAULT_IFS);
        struct variable *var = create_variable(val, 1);
        char *key = strdup("IFS");
        if (hash_map_insert(s->variables, key, var, free_variable) == -1)
            return NULL;
    }
    s->loop_depth = 0;
    s->break_depth = 0;
    s->continue_depth = 0;
    return s;
}

static int parse_exec_loop(struct shell_state *state)
{
    enum parser_status s;
    while (state->running)
    {
        struct ast *ast = NULL;

#ifdef RELEASE_BUILD
        if (state->interactive)
            print_prompt();
#endif /* ! RELEASE_BUILD */

        s = parse_input(&ast, state);
        if (s == P_ERR)
        {
            free_ast(ast);
            state->last_exit_code = RET_SHELL_INTERNAL;
            break;
        }

        if (s == P_SYNTAX_ERR)
        {
            free_ast(ast);
#ifdef RELEASE_BUILD
            if (state->interactive)
            {
                if (lexer_clear_line() == LEXER_SUCCESS)
                    continue;
            }
#endif /* ! RELEASE_BUILD */
            state->last_exit_code = RET_COMMAND_LINE_GRAMMAR;
            break;
        }

        if (ast != NULL)
        {
#ifdef PRINT_AST
            print_ast(ast);
#endif /* ! PRINT_AST */
            state->last_exit_code = execute_ast(ast, state);
        }

        free_ast(ast);
        if (s == P_EOF)
            break;
    }

    return state->last_exit_code;
}

int shell_loop(int argc, char **argv, char **envp, struct io_source source)
{
    struct shell_state *state = shell_state_init(argc, argv, envp);
    if (state == NULL)
        return -1;
    state->interactive =
        source.source_type == IO_FILE && source.source_stream.file == stdin;
    io_init(source);

    int status = parse_exec_loop(state);

    shell_state_free(state);
    return status;
}

int exec_subshell_stdout_redirected(struct shell_state *state,
                                    struct io_source source, int *p)
{
    int pid = fork();
    if (pid == -1)
    {
        return -1;
    }
    else if (pid == 0)
    {
        close(p[0]);
        if (dup2(p[1], STDOUT_FILENO) == -1)
            exit(RET_SHELL_INTERNAL);
        close(p[1]);
        io_init(source);
        state->interactive = 0;
        lexer_reset();
        int status = parse_exec_loop(state);
        shell_state_free(state);
        exit(status);
    }
    else
    {
        close(p[1]);
        int status;
        if (waitpid(pid, &status, 0) == -1)
            return RET_SHELL_INTERNAL;
        status =
            WIFEXITED(status) ? WEXITSTATUS(status) : RET_SIGNAL_TERMINATED;
        if (status == RET_SHELL_INTERNAL)
            return -1;
        state->last_exit_code = status;
        return status;
    }
}
