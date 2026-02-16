#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

enum parser_status parse_rule_while(struct ast_command *command,
                                    struct shell_state *state)
{
    enum parser_status status;
    struct ast_while *ast_while = malloc(sizeof(struct ast_while));
    if (ast_while == NULL)
        return P_ERR;
    command->command.while_statement = ast_while;
    command->type = COMMAND_WHILE;
    ast_while->do_list = NULL;
    ast_while->while_list = NULL;

    PEEK(&peeked);
    EAT("rule while", T_WHILE);

    struct ast_list *while_list = malloc(sizeof(struct ast_list));
    if (while_list == NULL)
        return P_ERR;

    while_list->list = vector_init(sizeof(struct ast_and_or *));
    if (while_list->list == NULL)
        return P_ERR;

    ast_while->while_list = while_list;

    if ((status = parse_compound_list(while_list, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked);
    EAT("rule while", T_DO);

    struct ast_list *do_list = malloc(sizeof(struct ast_list));
    if (do_list == NULL)
        return P_ERR;

    do_list->list = vector_init(sizeof(struct ast_and_or *));
    if (do_list->list == NULL)
        return P_ERR;

    ast_while->do_list = do_list;

    if ((status = parse_compound_list(do_list, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked);
    EAT("rule while", T_DONE);

    return P_SUCCESS;
}
