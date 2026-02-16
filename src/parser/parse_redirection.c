#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"
#include "utils/vector/vector_extra.h"

static struct token *peeked = NULL;

static void set_default_ionumber(struct redirection *redirection)
{
    if (redirection->ionumber != -1)
        return;
    switch (redirection->type)
    {
    case REDIR_RIGHT:
    case REDIR_RIGHT_APPEND:
    case REDIR_RIGHT_OVERWRITE:
    case REDIR_RIGHT_FD:
        redirection->ionumber = 1;
        break;
    case REDIR_LEFT:
    case REDIR_HEREDOC:
    case REDIR_HEREDOC_MINUS:
    case REDIR_RIGHT_LEFT:
    case REDIR_LEFT_FD:
        redirection->ionumber = 0;
        break;
    }
}

static void assign_redirection_type(struct redirection *redirection)
{
    if (peeked->type == T_GT)
    {
        redirection->type = REDIR_RIGHT;
    }
    else if (peeked->type == T_LT)
    {
        redirection->type = REDIR_LEFT;
    }
    else if (peeked->type == T_DOUBLE_GT)
    {
        redirection->type = REDIR_RIGHT_APPEND;
    }
    else if (peeked->type == T_GT_AND)
    {
        redirection->type = REDIR_RIGHT_FD;
    }
    else if (peeked->type == T_LT_AND)
    {
        redirection->type = REDIR_LEFT_FD;
    }
    else if (peeked->type == T_GT_PIPE)
    {
        redirection->type = REDIR_RIGHT_OVERWRITE;
    }
    else if (peeked->type == T_LT_GT)
    {
        redirection->type = REDIR_RIGHT_LEFT;
    }
}

enum parser_status parse_redirection(struct ast_command *command,
                                     struct shell_state *state)
{
    enum parser_status status;
    struct redirection *redirection = malloc(sizeof(struct redirection));
    redirection->ionumber = -1;
    redirection->saved_fd = -1;
    redirection->right_string = NULL;
    if (vector_append(command->redirections, &redirection) != 0)
        return P_ERR;

    PEEK(&peeked)
    if (peeked->type == T_IONUMBER)
    {
        redirection->ionumber = peeked->value.int_value;
        EAT("redirection", T_IONUMBER);
        PEEK(&peeked);
    }

    /* (<< | <<-) HEREDOC */
    if (peeked->type == T_DOUBLE_LT || peeked->type == T_DOUBLE_LT_MINUS)
    {
        if (peeked->type == T_DOUBLE_LT)
        {
            redirection->type = REDIR_HEREDOC;
        }
        else if (peeked->type == T_DOUBLE_LT_MINUS)
        {
            redirection->type = REDIR_HEREDOC_MINUS;
        }
        EAT_ONE_OF_N("redirection", 2, T_DOUBLE_LT, T_DOUBLE_LT_MINUS);
        if ((status = parse_heredoc(command, state)) != P_SUCCESS)
            return status;
        return P_SUCCESS;
    }
    /* Regular redirection */
    else
    {
        assign_redirection_type(redirection);
        EAT_ONE_OF_N("redirection", 7, T_GT, T_LT, T_DOUBLE_GT, T_GT_AND,
                     T_LT_AND, T_GT_PIPE, T_LT_GT);
        PEEK(&peeked)
        if (peeked->type == T_WORD)
        {
            m_string cpy = copy_m_string(peeked->value.string);
            if (cpy == NULL)
                return P_ERR;
            redirection->right_string = cpy;
        }
        EAT("redirection", T_WORD);
        set_default_ionumber(redirection);
        return P_SUCCESS;
    }
}
