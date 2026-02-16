#define _POSIX_C_SOURCE 200809L
#include "aliases.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io/io.h"
#include "lexer/lexer.h"
#include "utils/hash_map/hash_map.h"
#include "utils/vector/vector_extra.h"

vector *perform_alias(struct token *tok, struct shell_state *state)
{
    if (tok->type != T_WORD)
        return NULL;
    if (tok->value.string->size != 1)
        return NULL;
    struct string_segment *seg = vector_get_pointer(tok->value.string, 0);
    if (seg->escaped || seg->quote != Q_NOT_QUOTED
        || is_reserved_keyword_str(seg->str))
        return NULL;
    struct alias *a = hash_map_get(state->aliases, seg->str);
    if (a == NULL)
        return NULL;
    vector *tokens = vector_init(sizeof(struct token *));
    if (tokens == NULL)
        return NULL;
    for (size_t i = 0; i < a->tokens->size; i++)
    {
        struct token *tok = vector_get_pointer(a->tokens, i);
        struct token *cpy = token_copy(tok);
        if (vector_append(tokens, &cpy) == -1)
            return NULL;
    }
    return tokens;
}

struct alias *create_alias(char *raw_input)
{
    struct alias *a = malloc(sizeof(struct alias));
    if (a == NULL)
        return NULL;
    vector *tokens = vector_init(sizeof(struct token *));
    if (tokens == NULL)
        return NULL;
    a->raw_input = strdup(raw_input);
    a->tokens = tokens;
    struct io_source oldio = io_get_source();
    struct io_source source = { .source_type = IO_CHAR_STREAM,
                                .source_stream.char_stream = {
                                    .stream = raw_input,
                                    .size = strlen(raw_input) } };
    io_init(source);
    struct token *tok;
    if (lex_next_token(&tok) != LEXER_SUCCESS)
        goto err_cleanup;
    while (tok->type != T_EOF)
    {
        struct token *cpy = token_copy(tok);
        free_token(tok);
        if (vector_append(tokens, &cpy) == -1)
            goto err_cleanup;
        if (lex_next_token(&tok) != LEXER_SUCCESS)
            goto err_cleanup;
    }
    free_token(tok);
    lexer_reset();
    io_init(oldio);
    return a;
err_cleanup:
    free_alias(a);
    free(a);
    return NULL;
}

void free_alias(void *a)
{
    struct alias *alias = a;
    for (size_t i = 0; i < alias->tokens->size; i++)
    {
        struct token *tok = vector_get_pointer(alias->tokens, i);
        free_token(tok);
    }
    vector_free(alias->tokens);
    free(alias->raw_input);
    free(a);
}

void print_alias(struct alias *alias)
{
    printf("%s", alias->raw_input);
}
