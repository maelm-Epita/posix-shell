#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "execution_private.h"
#include "utils/vector/vector_extra.h"

static int clean_up_pipes_children(struct ast_pipeline *ast_pipeline,
                                   int **pipes, int *pids)
{
    int status;
    for (size_t i = 0; i < ast_pipeline->commands->size - 1; i++)
    {
        if (close(pipes[i][0]) == -1 || close(pipes[i][1]) == -1)
            return RET_SHELL_INTERNAL;
        free(pipes[i]);
    }
    for (size_t i = 0; i < ast_pipeline->commands->size; i++)
    {
        if (waitpid(pids[i], &status, 0) == -1)
            return RET_SHELL_INTERNAL;
        status =
            WIFEXITED(status) ? WEXITSTATUS(status) : RET_SIGNAL_TERMINATED;
        if (status == RET_SHELL_INTERNAL || status == RET_SIGNAL_TERMINATED)
            return status;
    }
    free(pipes);
    free(pids);
    return status;
}

static int init_pipes_pids_arrays(struct ast_pipeline *ast_pipeline,
                                  int ***pipes, int **pids)
{
    *pipes = malloc(sizeof(int *) * (ast_pipeline->commands->size - 1));
    *pids = malloc(sizeof(int) * ast_pipeline->commands->size);
    if (*pids == NULL || *pipes == NULL)
        return RET_SHELL_INTERNAL;
    for (size_t i = 0; i < ast_pipeline->commands->size - 1; i++)
    {
        (*pipes)[i] = malloc(sizeof(int) * 2);
        if ((*pipes)[i] == NULL)
            return RET_SHELL_INTERNAL;
        if (pipe((*pipes)[i]) == -1)
            return RET_SHELL_INTERNAL;
    }
    return 0;
}

int execute_pipeline(struct ast_pipeline *ast_pipeline,
                     struct shell_state *state)
{
    if (ast_pipeline->commands->size == 1)
    {
        int status = execute_command(
            vector_get_pointer(ast_pipeline->commands, 0), state);
        return ast_pipeline->negated ? (status == 0 ? 1 : 0) : status;
    }

    int **pipes;
    int *pids;
    if (init_pipes_pids_arrays(ast_pipeline, &pipes, &pids) != 0)
        return RET_SHELL_INTERNAL;

    int status = 0;
    for (size_t i = 0; i < ast_pipeline->commands->size; i++)
    {
        if (!state->running)
            break;
        int pid = fork();
        if (pid == -1)
        {
            return RET_SHELL_INTERNAL;
        }
        else if (pid == 0)
        {
            if (i != 0)
            {
                if (dup2(pipes[i - 1][0], 0) == -1)
                    goto error_cleanup;
            }
            if (i != ast_pipeline->commands->size - 1)
            {
                if (dup2(pipes[i][1], 1) == -1)
                    goto error_cleanup;
            }
            for (size_t j = 0; j < ast_pipeline->commands->size - 1; j++)
            {
                if (close(pipes[j][0]) == -1 || close(pipes[j][1]) == -1)
                    goto error_cleanup;
            }
            status = execute_command(
                vector_get_pointer(ast_pipeline->commands, i), state);
            exit(status);
        }
        else
        {
            pids[i] = pid;
        }
    }

    if ((status = clean_up_pipes_children(ast_pipeline, pipes, pids))
        == RET_SHELL_INTERNAL)
        return RET_SHELL_INTERNAL;

    return ast_pipeline->negated ? (status == 0 ? 1 : 0) : status;

error_cleanup:
    free(pipes);
    free(pids);
    exit(RET_SHELL_INTERNAL);
}
