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

static struct ast_case_branch *init_case_branch(void)
{
    struct ast_case_branch *case_branch =
        malloc(sizeof(struct ast_case_branch));
    if (case_branch == NULL)
        return NULL;

    case_branch->patterns = vector_init(sizeof(m_string));
    if (case_branch->patterns == NULL)
    {
        free(case_branch);
        return NULL;
    }

    case_branch->list = NULL;
    return case_branch;
}

static enum parser_status push_case_pattern(struct ast_case_branch *branch)
{
    m_string cpy = copy_m_string(peeked->value.string);
    if (cpy == NULL)
        return P_ERR;

    if (vector_append(branch->patterns, &cpy) != 0)
    {
        free_m_string(cpy);
        return P_ERR;
    }
    return P_SUCCESS;
}

enum parser_status parse_case_item(struct ast_case *ast_case,
                                   struct shell_state *state)
{
    enum parser_status status;

    struct ast_case_branch *case_branch = init_case_branch();
    if (case_branch == NULL)
        return P_ERR;

    if (vector_append(ast_case->branches, &case_branch) != 0)
    {
        vector_free(case_branch->patterns);
        free(case_branch);
        return P_ERR;
    }

    PEEK(&peeked);
    if (peeked->type == T_LEFT_PAREN)
    {
        EAT("case item", T_LEFT_PAREN);
        PEEK(&peeked);
    }

    if ((status = push_case_pattern(case_branch)) != P_SUCCESS)
        return status;
    EAT("case item", T_WORD);

    PEEK(&peeked);
    while (peeked->type == T_PIPE)
    {
        EAT("case item", T_PIPE);

        PEEK(&peeked);
        if ((status = push_case_pattern(case_branch)) != P_SUCCESS)
            return status;
        EAT("case item", T_WORD);

        PEEK(&peeked);
    }

    EAT("case item", T_RIGHT_PAREN);

    PEEK(&peeked);
    while (peeked->type == T_NL)
    {
        EAT("case item", T_NL);
        PEEK(&peeked);
    }

    if (is_in_compound_list_first(peeked))
    {
        case_branch->list = init_list();
        if (case_branch->list == NULL)
            return P_ERR;

        if ((status = parse_compound_list(case_branch->list, state))
            != P_SUCCESS)
            return status;
    }

    return P_SUCCESS;
}
