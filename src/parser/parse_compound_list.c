#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

enum parser_status parse_compound_list(struct ast_list *list,
                                       struct shell_state *state)
{
    enum parser_status status;

    PEEK(&peeked);
    while (peeked->type == T_NL)
    {
        EAT("compound list", T_NL);
        PEEK(&peeked);
    }

    if ((status = parse_and_or(list, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked);
    while (peeked->type == T_SEMICOL || peeked->type == T_AND
           || peeked->type == T_NL)
    {
        EAT_ONE_OF_N("compound list", 3, T_SEMICOL, T_AND, T_NL);

        PEEK(&peeked);
        while (peeked->type == T_NL)
        {
            EAT("compound list", T_NL);
            PEEK(&peeked);
        }

        if (!is_in_and_or_first(peeked))
            return P_SUCCESS;
        if ((status = parse_and_or(list, state)) != P_SUCCESS)
            return status;
        PEEK(&peeked);
    }

    PEEK(&peeked);
    if (peeked->type == T_SEMICOL || peeked->type == T_AND)
        EAT_ONE_OF_N("compound list", 2, T_SEMICOL, T_AND);

    PEEK(&peeked);
    while (peeked->type == T_NL)
    {
        EAT("compound list", T_NL);
        PEEK(&peeked);
    }

    return P_SUCCESS;
}
