#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

enum parser_status parse_heredoc(struct ast_command *command,
                                 struct shell_state *state)
{
    enum parser_status status;
    (void)status;
    (void)state;
    (void)command;

    PEEK(&peeked);
    if (peeked->type != T_WORD)
        EAT("heredoc", T_WORD);

    /* TODO heredoc
    char *terminator = expand_no_fs(peeked->value.string, 0);
    EAT("heredoc", T_WORD);

    PEEK(&peeked);
    EAT("heredoc", T_NL);

    PEEK(&peeked);
    while (!(token_type_has_string_value(peeked->type)
             && strcmp(peeked->value.str_value, terminator) == 0))
    {
    }
    */

    return P_SUCCESS;
}
