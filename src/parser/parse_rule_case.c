#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

enum parser_status parse_rule_case(struct ast_command *command,
                                   struct shell_state *state)
{
    enum parser_status status;
    struct ast_case *ast_case = malloc(sizeof(struct ast_case));
    if (ast_case == NULL)
        return P_ERR;
    ast_case->word = NULL;
    ast_case->branches = vector_init(sizeof(struct ast_case_branch *));
    if (ast_case->branches == NULL)
        return P_ERR;
    command->command.case_statement = ast_case;
    command->type = COMMAND_CASE;

    PEEK(&peeked);
    EAT("rule case", T_CASE);

    PEEK(&peeked);
    if (peeked->type == T_WORD)
    {
        m_string cpy = copy_m_string(peeked->value.string);
        if (cpy == NULL)
            return P_ERR;
        ast_case->word = cpy;
    }
    EAT("rule case", T_WORD);

    PEEK(&peeked);
    while (peeked->type == T_NL)
    {
        EAT("rule case", T_NL);
        PEEK(&peeked);
    }

    PEEK(&peeked);
    EAT("rule case", T_IN);

    PEEK(&peeked);
    while (peeked->type == T_NL)
    {
        EAT("rule case", T_NL);
        PEEK(&peeked);
    }

    if (is_in_case_item_clause_first(peeked))
    {
        if ((status = parse_case_clause(ast_case, state)) != P_SUCCESS)
            return status;
    }

    PEEK(&peeked);
    EAT("rule case", T_ESAC);

    return P_SUCCESS;
}
