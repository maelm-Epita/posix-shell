#include "parser_private.h"

#include <stdio.h>
#include <stdlib.h>

void print_syntax_error_near(struct token *tok)
{
    if (token_type_has_string_value(tok->type))
    {
        char *str = m_string_get_raw(tok->value.string);
        fprintf(stderr, "Syntax error near '%s'.\n", str);
    }
    else if (token_type_has_int_value(tok->type))
    {
        fprintf(stderr, "Syntax error near '%d'\n", tok->value.int_value);
    }
    else
    {
        fprintf(stderr, "Syntax error near '%s'\n",
                token_type_to_string(tok->type));
    }
}
