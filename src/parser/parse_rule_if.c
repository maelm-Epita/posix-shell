#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

static struct ast_if *init_if(void)
{
    struct ast_if *ast_if = malloc(sizeof(struct ast_if));
    if (ast_if == NULL)
        return NULL;
    ast_if->if_lists = vector_init(sizeof(struct ast_list *));
    if (ast_if->if_lists == NULL)
        return NULL;
    ast_if->then_lists = vector_init(sizeof(struct ast_list *));
    if (ast_if->then_lists == NULL)
        return NULL;
    return ast_if;
}

enum parser_status parse_rule_if(struct ast_command *command,
                                 struct shell_state *state)
{
    enum parser_status status;
    struct ast_if *ast_if = init_if();
    if (ast_if == NULL)
        return P_ERR;
    command->command.if_statement = ast_if;
    command->type = COMMAND_IF;

    PEEK(&peeked);
    EAT("rule if", T_IF);

    struct ast_list *if_list = malloc(sizeof(struct ast_list));
    if (if_list == NULL)
        return P_ERR;

    if_list->list = vector_init(sizeof(struct ast_and_or *));
    if (if_list->list == NULL)
        return P_ERR;

    if (vector_append(ast_if->if_lists, &if_list) != 0)
        return P_ERR;

    if ((status = parse_compound_list(if_list, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked);
    EAT("rule if", T_THEN);

    struct ast_list *then_list = malloc(sizeof(struct ast_list));
    if (then_list == NULL)
        return P_ERR;

    then_list->list = vector_init(sizeof(struct ast_and_or *));
    if (then_list->list == NULL)
        return P_ERR;

    if (vector_append(ast_if->then_lists, &then_list) != 0)
        return P_ERR;

    if ((status = parse_compound_list(then_list, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked);

    if (is_in_else_clause_first(peeked))
    {
        if ((status = parse_else_clause(ast_if, state)) != P_SUCCESS)
            return status;
    }

    PEEK(&peeked);
    EAT("rule if", T_FI);

    return P_SUCCESS;
}
