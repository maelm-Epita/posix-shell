#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "execution/execution.h"
#include "io/io.h"
#include "parser/parser.h"
#include "token/token.h"
#include "utils/queue/queue.h"

queue *lexer_queue_get(void);

static void lexer_clear_all(void)
{
    queue *q = lexer_queue_get();
    if (!q)
        return;

    while (!queue_isempty(q))
    {
        struct token *tok = queue_pop(q);
        if (tok)
            free_token(tok);
    }
}

static char *read_io_all(void)
{
    char *res = NULL;
    size_t len = 0;

    while (1)
    {
        char *peek = NULL;
        size_t n = io_peek(&peek, PEEK_CAPACITY);
        if (n == 0)
            break;

        char *tmp = realloc(res, len + n + 1);
        if (!tmp)
        {
            free(res);
            return NULL;
        }
        res = tmp;
        memcpy(res + len, peek, n);
        len += n;
        res[len] = 0;
        io_consume(n);
    }

    if (!res)
    {
        res = calloc(1, 1);
        if (!res)
            return NULL;
    }

    return res;
}

static void restore_input(int interactive, char *rest)
{
    if (!interactive)
    {
        struct io_source src = {
            .source_type = IO_CHAR_STREAM,
            .source_stream.char_stream.stream = rest,
            .source_stream.char_stream.size = strlen(rest),
        };
        io_init(src);
        free(rest);
    }
    else
    {
        struct io_source src = {
            .source_type = IO_FILE,
            .source_stream.file = stdin,
        };
        io_init(src);
    }
    lexer_clear_all();
}

static int execute_dot_file(FILE *file, struct shell_state *s)
{
    struct io_source dot_src = {
        .source_type = IO_FILE,
        .source_stream.file = file,
    };
    io_init(dot_src);

    enum parser_status status;
    while (s->running)
    {
        struct ast *ast = NULL;
        status = parse_input(&ast, s);

        if (status == P_ERR)
        {
            free_ast(ast);
            return RET_SHELL_INTERNAL;
        }
        if (status == P_SYNTAX_ERR)
        {
            free_ast(ast);
            return RET_COMMAND_LINE_GRAMMAR;
        }

        if (ast)
            s->last_exit_code = execute_ast(ast, s);

        free_ast(ast);

        if (status == P_EOF)
            break;
    }

    return s->last_exit_code;
}

int builtin_dot(char **args, struct shell_state *s)
{
    if (!args[1])
    {
        fprintf(stderr, ".: filename argument required\n");
        return 1;
    }

    int interactive =
        (s && s->args && s->args->size == 1 && isatty(STDIN_FILENO));

    char *rest = NULL;
    if (!interactive)
    {
        rest = read_io_all();
        if (!rest)
            return RET_SHELL_INTERNAL;
    }

    lexer_clear_all();

    FILE *file = fopen(args[1], "r");
    if (!file)
    {
        perror(args[1]);
        restore_input(interactive, rest);
        return 1;
    }

    int ret = execute_dot_file(file, s);
    fclose(file);
    lexer_clear_all();

    restore_input(interactive, rest);
    return ret;
}
