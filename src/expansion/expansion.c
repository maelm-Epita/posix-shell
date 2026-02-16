#define _POSIX_C_SOURCE 200809L

#include "expansion.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "shell/shell.h"
#include "token/string_metadata.h"
#include "utils/hash_map/hash_map.h"
#include "utils/vector/vector_extra.h"
#include "variables/variables.h"

static int is_separator(char c)
{
    return !(isalnum(c) || c == '_');
}

static int is_special_var(char c)
{
    return c == '@' || c == '*' || c == '?' || c == '$' || isdigit(c)
        || c == '#';
}

static char *shell_get_variable(struct shell_state *state, char *key)
{
    if (!key)
        return NULL;

    if (strcmp(key, "?") == 0)
    {
        char buf[12];
        sprintf(buf, "%d", state->last_exit_code);
        return strdup(buf);
    }
    if (strcmp(key, "$") == 0)
    {
        char buf[12];
        sprintf(buf, "%d", state->shell_pid);
        return strdup(buf);
    }
    if (strcmp(key, "#") == 0)
    {
        char buf[12];
        sprintf(buf, "%zu", state->args->size);
        return strdup(buf);
    }
    if (strcmp(key, "@") == 0 || strcmp(key, "*") == 0)
    {
        return ifs_join(state->args, state, 0);
    }
    if (strcmp(key, "UID") == 0)
    {
        char buf[12];
        sprintf(buf, "%d", getuid());
        return strdup(buf);
    }
    if (strcmp(key, "RANDOM") == 0)
    {
        char buf[12];
        sprintf(buf, "%d", rand());
        return strdup(buf);
    }

    char *endptr;
    long idx = strtol(key, &endptr, 10);
    if (*endptr == '\0' && idx >= 0)
    {
        if (idx == 0)
            return strdup(state->name);

        if (idx - 1 < (long)state->args->size)
        {
            char *val = vector_get_pointer(state->args, idx - 1);
            return strdup(val);
        }
        return strdup("");
    }

    struct variable *v = hash_map_get(state->variables, key);
    if (v)
        return strdup(v->value);

    return strdup("");
}

static int find_closing_paren(vector *acc, char *str, int i)
{
    int in_paren = 1;
    while (in_paren > 0)
    {
        if (str[i] == 0)
            return -1;
        if (str[i] == ')')
            in_paren--;
        if (str[i] == '(')
            in_paren++;
        if (vector_append(acc, str + i) == -1)
            return -1;
        i++;
    }
    i--;
    return i;
}

static int find_name_end_after(char *str, char c0, int braced, size_t *name_end)
{
    int after;
    if (is_special_var(c0))
    {
        *name_end += 1;
        after = braced ? (*name_end) + 1 : *name_end;
    }
    else
    {
        while (str[*name_end] != '\0')
        {
            unsigned char cc = (unsigned char)str[*name_end];
            if (is_separator(cc) || (braced && cc == '}'))
            {
                break;
            }
            *name_end += 1;
        }

        if (braced)
        {
            if (str[*name_end] != '}')
                return -1;
            after = *name_end + 1;
        }
        else
        {
            after = *name_end;
        }
    }
    return after;
}

static char *get_var_value(char *str, size_t name_end, size_t name_start,
                           struct shell_state *state)
{
    size_t nlen = name_end - name_start;
    char *name = calloc(nlen + 1, sizeof(char));
    if (name == NULL)
        return NULL;
    memcpy(name, str + name_start, nlen);

    char *val = shell_get_variable(state, name);
    free(name);
    return val;
}

struct string_segment *expand_variables(struct string_segment *segment,
                                        struct shell_state *state)
{
    vector *acc = vector_init(sizeof(char));
    if (acc == NULL)
        return NULL;
    char *str = segment->str;

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] != '$' || str[i + 1] == 0)
        {
            vector_append(acc, str + i);
            continue;
        }

        if (str[i + 1] == '(')
        {
            vector_append_n(acc, str + i, 2);
            i = find_closing_paren(acc, str, i + 2);
            if (i == -1)
                goto err_cleanup;
            continue;
        }

        size_t name_start = str[i + 1] == '{' ? i + 2 : i + 1;
        int braced = str[i + 1] == '{';

        unsigned char c0 = (unsigned char)str[name_start];
        if (is_separator(c0) && !is_special_var(c0))
        {
            vector_append(acc, str + i);
            continue;
        }

        size_t name_end = name_start;
        int after = find_name_end_after(str, c0, braced, &name_end);
        if (after == -1)
            goto err_cleanup;

        char *val = get_var_value(str, name_end, name_start, state);
        if (val == NULL)
            goto err_cleanup;

        vector_append_n(acc, val, strlen(val));
        free(val);

        i = after - 1;
    }

    struct string_segment *out_seg =
        create_segment(acc->data, acc->size, segment->quote, 0);
    vector_free(acc);
    return out_seg;

err_cleanup:
    vector_free(acc);
    return NULL;
}

static int do_subshell_append_result(char *str, size_t *index, vector *acc,
                                     struct shell_state *state)
{
    *index += 2;
    size_t len = 0;
    int in_paren = 1;
    while (in_paren > 0)
    {
        if (str[*index + len] == 0)
            return -1;
        if (str[*index + len] == ')')
            in_paren--;
        if (str[*index + len] == '(')
            in_paren++;
        len++;
    }
    len--;
    char *in = calloc(len + 1, sizeof(char));
    memcpy(in, str + *index, len);
    *index += len + 1;
    struct io_source src = { .source_type = IO_CHAR_STREAM,
                             .source_stream.char_stream.stream = in,
                             .source_stream.char_stream.size = len };
    int p[2] = { 0 };
    if (pipe(p) == -1)
        return -1;
    if (exec_subshell_stdout_redirected(state, src, p) == -1)
        return -1;
    char *buf[4096];
    size_t bytes = 0;
    while ((bytes = read(p[0], buf, 4096)) > 0)
    {
        if (vector_append_n(acc, buf, bytes) == -1)
            return -1;
    }
    close(p[0]);
    free(in);
    char *last;
    while (acc->size > 0 && *(last = vector_get(acc, acc->size - 1)) == '\n')
    {
        if (vector_pop(acc) == -1)
            return -1;
    }
    return 0;
}

struct string_segment *command_substitution(struct string_segment *seg,
                                            struct shell_state *state)
{
    char *str = seg->str;
    char term = 0;
    struct string_segment *res = malloc(sizeof(struct string_segment));
    res->escaped = seg->escaped;
    res->quote = seg->quote;
    vector *acc = vector_init(sizeof(char));
    if (acc == NULL)
        return NULL;
    size_t index = 0;
    while (str[index] != 0)
    {
        if (str[index] == '$' && str[index + 1] == '(')
        {
            if (do_subshell_append_result(str, &index, acc, state) == -1)
                goto err_cleanup;
            continue;
        }
        if (vector_append(acc, str + index) == -1)
            goto err_cleanup;
        index++;
    }

    if (vector_append(acc, &term) == -1)
        goto err_cleanup;
    res->str = strdup(acc->data);
    vector_free(acc);
    return res;
err_cleanup:
    free_segment(res);
    vector_free(acc);
    return NULL;
}

char *quote_removal(m_string str)
{
    vector *acc = vector_init(sizeof(char));
    if (acc == NULL)
        return NULL;
    for (size_t i = 0; i < str->size; i++)
    {
        struct string_segment *seg = vector_get_pointer(str, i);
        if (vector_append_n(acc, seg->str, strlen(seg->str)) != 0)
            goto err_cleanup;
    }
    char *res = malloc(sizeof(char) * (acc->size + 1));
    memcpy(res, acc->data, acc->size);
    res[acc->size] = 0;
    vector_free(acc);
    return res;

err_cleanup:
    vector_free(acc);
    return NULL;
}

static int is_ifs_char(char c, const char *ifs)
{
    if (!ifs)
        return 0;
    return strchr(ifs, c) != NULL;
}

static int is_ifs_whitespace(char c, const char *ifs)
{
    return is_ifs_char(c, ifs) && (c == ' ' || c == '\t' || c == '\n');
}

static void finish_word_and_reset_buf(vector **buf, vector *res)
{
    char term = 0;
    vector_append(*buf, &term);
    char *word = strdup((*buf)->data);
    vector_append(res, &word);

    vector_free(*buf);
    *buf = vector_init(sizeof(char));
}

static vector *perform_field_splitting(m_string expanded,
                                       struct shell_state *state)
{
    vector *res = vector_init(sizeof(char *));
    vector *buf = vector_init(sizeof(char));

    char *ifs_val = " \t\n";
    struct variable *v = hash_map_get(state->variables, "IFS");
    if (v)
        ifs_val = v->value;

    int no_split = (v && v->value && v->value[0] == '\0');

    for (size_t i = 0; i < expanded->size; i++)
    {
        struct string_segment *seg = vector_get_pointer(expanded, i);
        char *s = seg->str;

        if (seg->quote != Q_NOT_QUOTED || seg->escaped || no_split)
        {
            if (vector_append_n(buf, s, strlen(s)) == -1)
                goto err;
        }
        else
        {
            while (*s)
            {
                if (is_ifs_char(*s, ifs_val))
                {
                    if (is_ifs_whitespace(*s, ifs_val))
                    {
                        if (buf->size > 0)
                        {
                            finish_word_and_reset_buf(&buf, res);
                        }
                    }
                    else
                    {
                        finish_word_and_reset_buf(&buf, res);
                    }
                }
                else
                {
                    vector_append(buf, s);
                }
                s++;
            }
        }
    }

    if (buf->size > 0)
    {
        char term = 0;
        vector_append(buf, &term);
        char *word = strdup(buf->data);
        vector_append(res, &word);
    }

    vector_free(buf);
    return res;

err:
    vector_free(buf);
    if (res)
    {
        for (size_t k = 0; k < res->size; k++)
            free(vector_get_pointer(res, k));
        vector_free(res);
    }
    return NULL;
}

static int do_unsplit_args_check(vector *out, m_string str,
                                 struct shell_state *state)
{
    if (str->size == 1)
    {
        struct string_segment *seg = vector_get_pointer(str, 0);
        if (!seg->escaped && seg->quote != Q_SINGLE_QUOTE
            && strcmp(seg->str, "$@") == 0)
        {
            for (size_t i = 0; i < state->args->size; i++)
            {
                char *arg = vector_get_pointer(state->args, i);
                char *dup = strdup(arg);
                vector_append(out, &dup);
            }
            return 1;
        }
    }
    return 0;
}

vector *expand(m_string str, struct shell_state *state, int *is_empty_command)
{
    vector *out = vector_init(sizeof(char *));
    m_string expanded = vector_init(sizeof(struct string_segment *));

    if (do_unsplit_args_check(out, str, state))
    {
        free_m_string(expanded);
        return out;
    }

    for (size_t i = 0; i < str->size; i++)
    {
        struct string_segment *seg = vector_get_pointer(str, i);
        if (seg->escaped || seg->quote == Q_SINGLE_QUOTE)
        {
            struct string_segment *copy = create_segment(
                seg->str, strlen(seg->str), seg->quote, seg->escaped);
            if (copy == NULL || vector_append(expanded, &copy) == -1)
                goto err_cleanup;
            continue;
        }
        struct string_segment *var_expanded = expand_variables(seg, state);
        struct string_segment *command_sub =
            command_substitution(var_expanded, state);
        if (vector_append(expanded, &command_sub) == -1)
            goto err_cleanup;
        free_segment(var_expanded);
    }
    if (is_empty_command != NULL)
    {
        *is_empty_command = 0;
        if (expanded->size == 0)
            *is_empty_command = 1;
        if (expanded->size == 1)
        {
            struct string_segment *seg = vector_get_pointer(expanded, 0);
            if (seg->escaped == 0 && seg->quote == Q_NOT_QUOTED
                && seg->str[0] == 0)
                *is_empty_command = 1;
        }
    }

    vector *split_results = perform_field_splitting(expanded, state);
    if (split_results == NULL)
        goto err_cleanup;

    vector_free(out);
    out = split_results;
    free_m_string(expanded);
    return out;

err_cleanup:
    free_m_string(expanded);
    vector_free_pointer(out);
    return NULL;
}

vector *expand_vector(vector *strings, struct shell_state *state,
                      int *is_empty_command)
{
    int vector_empty = 1;
    vector *res = vector_init(sizeof(char *));
    if (res == NULL)
        return NULL;
    for (size_t i = 0; i < strings->size; i++)
    {
        m_string ms_arg = vector_get_pointer(strings, i);
        vector *expanded_words = expand(ms_arg, state, is_empty_command);
        if (is_empty_command != NULL)
        {
            if (*is_empty_command == 0)
            {
                vector_empty = 0;
            }
        }
        if (vector_append_n(res, expanded_words->data, expanded_words->size)
            != 0)
            return NULL;
        vector_free(expanded_words);
    }
    if (is_empty_command != NULL)
    {
        *is_empty_command = vector_empty;
    }
    return res;
}

char *ifs_join(vector *fields, struct shell_state *state, size_t offset)
{
    vector *acc = vector_init(sizeof(char));
    char sep;
    char term = 0;
    if (state == NULL)
    {
        sep = DEFAULT_IFS[0];
    }
    else
    {
        char *ifs = shell_get_variable(state, "IFS");
        sep = ifs[0] == 0 ? DEFAULT_IFS[0] : ifs[0];
        free(ifs);
    }
    for (size_t i = offset; i < fields->size; i++)
    {
        char *field = vector_get_pointer(fields, i);
        if (vector_append_n(acc, field, strlen(field)) == -1)
            goto err_cleanup;
        if (i < fields->size - 1)
        {
            if (vector_append(acc, &sep) == -1)
                goto err_cleanup;
        }
    }
    if (vector_append(acc, &term) == -1)
        goto err_cleanup;
    char *res = strdup(acc->data);
    vector_free(acc);
    return res;

err_cleanup:
    vector_free(acc);
    return NULL;
}
