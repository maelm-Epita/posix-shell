#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

enum parser_status parse_rule_until(struct ast_command *command,
                                    struct shell_state *state)
{
    enum parser_status status;
    struct ast_until *ast_until = malloc(sizeof(struct ast_until));
    if (ast_until == NULL)
        return P_ERR;
    command->command.until_statement = ast_until;
    command->type = COMMAND_UNTIL;
    ast_until->do_list = NULL;
    ast_until->until_list = NULL;

    PEEK(&peeked);
    EAT("rule until", T_UNTIL);

    struct ast_list *until_list = malloc(sizeof(struct ast_list));
    if (until_list == NULL)
        return P_ERR;

    until_list->list = vector_init(sizeof(struct ast_and_or *));
    if (until_list->list == NULL)
        return P_ERR;

    ast_until->until_list = until_list;

    if ((status = parse_compound_list(until_list, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked);
    EAT("rule until", T_DO);

    struct ast_list *do_list = malloc(sizeof(struct ast_list));
    if (do_list == NULL)
        return P_ERR;

    do_list->list = vector_init(sizeof(struct ast_and_or *));
    if (do_list->list == NULL)
        return P_ERR;

    ast_until->do_list = do_list;

    if ((status = parse_compound_list(do_list, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked);
    EAT("rule until", T_DONE);

    return P_SUCCESS;
}
