#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

static int ast_for_add_default_in(struct ast_for *ast_for)
{
    if (ast_for->words->size == 0)
    {
        m_string str = m_string_from_raw("$@");
        if (str == NULL)
            return -1;
        if (vector_append(ast_for->words, &str) == -1)
            return -1;
    }
    return 0;
}

enum parser_status parse_rule_for_do(struct ast_for *ast_for,
                                     struct shell_state *state)
{
    int status;

    PEEK(&peeked);
    EAT("rule for", T_DO);

    struct ast_list *list = malloc(sizeof(struct ast_list));
    if (list == NULL)
        return P_ERR;

    list->list = vector_init(sizeof(struct ast_and_or *));
    if (list->list == NULL)
        return P_ERR;

    ast_for->do_list = list;

    if ((status = parse_compound_list(list, state)) != P_SUCCESS)
        return status;
    return P_SUCCESS;
}

enum parser_status parse_rule_for_in(struct ast_for *ast_for)
{
    PEEK(&peeked);
    EAT("rule for", T_IN);

    PEEK(&peeked);
    while (peeked->type == T_WORD)
    {
        m_string cpy = copy_m_string(peeked->value.string);
        if (cpy == NULL)
            return P_ERR;
        if (vector_append(ast_for->words, &cpy) != 0)
            return P_ERR;
        EAT("rule for", T_WORD);
        PEEK(&peeked);
    }

    PEEK(&peeked);
    EAT_ONE_OF_N("rule for", 2, T_SEMICOL, T_NL);
    return P_SUCCESS;
}

enum parser_status parse_rule_for(struct ast_command *command,
                                  struct shell_state *state)
{
    enum parser_status status;
    struct ast_for *ast_for = malloc(sizeof(struct ast_for));
    if (ast_for == NULL)
        return P_ERR;
    command->command.for_statement = ast_for;
    command->type = COMMAND_FOR;
    ast_for->words = vector_init(sizeof(m_string));
    ast_for->do_list = NULL;

    PEEK(&peeked);
    EAT("rule for", T_FOR);

    PEEK(&peeked);
    if (peeked->type == T_WORD)
    {
        m_string cpy = copy_m_string(peeked->value.string);
        if (cpy == NULL)
            return P_ERR;
        ast_for->element_identifier = cpy;
    }
    EAT("rule for", T_WORD);

    PEEK(&peeked);
    if (peeked->type == T_SEMICOL)
    {
        EAT("rule for", T_SEMICOL);
    }
    else
    {
        if (peeked->type == T_NL || token_matches_type(peeked, T_IN))
        {
            EAT_NEWLINES();

            PEEK(&peeked);
            if (token_matches_type(peeked, T_IN))
            {
                if ((status = parse_rule_for_in(ast_for)) != P_SUCCESS)
                    return status;
            }
        }
    }

    if (ast_for_add_default_in(ast_for) == -1)
        return P_ERR;

    PEEK(&peeked);
    EAT_NEWLINES();

    if ((status = parse_rule_for_do(ast_for, state)) != P_SUCCESS)
        return status;

    PEEK(&peeked);
    EAT("rule for", T_DONE);

    return P_SUCCESS;
}
