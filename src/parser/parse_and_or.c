#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

enum parser_status parse_and_or(struct ast_list *ast_list,
                                struct shell_state *state)
{
    enum parser_status status;

    struct ast_and_or *and_or = malloc(sizeof(struct ast_and_or));
    if (and_or == NULL)
        return P_ERR;
    and_or->pipelines = vector_init(sizeof(struct pipeline *));
    if (and_or->pipelines == NULL)
        return P_ERR;
    and_or->connection_types = vector_init(sizeof(enum and_or_type));
    if (and_or->connection_types == NULL)
        return P_ERR;
    if (vector_append(ast_list->list, &and_or) != 0)
        return P_ERR;
    and_or->background = 0;

    if ((status = parse_pipeline(and_or, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked);

    while (peeked->type == T_DOUBLE_AND || peeked->type == T_DOUBLE_PIPE)
    {
        if (peeked->type == T_DOUBLE_AND)
        {
            EAT("and_or", T_DOUBLE_AND);
            enum and_or_type type = AND_OR_AND;
            if (vector_append(and_or->connection_types, &type) != 0)
                return P_ERR;
        }
        else if (peeked->type == T_DOUBLE_PIPE)
        {
            EAT("and_or", T_DOUBLE_PIPE);
            enum and_or_type type = AND_OR_OR;
            if (vector_append(and_or->connection_types, &type) != 0)
                return P_ERR;
        }

        PEEK(&peeked);
        while (peeked->type == T_NL)
        {
            EAT("and_or", T_NL)
            PEEK(&peeked);
        }

        if ((status = parse_pipeline(and_or, state)) != P_SUCCESS)
            return status;

        PEEK(&peeked);
    }

    return P_SUCCESS;
}
