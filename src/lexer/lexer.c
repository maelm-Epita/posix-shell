#include "lexer.h"

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "expansion/expansion.h"
#include "io/io.h"
#include "token/string_metadata.h"
#include "utils/queue/queue.h"
#include "utils/vector/vector.h"
#include "utils/vector/vector_extra.h"

static queue *lexer_queue = NULL;
#define SUCCESS_OR_ERROR *out == NULL ? LEXER_INTERNAL_ERR : LEXER_SUCCESS
#define MAX_EAT_COUNT 64

int is_separator(char c)
{
    return c == ' ' || c == '\n' || c == '(' || c == ')' || c == '|' || c == ';'
        || c == '&' || c == '>' || c == '<';
}

static int is_skipped_whitespace(char c)
{
    return c == ' ' || c == '\t';
}

static void skip_whitespace_comments(char **peek, size_t *bytes)
{
    *bytes = io_peek(peek, 1);
    while (*bytes == 1 && is_skipped_whitespace(**peek))
    {
        io_consume(1);
        *bytes = io_peek(peek, 1);
    }

    if (*bytes == 1 && **peek == '#')
    {
        while (*bytes == 1 && **peek != '\n')
        {
            io_consume(1);
            *bytes = io_peek(peek, 1);
        }
    }
}

static int lex_single_char_tokens(char *peek, size_t bytes, struct token **out)
{
    if (bytes == 0)
    {
        *out = create_token_string(T_EOF, NULL, 0);
        io_consume(1);
        return 1;
    }

    if (*peek == '\n')
    {
        *out = create_token_string(T_NL, peek, 1);
        io_consume(1);
        return 1;
    }
    else if (*peek == '(')
    {
        *out = create_token_string(T_LEFT_PAREN, peek, 1);
        io_consume(1);
        return 1;
    }
    else if (*peek == ')')
    {
        *out = create_token_string(T_RIGHT_PAREN, peek, 1);
        io_consume(1);
        return 1;
    }
    return 0;
}

static int lex_pipe(char **peek, size_t *bytes, struct token **out)
{
    if (**peek == '|') // PIPE | DOUBLE PIPE
    {
        *bytes = io_peek(peek, 2);
        if (*bytes == 2 && memcmp(*peek, "||", 2) == 0)
        {
            *out = create_token_string(T_DOUBLE_PIPE, *peek, 2);
            io_consume(2);
        }
        else
        {
            *out = create_token_string(T_PIPE, *peek, 1);
            io_consume(1);
        }
        return 1;
    }
    return 0;
}

static int lex_semicol(char **peek, size_t *bytes, struct token **out)
{
    if (**peek == ';') // SEMICOL | DOUBLE SEMICOL
    {
        *bytes = io_peek(peek, 2);
        if (*bytes == 2 && memcmp(*peek, ";;", 2) == 0)
        {
            *out = create_token_string(T_DOUBLE_SEMICOL, *peek, 2);
            io_consume(2);
        }
        else
        {
            *out = create_token_string(T_SEMICOL, *peek, 1);
            io_consume(1);
        }
        return 1;
    }
    return 0;
}

static int lex_and(char **peek, size_t *bytes, struct token **out)
{
    if (**peek == '&') // AND | DOUBLE AND
    {
        *bytes = io_peek(peek, 2);
        if (*bytes == 2 && memcmp(*peek, "&&", 2) == 0)
        {
            *out = create_token_string(T_DOUBLE_AND, *peek, 2);
            io_consume(2);
        }
        else
        {
            *out = create_token_string(T_AND, *peek, 1);
            io_consume(1);
        }
        return 1;
    }
    return 0;
}

static int lex_redir(char **peek, size_t *bytes, struct token **out)
{
    if (**peek == '>') // GT | DOUBLE GT | GT AND | GT PIPE
    {
        *bytes = io_peek(peek, 2);
        if (*bytes == 2 && memcmp(*peek, ">>", 2) == 0)
        {
            *out = create_token_string(T_DOUBLE_GT, *peek, 2);
            io_consume(2);
        }
        else if (*bytes == 2 && memcmp(*peek, ">&", 2) == 0)
        {
            *out = create_token_string(T_GT_AND, *peek, 2);
            io_consume(2);
        }
        else if (*bytes == 2 && memcmp(*peek, ">|", 2) == 0)
        {
            *out = create_token_string(T_GT_PIPE, *peek, 2);
            io_consume(2);
        }
        else
        {
            *out = create_token_string(T_GT, *peek, 1);
            io_consume(1);
        }
        return 1;
    }
    else if (**peek == '<') // LT | DOUBLE LT | LT AND | LT GT | DOUBLE LT MINUS
    {
        *bytes = io_peek(peek, 3);
        if (*bytes == 3 && memcmp(*peek, "<<-", 3) == 0)
        {
            *out = create_token_string(T_DOUBLE_LT_MINUS, *peek, 3);
            io_consume(3);
        }
        else
        {
            *bytes = io_peek(peek, 2);
            if (*bytes == 2 && memcmp(*peek, "<<", 2) == 0)
            {
                *out = create_token_string(T_DOUBLE_LT, *peek, 2);
                io_consume(2);
            }
            else if (*bytes == 2 && memcmp(*peek, "<&", 2) == 0)
            {
                *out = create_token_string(T_LT_AND, *peek, 2);
                io_consume(2);
            }
            else if (*bytes == 2 && memcmp(*peek, "<>", 2) == 0)
            {
                *out = create_token_string(T_LT_GT, *peek, 2);
                io_consume(2);
            }
            else
            {
                *out = create_token_string(T_LT, *peek, 1);
                io_consume(1);
            }
        }
        return 1;
    }
    return 0;
}

static int lex_word_init(struct lex_word_state *st)
{
    st->escaping = 0;
    st->escaped_last = 0;
    st->in_paren = 0;
    st->in_sub_brack = 0;
    st->type = Q_NOT_QUOTED;
    st->segments = vector_init(sizeof(struct string_segment *));
    st->acc = vector_init(sizeof(char));

    if (st->segments == NULL || st->acc == NULL)
        return LEXER_INTERNAL_ERR;
    return LEXER_SUCCESS;
}

static int save_word_segment(vector *acc, vector *segments,
                             enum quote_type type, int escaped_last)
{
    struct string_segment *seg =
        create_segment(acc->data, acc->size, type, escaped_last);
    if (seg == NULL)
        return LEXER_INTERNAL_ERR;
    if (vector_append(segments, &seg) == -1)
        return LEXER_INTERNAL_ERR;
    vector_clear(acc);
    return LEXER_SUCCESS;
}

static int lex_word_escaping(char **peek, int *skip, int *escaping)
{
    if (**peek == '\n')
    {
        *skip = 1;
        *escaping = 0;
    }
    return 0;
}

static int lex_word_in_paren(char **peek, int *in_paren)
{
    if (**peek == '(')
        (*in_paren)++;
    if (**peek == ')')
        (*in_paren)--;
    return 0;
}

static int lex_word_handle_quotes(char **peek, struct lex_word_state *st,
                                  int *skip)
{
    if ((**peek == '"') && !st->in_paren && !st->escaping
        && st->type != Q_SINGLE_QUOTE)
    {
        if (st->type != Q_NOT_QUOTED || st->acc->size > 0)
        {
            if (save_word_segment(st->acc, st->segments, st->type,
                                  st->escaped_last)
                != LEXER_SUCCESS)
                return LEXER_INTERNAL_ERR;
        }
        st->type = st->type == Q_NOT_QUOTED ? Q_DOUBLE_QUOTE : Q_NOT_QUOTED;
        *skip = 1;
    }

    if ((**peek == '\'') && !st->in_paren && !st->escaping
        && st->type != Q_DOUBLE_QUOTE)
    {
        if (st->type != Q_NOT_QUOTED || st->acc->size > 0)
        {
            if (save_word_segment(st->acc, st->segments, st->type,
                                  st->escaped_last)
                != LEXER_SUCCESS)
                return LEXER_INTERNAL_ERR;
        }
        st->type = st->type == Q_NOT_QUOTED ? Q_SINGLE_QUOTE : Q_NOT_QUOTED;
        *skip = 1;
    }

    return LEXER_SUCCESS;
}

static int lex_word_handle_escape(char **peek, size_t *bytes,
                                  struct lex_word_state *st, int *skip)
{
    if (**peek != '\\' || st->in_paren || st->escaping
        || st->type == Q_SINGLE_QUOTE)
        return LEXER_SUCCESS;

    *bytes = io_peek(peek, 2);
    if (st->type != Q_DOUBLE_QUOTE
        || (*bytes == 2
            && ((*peek)[1] == '$' || (*peek)[1] == '`' || (*peek)[1] == '"'
                || (*peek)[1] == '\\')))
    {
        if (!st->escaped_last && (*peek)[1] != '\n')
        {
            if (st->acc->size > 0)
            {
                if (save_word_segment(st->acc, st->segments, st->type, 0)
                    != LEXER_SUCCESS)
                    return LEXER_INTERNAL_ERR;
            }
        }

        if (*bytes == 2 && (*peek)[1] == '\n')
        {
            io_consume(1);
            *bytes = io_peek(peek, 1);
            io_consume(1);
            *bytes = io_peek(peek, 1);
        }
        else
        {
            st->escaping = 1;
            *skip = 1;
        }
    }

    return LEXER_SUCCESS;
}

static int lex_word_handle_dollar(char **peek, size_t *bytes,
                                  struct lex_word_state *st)
{
    if (**peek == '$' && !st->in_paren && st->type != Q_SINGLE_QUOTE
        && !st->escaping)
    {
        *bytes = io_peek(peek, 2);
        if (*bytes == 2 && (*peek)[1] == '(')
        {
            st->in_paren = 1;
            if (vector_append(st->acc, *peek) != 0)
                return LEXER_INTERNAL_ERR;
            io_consume(1);
            *bytes = io_peek(peek, 1);
        }
        else if (*bytes == 2 && (*peek)[1] == '{')
            st->in_sub_brack += 1;
    }
    return LEXER_SUCCESS;
}

static int lex_word_append_char(char **peek, struct lex_word_state *st,
                                int skip)
{
    if (!skip)
    {
        if (vector_append(st->acc, *peek) != 0)
            return LEXER_INTERNAL_ERR;
        if (st->escaping)
        {
            st->escaping = 0;
            st->escaped_last = 1;
        }
    }
    return LEXER_SUCCESS;
}
static int lex_word_finalize(struct lex_word_state *st, struct token **out)
{
    if (st->in_paren)
    {
        fprintf(stderr,
                "Syntax error: found EOF while looking for matching "
                "parenthesis.\n");
        return LEXER_SYNTAX_ERR;
    }

    if (st->in_sub_brack)
    {
        fprintf(stderr,
                "Syntax error: found EOF while looking for matching "
                "bracket.\n");
        return LEXER_SYNTAX_ERR;
    }

    if (st->type != Q_NOT_QUOTED)
    {
        fprintf(stderr,
                "Syntax error: found EOF while looking for matching quote.\n");
        return LEXER_SYNTAX_ERR;
    }

    if (st->acc->size > 0 || st->segments->size == 0)
    {
        if (save_word_segment(st->acc, st->segments, st->type, st->escaped_last)
            != LEXER_SUCCESS)
            return LEXER_INTERNAL_ERR;
    }

    union token_value val = { .string = st->segments };
    *out = create_token(T_WORD, val);

    vector_free(st->acc);
    return LEXER_SUCCESS;
}

static int lex_word(char **peek, size_t *bytes, struct token **out)
{
    struct lex_word_state st;
    if (lex_word_init(&st) != LEXER_SUCCESS)
        return LEXER_INTERNAL_ERR;

    while (*bytes > 0
           && (st.in_paren || st.escaping || st.type != Q_NOT_QUOTED
               || !is_separator(**peek)))
    {
        int skip = 0;

        if (st.escaping)
            lex_word_escaping(peek, &skip, &st.escaping);
        else if (st.in_paren)
            lex_word_in_paren(peek, &st.in_paren);

        if (lex_word_handle_quotes(peek, &st, &skip) != LEXER_SUCCESS
            || lex_word_handle_escape(peek, bytes, &st, &skip) != LEXER_SUCCESS
            || lex_word_handle_dollar(peek, bytes, &st) != LEXER_SUCCESS)
            goto err_cleanup;

        if (**peek == '}' && !st.escaping && st.type != Q_SINGLE_QUOTE
            && st.in_sub_brack)
            st.in_sub_brack--;

        if (st.acc->size > 0 && st.escaped_last && !st.escaping)
        {
            if (save_word_segment(st.acc, st.segments, st.type, 1)
                != LEXER_SUCCESS)
                goto err_cleanup;
        }

        st.escaped_last = 0;

        if (lex_word_append_char(peek, &st, skip) != LEXER_SUCCESS)
            goto err_cleanup;

        io_consume(1);
        *bytes = io_peek(peek, 1);
    }

    {
        int ret = lex_word_finalize(&st, out);
        if (ret != LEXER_SUCCESS)
            goto err_cleanup;
        return ret;
    }

err_cleanup:
    vector_free(st.acc);
    free_m_string(st.segments);
    return LEXER_SYNTAX_ERR;
}

static int lex_other(char **peek, size_t *bytes, struct token **out)
{
    *bytes = io_peek(peek, 2);
    if (*bytes == 2 && isdigit(**peek)
        && (*(*peek + 1) == '<' || *(*peek + 1) == '>')) // IONUMBER
    {
        *out = create_token_string(T_IONUMBER, *peek, 1);
        io_consume(1);
        return SUCCESS_OR_ERROR;
    }
    return lex_word(peek, bytes, out);
}

enum lexer_state lex_next_token(struct token **out)
{
    char *peek;
    size_t bytes;

    skip_whitespace_comments(&peek, &bytes);

    if (lex_single_char_tokens(peek, bytes, out))
        return SUCCESS_OR_ERROR;
    else if (lex_pipe(&peek, &bytes, out))
        return SUCCESS_OR_ERROR;
    else if (lex_semicol(&peek, &bytes, out))
        return SUCCESS_OR_ERROR;
    else if (lex_and(&peek, &bytes, out))
        return SUCCESS_OR_ERROR;
    else if (lex_redir(&peek, &bytes, out))
        return SUCCESS_OR_ERROR;
    else // WORD/ASSIGNMENT_WORD or KEYWORD or IONUMBER
        return lex_other(&peek, &bytes, out);
    return SUCCESS_OR_ERROR;
}

queue *lexer_queue_get(void)
{
    if (lexer_queue == NULL)
        lexer_queue = queue_init();
    return lexer_queue;
}

enum lexer_state lexer_lookahead(struct token **out, size_t count)
{
    queue *lexer_queue = lexer_queue_get();
    queue *copy_queue = queue_init();
    if (copy_queue == NULL)
        return LEXER_INTERNAL_ERR;

    struct token *tok;
    enum lexer_state s;

    if ((s = lexer_peek(&tok)) != LEXER_SUCCESS)
        return s;
    while (count > 0)
    {
        if (queue_push_front(copy_queue, tok) != 0)
            return LEXER_INTERNAL_ERR;
        queue_pop(lexer_queue);
        if ((s = lexer_peek(&tok)) != LEXER_SUCCESS)
        {
            while (!queue_isempty(copy_queue))
            {
                tok = queue_pop(copy_queue);
                free_token(tok);
            }
            queue_free(copy_queue);
            return s;
        }
        count--;
    }

    *out = tok;

    while (!queue_isempty(copy_queue))
    {
        tok = queue_pop(copy_queue);
        if (queue_push_front(lexer_queue, tok) != 0)
            return LEXER_INTERNAL_ERR;
    }

    queue_free(copy_queue);
    return LEXER_SUCCESS;
}

enum lexer_state lexer_peek(struct token **out)
{
    queue *lexer_queue = lexer_queue_get();
    enum lexer_state s = LEXER_SUCCESS;
    if (queue_isempty(lexer_queue))
    {
        struct token *tok = NULL;
        s = lex_next_token(&tok);
        if (tok == NULL)
            return s;
        if (queue_push(lexer_queue, tok) == -1)
            return LEXER_INTERNAL_ERR;
    }
    *out = queue_peek(lexer_queue);
    return s;
}

static void lexer_wrong_lookahead(char *rule, char *expected, struct token *got)
{
    if (token_type_has_string_value(got->type))
    {
        char *str = m_string_get_raw(got->value.string);
        fprintf(stderr,
                "Syntax error in %s rule. Expected '%s' but got '%s(%s)'\n",
                rule, expected, token_type_to_string(got->type), str);
    }
    else if (token_type_has_int_value(got->type))
    {
        fprintf(stderr,
                "Syntax error in %s rule. Expected '%s' but got '%s(%d)'\n",
                rule, expected, token_type_to_string(got->type),
                got->value.int_value);
    }
    else
    {
        fprintf(stderr, "Syntax error in %s rule. Expected '%s' but got '%s'\n",
                rule, expected, token_type_to_string(got->type));
    }
}

int lexer_eat(char *rule, enum token_type tok_type)
{
    queue *lexer_queue = lexer_queue_get();
    struct token *top_tok = queue_peek(lexer_queue);
    assert(top_tok != NULL);
    if (!token_matches_type(top_tok, tok_type))
    {
        lexer_wrong_lookahead(rule, token_type_to_string(tok_type), top_tok);
        return -1;
    }
    queue_pop(lexer_queue);
    free_token(top_tok);
    return 0;
}

int lexer_eat_one_of_n(char *rule, size_t n, ...)
{
    assert(n <= MAX_EAT_COUNT);

    enum token_type types[MAX_EAT_COUNT] = { 0 };

    queue *lexer_queue = lexer_queue_get();
    struct token *top_tok = queue_pop(lexer_queue);
    assert(top_tok != NULL);

    va_list token_types;
    va_start(token_types, n);
    for (size_t i = 0; i < n; i++)
    {
        enum token_type type = va_arg(token_types, enum token_type);
        if (token_matches_type(top_tok, type))
        {
            va_end(token_types);
            free_token(top_tok);
            return 0;
        }
        types[i] = type;
    }
    va_end(token_types);

    size_t expected_size = 0;
    char expected[4096] = { 0 };
    for (size_t i = 0; i < n; i++)
    {
        if (i == n - 1)
        {
            expected_size +=
                snprintf(expected + expected_size, 4096 - expected_size, "%s",
                         token_type_to_string(types[i]));
        }
        else
        {
            expected_size +=
                snprintf(expected + expected_size, 4096 - expected_size,
                         "%s or ", token_type_to_string(types[i]));
        }
    }
    lexer_wrong_lookahead(rule, expected, top_tok);
    return -1;
}

enum lexer_state lexer_clear_line(void)
{
    queue *lexer_queue = lexer_queue_get();
    // pop all from the queue until queue empty OR newline found OR eof
    struct token *tok = queue_pop(lexer_queue);
    while (!queue_isempty(lexer_queue) && tok->type != T_NL
           && tok->type != T_EOF)
    {
        free_token(tok);
        tok = queue_pop(lexer_queue);
    }
    // keep consuming tokens until newline or eof
    while (tok == NULL || (tok->type != T_NL && tok->type != T_EOF))
    {
        if (lex_next_token(&tok) != LEXER_SUCCESS)
            return LEXER_INTERNAL_ERR;
    }
    // if found eof then push it back into queue for next parsing
    if (tok->type == T_EOF)
    {
        if (queue_push(lexer_queue, tok) == -1)
            return LEXER_INTERNAL_ERR;
    }
    // if not then found a newline (consuming because not in queue)
    return LEXER_SUCCESS;
}

void lexer_reset(void)
{
    if (lexer_queue == NULL)
        return;
    while (!queue_isempty(lexer_queue))
    {
        struct token *tok = queue_pop(lexer_queue);
        free_token(tok);
    }
    queue_free(lexer_queue);
    lexer_queue = NULL;
}

int lexer_queue_push_front(struct token *tok)
{
    return queue_push_front(lexer_queue, tok);
}
