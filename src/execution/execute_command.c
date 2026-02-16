#include "execution_private.h"

int execute_command(struct ast_command *ast_command, struct shell_state *state)
{
    if (ast_command->type != COMMAND_SIMPLE)
    {
        if (execute_redirections(ast_command->redirections, state) != 0)
            return RET_SHELL_INTERNAL;
    }
    switch (ast_command->type)
    {
    case COMMAND_FUNCDEC:
        return execute_funcdec(ast_command->command.funcdec, state);
    case COMMAND_LIST:
        return execute_list(ast_command->command.list, state);
    case COMMAND_SIMPLE:
        return execute_simple_command(ast_command->command.simple_command,
                                      ast_command->redirections, state);
    case COMMAND_IF:
        return execute_if(ast_command->command.if_statement, state);
    case COMMAND_CASE:
        return execute_case(ast_command->command.case_statement, state);
    case COMMAND_FOR:
        return execute_for(ast_command->command.for_statement, state);
    case COMMAND_WHILE:
        return execute_while(ast_command->command.while_statement, state);
    case COMMAND_UNTIL:
        return execute_until(ast_command->command.until_statement, state);
    case COMMAND_SUBSHELL:
        return execute_subshell(ast_command->command.subshell, state);
    }
    // Cannot happen
    return 1;
}
