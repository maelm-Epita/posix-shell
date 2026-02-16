#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "execution_private.h"
#include "expansion/expansion.h"
#include "utils/string/string_helpers.h"
#include "utils/vector/vector_extra.h"

static int execute_redirection_heredoc(struct redirection *redirection,
                                       int strip_tabs, char *right)
{
    if (strip_tabs)
    {
        // TODO
        return 0;
    }
    else
    {
        int p[2];
        if (pipe(p) == -1)
            return -1;
        if (write(p[1], redirection->right_string, strlen(right)) == -1)
            return -1;
        if (close(p[1]) == -1)
            return -1;
        if (dup2(p[0], redirection->ionumber) == -1)
            return -1;
        return close(p[0]);
    }
}

static int execute_redirection_right_left(struct redirection *redirection,
                                          char *right)
{
    int fd = open(right, O_RDWR | O_CREAT, 0644);
    if (fd == -1)
        return -1;
    if (dup2(fd, redirection->ionumber) == -1)
        return -1;
    return close(fd);
}

static int execute_redirection_left(struct redirection *redirection, int fd,
                                    char *right)
{
    if (fd)
    {
        if (right[0] == '-' && right[1] == 0)
        {
            return close(redirection->ionumber);
        }
        else
        {
            int r = string_to_int(right);
            if (r < 0)
                return -1;
            if (dup2(r, redirection->ionumber) == -1)
                return -1;
            return close(r);
        }
    }
    else
    {
        int fd = open(right, O_RDONLY, 0644);
        if (fd == -1)
            return -1;
        if (dup2(fd, redirection->ionumber) == -1)
            return -1;
        return close(fd);
    }
}

static int execute_redirection_right(struct redirection *redirection,
                                     int append, int fd, char *right)
{
    if (fd)
    {
        if (right[0] == '-' && right[1] == 0)
        {
            return close(redirection->ionumber);
        }
        else
        {
            int r = string_to_int(right);
            if (r < 0)
                return -1;
            if (dup2(r, redirection->ionumber) == -1)
                return -1;
            return close(r);
        }
    }
    else
    {
        int flags = O_CREAT | O_WRONLY;
        flags |= append ? O_APPEND : O_TRUNC;
        int fd = open(right, flags, 0644);
        if (fd == -1)
            return -1;
        if (dup2(fd, redirection->ionumber) == -1)
            return -1;
        return close(fd);
    }
}

static int setup_redirection(struct redirection *redirection,
                             struct shell_state *state, char **right)
{
    redirection->saved_fd = dup(redirection->ionumber);
    vector *right_fields = expand(redirection->right_string, state, NULL);
    *right = ifs_join(right_fields, state, 0);
    vector_free_pointer(right_fields);
    if (right == NULL)
        return -1;
    if (fcntl(redirection->saved_fd, F_SETFD, FD_CLOEXEC) == -1)
        return -1;
    if (redirection->saved_fd == -1)
        return -1;
    return 0;
}

int execute_redirections(vector *redirections, struct shell_state *state)
{
    int status = 0;
    for (size_t i = 0; i < redirections->size; i++)
    {
        struct redirection *redirection = vector_get_pointer(redirections, i);
        char *right;
        if (setup_redirection(redirection, state, &right) != 0)
        {
            free(right);
            return -1;
        }
        switch (redirection->type)
        {
        case REDIR_RIGHT:
        case REDIR_RIGHT_OVERWRITE:
            status = execute_redirection_right(redirection, 0, 0, right);
            break;
        case REDIR_RIGHT_APPEND:
            status = execute_redirection_right(redirection, 1, 0, right);
            break;
        case REDIR_RIGHT_FD:
            status = execute_redirection_right(redirection, 0, 1, right);
            break;
        case REDIR_LEFT:
            status = execute_redirection_left(redirection, 0, right);
            break;
        case REDIR_LEFT_FD:
            status = execute_redirection_left(redirection, 1, right);
            break;
        case REDIR_RIGHT_LEFT:
            status = execute_redirection_right_left(redirection, right);
            break;
        case REDIR_HEREDOC:
            status = execute_redirection_heredoc(redirection, 0, right);
            break;
        case REDIR_HEREDOC_MINUS:
            status = execute_redirection_heredoc(redirection, 1, right);
            break;
        }
        free(right);
        if (status == -1)
            return -1;
    }
    return 0;
}

int revert_redirections(vector *redirections)
{
    for (size_t i = 0; i < redirections->size; i++)
    {
        struct redirection *redirection = vector_get_pointer(redirections, i);
        if (dup2(redirection->saved_fd, redirection->ionumber) == -1)
            return -1;
        if (close(redirection->saved_fd) == -1)
            return -1;
    }
    return 0;
}
