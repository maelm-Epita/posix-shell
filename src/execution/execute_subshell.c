#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "execution_private.h"

int execute_subshell(struct ast_subshell *ast_subshell,
                     struct shell_state *state)
{
    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork");
        return RET_SHELL_INTERNAL;
    }
    else if (pid == 0)
    {
        int status = execute_list(ast_subshell->list, state);

        exit(status);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status))
            return WEXITSTATUS(status);
        if (WIFSIGNALED(status))
            return 128 + WTERMSIG(status);

        return status;
    }
}
