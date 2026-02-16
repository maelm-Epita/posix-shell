#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

static struct ast_list *init_list(void)
{
    struct ast_list *list = malloc(sizeof(struct ast_list));
    if (list == NULL)
        return NULL;

    list->list = vector_init(sizeof(struct ast_and_or *));
    if (list->list == NULL)
        return NULL;
    return list;
}

enum parser_status parse_else_elif(struct ast_if *ast_if,
                                   struct shell_state *state)
{
    int status;
    EAT("else clause", T_ELIF);

    struct ast_list *if_list = init_list();
    if (if_list == NULL)
        return P_ERR;

    if (vector_append(ast_if->if_lists, &if_list) != 0)
        return P_ERR;

    if ((status = parse_compound_list(if_list, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked);
    EAT("else clause", T_THEN);

    struct ast_list *then_list = init_list();
    if (then_list == NULL)
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
    return P_SUCCESS;
}

enum parser_status parse_else_clause(struct ast_if *ast_if,
                                     struct shell_state *state)
{
    enum parser_status status;

    PEEK(&peeked);
    if (token_matches_type(peeked, T_ELSE))
    {
        EAT("else clause", T_ELSE);

        struct ast_list *then_list = init_list();
        if (then_list == NULL)
            return P_ERR;

        if (vector_append(ast_if->then_lists, &then_list) != 0)
            return P_ERR;

        if ((status = parse_compound_list(then_list, state)) != P_SUCCESS)
            return status;
        return P_SUCCESS;
    }
    else if (token_matches_type(peeked, T_ELIF))
    {
        if ((status = parse_else_elif(ast_if, state)) != P_SUCCESS)
            return status;
    }
    else
    {
        // should never happen
        print_syntax_error_near(peeked);
        return P_SYNTAX_ERR;
    }
    return P_SUCCESS;
}
