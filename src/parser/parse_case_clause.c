#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

enum parser_status parse_case_clause(struct ast_case *ast_case,
                                     struct shell_state *state)
{
    enum parser_status status;

    if ((status = parse_case_item(ast_case, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked);
    while (peeked->type == T_DOUBLE_SEMICOL)
    {
        EAT("case clause", T_DOUBLE_SEMICOL);

        PEEK(&peeked);
        while (peeked->type == T_NL)
        {
            EAT("case clause", T_NL);
            PEEK(&peeked);
        }

        if (!is_in_case_item_clause_first(peeked))
            break;

        if ((status = parse_case_item(ast_case, state)) != P_SUCCESS)
            return status;

        PEEK(&peeked);
    }

    while (peeked->type == T_NL)
    {
        EAT("case clause", T_NL);
        PEEK(&peeked);
    }

    return P_SUCCESS;
}
