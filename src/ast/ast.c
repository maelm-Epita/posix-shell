#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

#include "expansion/expansion.h"
#include "utils/vector/vector_extra.h"

static void free_list(struct ast_list *ast_list);

void free_ast_list(struct ast_list *list)
{
    free_list(list);
}

void free_ast_subshell(struct ast_subshell *sub)
{
    if (!sub)
        return;
    if (sub->list != NULL)
        free_list(sub->list);
    free(sub);
}

static void free_case(struct ast_case *ast_case)
{
    if (ast_case == NULL)
        return;
    if (ast_case->word != NULL)
        free_m_string(ast_case->word);
    if (ast_case->branches != NULL)
    {
        for (size_t i = 0; i < ast_case->branches->size; i++)
        {
            struct ast_case_branch *branch =
                vector_get_pointer(ast_case->branches, i);
            if (branch == NULL)
                continue;
            if (branch->list != NULL)
                free_list(branch->list);
            if (branch->patterns != NULL)
            {
                for (size_t j = 0; j < branch->patterns->size; j++)
                {
                    m_string pat = vector_get_pointer(branch->patterns, j);
                    if (pat != NULL)
                        free_m_string(pat);
                }
                vector_free(branch->patterns);
            }
            free(branch);
        }
        vector_free(ast_case->branches);
    }
    free(ast_case);
}

static void free_for(struct ast_for *ast_for)
{
    if (ast_for == NULL)
        return;
    if (ast_for->words != NULL)
    {
        for (size_t i = 0; i < ast_for->words->size; i++)
            free_m_string(vector_get_pointer(ast_for->words, i));
        vector_free(ast_for->words);
    }
    if (ast_for->element_identifier != NULL)
        free_m_string(ast_for->element_identifier);
    if (ast_for->do_list != NULL)
        free_list(ast_for->do_list);
    free(ast_for);
}

static void free_funcdec(struct ast_funcdec *ast_funcdec)
{
    if (ast_funcdec == NULL)
        return;
    if (ast_funcdec->identifier != NULL)
        free(ast_funcdec->identifier);
    if (ast_funcdec->command != NULL)
        free_command(ast_funcdec->command);
    free(ast_funcdec);
}

static void free_if(struct ast_if *ast_if)
{
    if (ast_if == NULL)
        return;
    if (ast_if->if_lists != NULL)
    {
        for (size_t i = 0; i < ast_if->if_lists->size; i++)
            free_list(vector_get_pointer(ast_if->if_lists, i));
        vector_free(ast_if->if_lists);
    }
    if (ast_if->then_lists != NULL)
    {
        for (size_t i = 0; i < ast_if->then_lists->size; i++)
            free_list(vector_get_pointer(ast_if->then_lists, i));
        vector_free(ast_if->then_lists);
    }
    free(ast_if);
}

static void free_simple(struct ast_simple_command *ast_simple)
{
    if (ast_simple == NULL)
        return;
    if (ast_simple->arguments != NULL)
    {
        for (size_t i = 0; i < ast_simple->arguments->size; i++)
            free_m_string(vector_get_pointer(ast_simple->arguments, i));
        vector_free(ast_simple->arguments);
    }
    if (ast_simple->assignments != NULL)
    {
        for (size_t i = 0; i < ast_simple->assignments->size; i++)
            free_m_string(vector_get_pointer(ast_simple->assignments, i));
        vector_free(ast_simple->assignments);
    }
    free(ast_simple);
}

static void free_until(struct ast_until *ast_until)
{
    if (ast_until == NULL)
        return;
    if (ast_until->do_list != NULL)
        free_list(ast_until->do_list);
    if (ast_until->until_list != NULL)
        free_list(ast_until->until_list);
    free(ast_until);
}

static void free_while(struct ast_while *ast_while)
{
    if (ast_while == NULL)
        return;
    if (ast_while->do_list != NULL)
        free_list(ast_while->do_list);
    if (ast_while->while_list != NULL)
        free_list(ast_while->while_list);
    free(ast_while);
}

void free_command(struct ast_command *command)
{
    if (command == NULL)
        return;
    if (command->redirections != NULL)
    {
        for (size_t i = 0; i < command->redirections->size; i++)
        {
            struct redirection *redirection =
                vector_get_pointer(command->redirections, i);
            if (redirection->right_string != NULL)
                free_m_string(redirection->right_string);
            free(redirection);
        }
        vector_free(command->redirections);
    }

    switch (command->type)
    {
    case COMMAND_CASE:
        free_case(command->command.case_statement);
        break;
    case COMMAND_FOR:
        free_for(command->command.for_statement);
        break;
    case COMMAND_FUNCDEC:
        free_funcdec(command->command.funcdec);
        break;
    case COMMAND_IF:
        free_if(command->command.if_statement);
        break;
    case COMMAND_LIST:
        free_list(command->command.list);
        break;
    case COMMAND_SIMPLE:
        free_simple(command->command.simple_command);
        break;
    case COMMAND_UNTIL:
        free_until(command->command.until_statement);
        break;
    case COMMAND_WHILE:
        free_while(command->command.while_statement);
        break;
    case COMMAND_SUBSHELL:
        free_ast_subshell(command->command.subshell);
        break;
    }
    free(command);
}

static void free_list(struct ast_list *ast_list)
{
    if (ast_list == NULL)
        return;
    if (ast_list->list == NULL)
    {
        free(ast_list);
        return;
    }

    vector *list = ast_list->list;
    for (size_t i = 0; i < list->size; i++)
    {
        struct ast_and_or *and_or = vector_get_pointer(list, i);
        if (and_or->pipelines != NULL)
        {
            for (size_t j = 0; j < and_or->pipelines->size; j++)
            {
                struct ast_pipeline *pipeline =
                    vector_get_pointer(and_or->pipelines, j);
                if (pipeline->commands != NULL)
                {
                    for (size_t w = 0; w < pipeline->commands->size; w++)
                    {
                        struct ast_command *command =
                            vector_get_pointer(pipeline->commands, w);
                        free_command(command);
                    }
                    vector_free(pipeline->commands);
                }
                free(pipeline);
            }
            vector_free(and_or->pipelines);
        }
        if (and_or->connection_types)
            vector_free(and_or->connection_types);
        free(and_or);
    }
    vector_free(ast_list->list);
    free(ast_list);
}

void free_ast(struct ast *ast)
{
    if (ast == NULL)
        return;
    free_list(ast->list);
    free(ast);
}

#ifdef PRINT_AST
static void print_simple_command(struct ast_simple_command *simple_command)
{
    if (simple_command == NULL)
        return;
    printf("command ");
    for (size_t i = 0; i < simple_command->assignments->size; i++)
    {
        char *assignment = vector_get_pointer(simple_command->assignments, i);
        printf("assignment \"%s\" ", assignment);
    }
    for (size_t i = 0; i < simple_command->arguments->size; i++)
    {
        m_string arg = vector_get_pointer(simple_command->arguments, i);
        printf("\"%s\" ", m_string_get_raw(arg));
    }
}

static void print_redirection(struct redirection *redirection)
{
    char *mid = "unknown";
    switch (redirection->type)
    {
    case REDIR_RIGHT:
        mid = ">";
        break;
    case REDIR_LEFT:
        mid = "<";
        break;
    case REDIR_RIGHT_APPEND:
        mid = ">>";
        break;
    case REDIR_RIGHT_FD:
        mid = ">&";
        break;
    case REDIR_HEREDOC:
        mid = "<<";
        break;
    case REDIR_RIGHT_OVERWRITE:
        mid = ">|";
        break;
    case REDIR_LEFT_FD:
        mid = "<&";
        break;
    case REDIR_RIGHT_LEFT:
        mid = "<>";
        break;
    case REDIR_HEREDOC_MINUS:
        mid = "<<-";
        break;
    }
    printf(" redirection \"%d%s%s\"", redirection->ionumber, mid,
           m_string_get_raw(redirection->right_string));
}

static void print_command(struct ast_command *command)
{
    if (command == NULL)
        return;

    switch (command->type)
    {
    case COMMAND_SIMPLE:
        print_simple_command(command->command.simple_command);
        break;
    case COMMAND_IF:
        printf("if: todo ");
        break;
    case COMMAND_WHILE:
        printf("while: todo ");
        break;
    case COMMAND_UNTIL:
        printf("until: todo ");
        break;
    case COMMAND_FOR:
        printf("for: todo ");
        break;
    case COMMAND_CASE:
        printf("case: todo ");
        break;
    case COMMAND_FUNCDEC:
        printf("funcdec: todo ");
        break;
    case COMMAND_LIST:
        printf("{ list } ");
        break;
    case COMMAND_SUBSHELL:
        printf("( subshell ) ");
        break;
    }

    if (command->redirections)
    {
        for (size_t i = 0; i < command->redirections->size; i++)
        {
            print_redirection(vector_get_pointer(command->redirections, i));
        }
    }
}

static void print_pipeline(struct ast_pipeline *pipeline)
{
    if (pipeline == NULL)
        return;
    if (pipeline->negated)
        printf("! ");
    for (size_t i = 0; i < pipeline->commands->size; i++)
    {
        struct ast_command *command = vector_get_pointer(pipeline->commands, i);
        print_command(command);
        if (i != pipeline->commands->size - 1)
            printf(" | ");
    }
}

static void print_and_or(struct ast_and_or *and_or)
{
    if (and_or == NULL)
        return;

    for (size_t i = 0; i < and_or->pipelines->size; i++)
    {
        struct ast_pipeline *pipeline =
            vector_get_pointer(and_or->pipelines, i);
        print_pipeline(pipeline);
        if (and_or->connection_types && i < and_or->connection_types->size)
        {
            enum and_or_type *type = vector_get(and_or->connection_types, i);
            printf(*type == AND_OR_AND ? " && " : " || ");
        }
    }
    printf(and_or->background ? " & " : " ; ");
}

void print_ast(struct ast *ast)
{
    if (ast == NULL || ast->list == NULL || ast->list->list == NULL)
        return;
    for (size_t i = 0; i < ast->list->list->size; i++)
    {
        struct ast_and_or *and_or = vector_get_pointer(ast->list->list, i);
        print_and_or(and_or);
    }
    printf("\n");
}
#endif /* ! PRINT_AST */
