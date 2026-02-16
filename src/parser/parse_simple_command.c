
#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

static struct ast_simple_command *init_simple_command(void)
{
    struct ast_simple_command *simple_command =
        malloc(sizeof(struct ast_simple_command));
    if (simple_command == NULL)
        return NULL;
    simple_command->arguments = vector_init(sizeof(m_string));
    if (simple_command->arguments == NULL)
        return NULL;
    simple_command->assignments = vector_init(sizeof(m_string));
    if (simple_command->assignments == NULL)
        return NULL;
    return simple_command;
}

enum parser_status parse_simple_command(struct ast_command *command,
                                        struct shell_state *state)
{
    enum parser_status status;

    struct ast_simple_command *simple_command = init_simple_command();
    if (simple_command == NULL)
        return P_ERR;
    command->command.simple_command = simple_command;
    command->type = COMMAND_SIMPLE;

    int had_prefix = 0;
    PEEK(&peeked);
    while (is_in_prefix_first(peeked))
    {
        had_prefix = 1;
        if ((status = parse_prefix(command, state)) != P_SUCCESS)
            return status;
        PEEK(&peeked);
    }

    /* { prefix } WORD { element } */
    if (peeked->type == T_WORD)
    {
        if (is_reserved_keyword(peeked))
        {
            print_syntax_error_near(peeked);
            return P_SYNTAX_ERR;
        }

        m_string cpy = copy_m_string(peeked->value.string);
        if (cpy == NULL)
            return P_ERR;
        if (vector_append(simple_command->arguments, &cpy) != 0)
            return P_ERR;
        EAT("simple_command", T_WORD);
        PEEK(&peeked);
        while (is_in_element_first(peeked))
        {
            if ((status = parse_element(command, state)) != P_SUCCESS)
                return status;
            PEEK(&peeked);
        }

        return P_SUCCESS;
    }
    /* prefix { prefix } */
    else
    {
        if (!had_prefix)
        {
            print_syntax_error_near(peeked);
            return P_SYNTAX_ERR;
        }
        return P_SUCCESS;
    }
}
