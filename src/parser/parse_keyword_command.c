#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

enum parser_status parse_keyword_command(struct ast_command *command,
                                         struct shell_state *state)
{
    enum parser_status status;
    PEEK(&peeked);
    if (token_matches_type(peeked, T_FOR))
    {
        if ((status = parse_rule_for(command, state)) != P_SUCCESS)
            return status;
        return P_SUCCESS;
    }
    else if (token_matches_type(peeked, T_WHILE))
    {
        if ((status = parse_rule_while(command, state)) != P_SUCCESS)
            return status;
        return P_SUCCESS;
    }
    else if (token_matches_type(peeked, T_UNTIL))
    {
        if ((status = parse_rule_until(command, state)) != P_SUCCESS)
            return status;
        return P_SUCCESS;
    }
    else if (token_matches_type(peeked, T_CASE))
    {
        if ((status = parse_rule_case(command, state)) != P_SUCCESS)
            return status;
        return P_SUCCESS;
    }
    else if (token_matches_type(peeked, T_IF))
    {
        if ((status = parse_rule_if(command, state)) != P_SUCCESS)
            return status;
        return P_SUCCESS;
    }
    else
    {
        print_syntax_error_near(peeked);
        return P_SYNTAX_ERR;
    }
}
