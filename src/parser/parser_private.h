#ifndef PARSER_PRIVATE_H
#define PARSER_PRIVATE_H

#include "parser.h"
#include "token/token.h"

/**
 * @brief Prints a syntax error message near a token.
 * @param tok Token near which the error happened
 */
void print_syntax_error_near(struct token *tok);

/**
 * @brief Parses the top-level list rule.
 * @param ast AST root being built
 * @return Parser status
 */
enum parser_status parse_list(struct ast *ast, struct shell_state *state);

/**
 * @brief Parses an and_or rule.
 * @param ast_list List node being filled
 * @return Parser status
 */
enum parser_status parse_and_or(struct ast_list *ast_list,
                                struct shell_state *state);

/**
 * @brief Parses a pipeline rule.
 * @param and_or and_or node being filled
 * @return Parser status
 */
enum parser_status parse_pipeline(struct ast_and_or *and_or,
                                  struct shell_state *state);

/**
 * @brief Parses a command rule.
 * @param pipeline Pipeline node being filled
 * @return Parser status
 */
enum parser_status parse_command(struct ast_pipeline *pipeline,
                                 struct shell_state *state);

/**
 * @brief Parses a simple_command rule.
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_simple_command(struct ast_command *command,
                                        struct shell_state *state);

/**
 * @brief Parses a funcdec rule.
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_funcdec(struct ast_command *command,
                                 struct shell_state *state);

/**
 * @brief Parses a shell_command rule.
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_shell_command(struct ast_command *command,
                                       struct shell_state *state);

/**
 * @brief Parses a keyword_command rule (for/while/until/case/if).
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_keyword_command(struct ast_command *command,
                                         struct shell_state *state);

/**
 * @brief Parses a compound_list rule.
 * @param ast_list List node being filled
 * @return Parser status
 */
enum parser_status parse_compound_list(struct ast_list *ast_list,
                                       struct shell_state *state);

/**
 * @brief Parses a for rule.
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_rule_for(struct ast_command *command,
                                  struct shell_state *state);

/**
 * @brief Parses a while rule.
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_rule_while(struct ast_command *command,
                                    struct shell_state *state);

/**
 * @brief Parses an until rule.
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_rule_until(struct ast_command *command,
                                    struct shell_state *state);

/**
 * @brief Parses a case rule.
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_rule_case(struct ast_command *command,
                                   struct shell_state *state);

/**
 * @brief Parses an if rule.
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_rule_if(struct ast_command *command,
                                 struct shell_state *state);

/**
 * @brief Parses a prefix rule (assignments and/or redirections before words).
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_prefix(struct ast_command *command,
                                struct shell_state *state);

/**
 * @brief Parses an element rule (word or redirection).
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_element(struct ast_command *command,
                                 struct shell_state *state);

/**
 * @brief Parses a redirection rule.
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_redirection(struct ast_command *command,
                                     struct shell_state *state);

/**
 * @brief Parses a heredoc redirection and captures its content.
 * @param command Command node being filled
 * @return Parser status
 */
enum parser_status parse_heredoc(struct ast_command *command,
                                 struct shell_state *state);

/**
 * @brief Parses a case_clause rule.
 * @param ast_case Case node being filled
 * @return Parser status
 */
enum parser_status parse_case_clause(struct ast_case *ast_case,
                                     struct shell_state *state);

/**
 * @brief Parses a case_item rule.
 * @param ast_case Case node being filled
 * @return Parser status
 */
enum parser_status parse_case_item(struct ast_case *ast_case,
                                   struct shell_state *state);

/**
 * @brief Parses an else_clause rule (else/elif parts).
 * @param ast_if If node being filled
 * @return Parser status
 */
enum parser_status parse_else_clause(struct ast_if *ast_if,
                                     struct shell_state *state);

#endif /* ! PARSER_PRIVATE_H */
