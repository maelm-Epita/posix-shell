
#include <stdlib.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"
#include "token/aliases.h"
#include "utils/vector/vector_extra.h"

static struct token *peeked = NULL;
static struct token *lookahead = NULL;

static struct ast_command *init_command(void)
{
    struct ast_command *command = malloc(sizeof(struct ast_command));
    if (command == NULL)
        return NULL;
    command->redirections = vector_init(sizeof(struct redirection *));
    if (command->redirections == NULL)
        return NULL;
    return command;
}

static int perform_nested_aliases(struct shell_state *s)
{
    PEEK(&peeked);
    vector *alias_tokens = perform_alias(peeked, s);
    if (alias_tokens == NULL)
        return 0;
    EAT("perform alias", peeked->type);
    for (int i = alias_tokens->size - 1; i >= 0; i--)
    {
        struct token *tok = vector_get_pointer(alias_tokens, i);
        if (lexer_queue_push_front(tok) == -1)
            return -1;
    }
    vector_free(alias_tokens);
    return 1;
}

enum parser_status parse_command_with_aliases(struct ast_command *command,
                                              struct shell_state *state)
{
    enum parser_status status;

    PEEK(&peeked);
    LOOKAHEAD(&lookahead, 1)
    if (is_in_shell_command_first(peeked))
    {
        if ((status = parse_shell_command(command, state)) != P_SUCCESS)
            return status;

        PEEK(&peeked);
        while (is_in_redirection_first(peeked))
        {
            if ((status = parse_redirection(command, state)) != P_SUCCESS)
                return status;
            PEEK(&peeked);
        }
    }
    /* if peeked is a WORD that is not in shell command's first
     * and lookahead is '(', then go into this rule
     * however funcdec should detect if WORD is a keyword and make an error
     */
    else if (is_funcdec(peeked, lookahead))
    {
        if ((status = parse_funcdec(command, state)) != P_SUCCESS)
            return status;

        PEEK(&peeked);
        while (is_in_redirection_first(peeked))
        {
            if ((status = parse_redirection(command, state)) != P_SUCCESS)
                return status;
            PEEK(&peeked);
        }
    }
    /* same here, should detect if WORD is a keyword and error */
    else
    {
        int aliases_state = perform_nested_aliases(state);
        if (aliases_state == -1)
            return P_ERR;
        if (aliases_state)
        {
            return parse_command_with_aliases(command, state);
        }
        if ((status = parse_simple_command(command, state)) != P_SUCCESS)
            return status;
    }

    return P_SUCCESS;
}

enum parser_status parse_command(struct ast_pipeline *pipeline,
                                 struct shell_state *state)
{
    struct ast_command *command = init_command();
    if (command == NULL)
        return P_ERR;
    if (vector_append(pipeline->commands, &command) != 0)
        return P_ERR;
    return parse_command_with_aliases(command, state);
}
