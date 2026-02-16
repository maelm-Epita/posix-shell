#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

enum parser_status parse_prefix(struct ast_command *command,
                                struct shell_state *state)
{
    enum parser_status status;

    PEEK(&peeked);
    if (is_assignment(peeked))
    {
        m_string cpy = copy_m_string(peeked->value.string);
        if (cpy == NULL)
            return P_ERR;
        if (vector_append(command->command.simple_command->assignments, &cpy)
            != 0)
            return P_ERR;
        EAT("prefix", T_WORD);
        return P_SUCCESS;
    }
    else
    {
        if ((status = parse_redirection(command, state)) != P_SUCCESS)
            return status;
        return P_SUCCESS;
    }
}
