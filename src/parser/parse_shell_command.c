#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

enum parser_status parse_shell_command(struct ast_command *command,
                                       struct shell_state *state)
{
    enum parser_status status;

    PEEK(&peeked);

    if (peeked->type == T_LEFT_PAREN)
    {
        EAT("shell command", T_LEFT_PAREN);

        struct ast_subshell *subshell = malloc(sizeof(struct ast_subshell));
        if (!subshell)
            return P_ERR;

        subshell->list = malloc(sizeof(struct ast_list));
        if (!subshell->list)
            return P_ERR;

        subshell->list->list = vector_init(sizeof(struct ast_and_or *));

        command->command.subshell = subshell;
        command->type = COMMAND_SUBSHELL;

        if ((status = parse_compound_list(subshell->list, state)) != P_SUCCESS)
            return status;

        PEEK(&peeked);
        EAT("shell command", T_RIGHT_PAREN);
        return P_SUCCESS;
    }

    else if (token_matches_type(peeked, T_LEFT_BRACK))
    {
        EAT("shell command", T_LEFT_BRACK);

        struct ast_list *list = malloc(sizeof(struct ast_list));
        if (list == NULL)
            return P_ERR;

        command->command.list = list;
        command->type = COMMAND_LIST;

        list->list = vector_init(sizeof(struct ast_and_or *));
        if (list->list == NULL)
            return P_ERR;

        if ((status = parse_compound_list(list, state)) != P_SUCCESS)
            return status;

        PEEK(&peeked);
        EAT("shell command", T_RIGHT_BRACK);
        return P_SUCCESS;
    }
    else
    {
        return parse_keyword_command(command, state);
    }
}
