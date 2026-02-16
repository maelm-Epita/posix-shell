#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer/lexer.h"
#include "parser_helpers.h"
#include "parser_private.h"

static struct token *peeked = NULL;

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

static int is_valid_name(char *str)
{
    if (isdigit(str[0]))
        return 0;
    int has_alpha_under = 0;
    for (size_t i = 0; str[i] != 0; i++)
    {
        if (!isalnum(str[i]) && str[i] != '_')
            return 0;
        if (isalpha(str[i]) || str[i] == '_')
            has_alpha_under = 1;
    }
    return has_alpha_under;
}

enum parser_status parse_funcdec(struct ast_command *command,
                                 struct shell_state *state)
{
    enum parser_status status;
    struct ast_funcdec *funcdec = malloc(sizeof(struct ast_funcdec));
    if (funcdec == NULL)
        return P_ERR;
    funcdec->command = init_command();
    if (funcdec->command == NULL)
        return P_ERR;
    funcdec->identifier = NULL;
    command->command.funcdec = funcdec;
    command->type = COMMAND_FUNCDEC;

    PEEK(&peeked);
    if (peeked->type == T_WORD)
    {
        char *name = m_string_get_raw(peeked->value.string);
        if (!is_valid_name(name))
        {
            fprintf(stderr, "42sh: %s: not a valid identifier\n", name);
            return P_SYNTAX_ERR;
        }
        funcdec->identifier = strdup(name);
        if (funcdec->identifier == NULL)
            return P_ERR;
    }
    EAT("function declaration", T_WORD);

    PEEK(&peeked);
    EAT("function declaration", T_LEFT_PAREN);
    PEEK(&peeked);
    EAT("function declaration", T_RIGHT_PAREN);

    PEEK(&peeked);
    while (peeked->type == T_NL)
    {
        EAT("function declaration", T_NL);
        PEEK(&peeked);
    }

    if ((status = parse_shell_command(funcdec->command, state)) != P_SUCCESS)
        return status;

    return P_SUCCESS;
}
