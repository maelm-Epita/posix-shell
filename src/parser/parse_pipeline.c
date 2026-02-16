
#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

enum parser_status parse_pipeline(struct ast_and_or *and_or,
                                  struct shell_state *state)
{
    enum parser_status status;

    struct ast_pipeline *pipeline = malloc(sizeof(struct ast_and_or));
    if (pipeline == NULL)
        return P_ERR;
    pipeline->commands = vector_init(sizeof(struct ast_command *));
    if (pipeline->commands == NULL)
        return P_ERR;
    if (vector_append(and_or->pipelines, &pipeline) != 0)
        return P_ERR;
    pipeline->negated = 0;

    PEEK(&peeked);
    if (token_matches_type(peeked, T_EXCLAMATION))
    {
        EAT("pipeline", T_EXCLAMATION);
        PEEK(&peeked);
        pipeline->negated = 1;
    }

    if ((status = parse_command(pipeline, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked);

    while (peeked->type == T_PIPE)
    {
        EAT("pipeline", T_PIPE);

        PEEK(&peeked);
        while (peeked->type == T_NL)
        {
            EAT("pipeline", T_NL)
            PEEK(&peeked);
        }

        if ((status = parse_command(pipeline, state)) != P_SUCCESS)
            return status;

        PEEK(&peeked);
    }

    return P_SUCCESS;
}
