#ifndef EXECUTION_PRIVATE_H
#define EXECUTION_PRIVATE_H

#include "ast/ast.h"
#include "shell/shell.h"

/**
 * @brief Executes an AST list node.
 * @param ast_list The list to execute
 * @param state Shell state
 * @return Exit status of the list execution
 */

int execute_list(struct ast_list *ast_list, struct shell_state *state);

/**
 * @brief Executes an and_or node (pipelines connected with && / ||).
 * @param ast_and_or The and_or node to execute
 * @param state Shell state
 * @return Exit status of the and_or execution
 */

int execute_and_or(struct ast_and_or *ast_and_or, struct shell_state *state);

/**
 * @brief Executes a pipeline (cmd | cmd | ...).
 * @param ast_pipeline The pipeline to execute
 * @param state Shell state
 * @return Exit status of the pipeline execution
 */

int execute_pipeline(struct ast_pipeline *ast_pipeline,
                     struct shell_state *state);

/**
 * @brief Executes a command node (simple/compound/function/etc.).
 * @param ast_command The command to execute
 * @param state Shell state
 * @return Exit status of the command execution
 */

int execute_command(struct ast_command *ast_command, struct shell_state *state);

/**
 * @brief Executes a while loop node.
 * @param ast_while The while node to execute
 * @param state Shell state
 * @return Exit status of the loop execution
 */

int execute_while(struct ast_while *ast_while, struct shell_state *state);

/**
 * @brief Executes an until loop node.
 * @param ast_until The until node to execute
 * @param state Shell state
 * @return Exit status of the loop execution
 */

int execute_until(struct ast_until *ast_until, struct shell_state *state);

/**
 * @brief Executes a for loop node.
 * @param ast_for The for node to execute
 * @param state Shell state
 * @return Exit status of the loop execution
 */

int execute_for(struct ast_for *ast_for, struct shell_state *state);

/**
 * @brief Executes a case node.
 * @param ast_case The case node to execute
 * @param state Shell state
 * @return Exit status of the case execution
 */

int execute_case(struct ast_case *ast_case, struct shell_state *state);

/**
 * @brief Executes an if/elif/else node.
 * @param ast_if The if node to execute
 * @param state Shell state
 * @return Exit status of the if execution
 */

int execute_if(struct ast_if *ast_if, struct shell_state *state);

/**
 * @brief Executes a simple command node (argv/assignments + redirections).
 * @param ast_simple_command The simple command to execute
 * @param redirections Redirections attached to the command (vector of
 * redirection*)
 * @param state Shell state
 * @return Exit status of the simple command execution
 */

int execute_simple_command(struct ast_simple_command *ast_simple_command,
                           vector *redirections, struct shell_state *state);

/**
 * @brief Executes a function declaration node.
 * @param ast_funcdec The function declaration to execute
 * @param state Shell state
 * @return Exit status of the function declaration execution
 */

int execute_funcdec(struct ast_funcdec *ast_funcdec, struct shell_state *state);

/**
 * @brief Applies redirections for the current command context.
 * @param redirections Vector of redirection*
 * @param state Shell state
 * @return 0 on success, non-zero on error
 */

int execute_redirections(vector *redirections, struct shell_state *state);

/**
 * @brief Restores previously applied redirections.
 * @param redirections Vector of redirection*
 * @return 0 on success, non-zero on error
 */

int revert_redirections(vector *redirections);

/**
 * @brief Executes assignments (NAME=VALUE) in the current context.
 * @param assignments Vector of assignment strings
 * @param state Shell state
 * @return 0 on success, non-zero on error
 */

int execute_assignments(vector *assignments, struct shell_state *state);

int execute_subshell(struct ast_subshell *ast_subshell,
                     struct shell_state *state);

#endif /* ! EXECUTION_PRIVATE_H */
