#ifndef AST_H
#define AST_H

#include "token/string_metadata.h"
#include "utils/vector/vector.h"

/**
 * @brief Redirection operator types.
 */

enum redirection_type
{
    REDIR_RIGHT,
    REDIR_RIGHT_OVERWRITE,
    REDIR_RIGHT_FD,
    REDIR_LEFT,
    REDIR_LEFT_FD,
    REDIR_RIGHT_APPEND,
    REDIR_HEREDOC,
    REDIR_HEREDOC_MINUS,
    REDIR_RIGHT_LEFT,
};

/**
 * @brief Represents a redirection attached to a command.
 * @param ionumber The file descriptor number (e.g. 2> => 2)
 * @param saved_fd Saved file descriptor for later restoration
 * @param right_string The redirection target (file / word / fd depending on
 * type)
 * @param type The redirection type
 */

struct redirection
{
    int ionumber;
    int saved_fd;
    m_string right_string;
    enum redirection_type type;
};

/**
 * @brief Logical connection types between pipelines.
 */

enum and_or_type
{
    AND_OR_AND,
    AND_OR_OR,
};

/**
 * @brief Command kinds stored in the AST.
 */

enum command_type
{
    COMMAND_FUNCDEC,
    COMMAND_LIST,
    COMMAND_SIMPLE,
    COMMAND_FOR,
    COMMAND_WHILE,
    COMMAND_UNTIL,
    COMMAND_CASE,
    COMMAND_SUBSHELL,
    COMMAND_IF,
};

struct ast_list;

/**
 * @brief AST node for if/elif/else.
 * @param if_lists Vector of condition lists (if + elif)
 * @param then_lists Vector of bodies (then + elif + optional else)
 */

struct ast_if
{
    vector *if_lists;
    // if the then_lists vector's size is one more than if_lists, the last list
    // is the "else" list
    vector *then_lists;
};

/**
 * @brief A case branch (patterns + body).
 * @param patterns Vector of patterns (words)
 * @param list Body executed when a pattern matches
 */

struct ast_case_branch
{
    vector *patterns;
    struct ast_list *list;
};

/**
 * @brief AST node for case.
 * @param word Tested word
 * @param branches Vector of branches (ast_case_branch*)
 */

struct ast_case
{
    m_string word;
    vector *branches;
};

/**
 * @brief AST node for until loop.
 * @param until_list Condition list
 * @param do_list Body list
 */

struct ast_until
{
    struct ast_list *until_list;
    struct ast_list *do_list;
};

/**
 * @brief AST node for while loop.
 * @param while_list Condition list
 * @param do_list Body list
 */

struct ast_while
{
    struct ast_list *while_list;
    struct ast_list *do_list;
};

/**
 * @brief AST node for for loop.
 * @param element_identifier Loop variable name
 * @param words Vector of words after "in"
 * @param do_list Body list
 */

struct ast_for
{
    m_string element_identifier;
    vector *words;
    struct ast_list *do_list;
};

/**
 * @brief AST node for a simple command.
 * @param arguments Vector of arguments (usually includes command name)
 * @param assignments Vector of assignments (NAME=VALUE)
 */

struct ast_simple_command
{
    vector *arguments;
    vector *assignments;
};

/**
 * @brief AST node for a function declaration.
 * @param identifier Function name
 * @param command Function body
 */

struct ast_command;

struct ast_funcdec
{
    char *identifier;
    // Can only be list or for, if, etc...
    struct ast_command *command;
};

struct ast_subshell
{
    struct ast_list *list;
};

/**
 * @brief Concrete command payload depending on command type.
 */

union command
{
    struct ast_funcdec *funcdec;
    struct ast_simple_command *simple_command;
    struct ast_list *list;
    struct ast_subshell *subshell;
    struct ast_for *for_statement;
    struct ast_while *while_statement;
    struct ast_until *until_statement;
    struct ast_case *case_statement;
    struct ast_if *if_statement;
};

/**
 * @brief AST node representing a command with optional redirections.
 * @param redirections Vector of redirections (redirection*)
 * @param type The command type
 * @param command The command payload
 */

// either a funcdec
// or a simple command
// or a list
// or a for , if , etc...
struct ast_command
{
    vector *redirections;
    enum command_type type;
    union command command;
};

/**
 * @brief AST node for a pipeline (cmd | cmd | ...).
 * @param commands Vector of commands (ast_command*)
 * @param negated Non-zero if prefixed by '!'
 */

struct ast_pipeline
{
    vector *commands;
    int negated;
};

/**
 * @brief AST node for pipelines connected by && / ||.
 * @param pipelines Vector of pipelines (ast_pipeline*)
 * @param connection_types Vector of connectors (and_or_type)
 * @param background Non-zero if running in background ('&')
 */

struct ast_and_or
{
    vector *pipelines;
    vector *connection_types;
    int background;
};

/**
 * @brief AST node for a list.
 * @param list Vector of elements (often ast_and_or*)
 */

struct ast_list
{
    vector *list;
};

/**
 * @brief AST root.
 * @param list Main list
 */

struct ast
{
    struct ast_list *list;
};

/**
 * @brief Frees an AST and all its children.
 * @param ast The AST to free
 */
void free_ast(struct ast *ast);

/**
 * @brief Prints an AST (debug).
 * @param ast The AST to print
 */
void print_ast(struct ast *ast);

void free_ast_list(struct ast_list *list);

void free_command(struct ast_command *command);

#endif /* ! AST_H */
