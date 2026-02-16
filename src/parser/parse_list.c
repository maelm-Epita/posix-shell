#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"
#include "utils/vector/vector_extra.h"

static struct token *peeked = NULL;
static struct token *lookahead = NULL;

enum parser_status parse_list(struct ast *ast, struct shell_state *state)
{
    enum parser_status status;

    struct ast_list *list = malloc(sizeof(struct ast_list));
    if (list == NULL)
        return P_ERR;
    ast->list = list;

    list->list = vector_init(sizeof(struct ast_and_or *));
    if (list->list == NULL)
        return P_ERR;

    if ((status = parse_and_or(ast->list, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked)

    while (peeked->type == T_SEMICOL || peeked->type == T_AND)
    {
        if (peeked->type == T_AND)
        {
            struct ast_and_or *last =
                vector_get_pointer(list->list, list->list->size - 1);
            last->background = 1;
        }
        // case ending optionals
        LOOKAHEAD(&lookahead, 1);
        if (lookahead->type == T_NL || lookahead->type == T_EOF)
            break;

        // case in 0 to infinite
        EAT_ONE_OF_N("list", 2, T_SEMICOL, T_AND);
        if ((status = parse_and_or(ast->list, state)) != P_SUCCESS)
            return status;

        PEEK(&peeked);
    }

    // eat optional
    if (peeked->type == T_SEMICOL || peeked->type == T_AND)
    {
        EAT("list", peeked->type);
    }

    return P_SUCCESS;
}
