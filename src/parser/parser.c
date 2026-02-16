#include "parser.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

enum parser_status parse_input(struct ast **ast, struct shell_state *state)
{
    enum parser_status status;
    PEEK(&peeked);

    *ast = NULL;

    /* EOF */
    if (peeked->type == T_EOF)
    {
        return P_EOF;
    }
    /* '\n' */
    else if (peeked->type == T_NL)
    {
        EAT("input", T_NL);
        return P_SUCCESS;
    }

    *ast = malloc(sizeof(struct ast));
    if (*ast == NULL)
        return P_ERR;

    /* list (EOF | '\n') */
    if ((status = parse_list(*ast, state)) != P_SUCCESS)
        return status;
    PEEK(&peeked);
    if (peeked->type == T_EOF)
    {
        return P_EOF;
    }
    EAT("input", T_NL);

    return P_SUCCESS;
}
