#define _POSIX_C_SOURCE 200809L

#include "token.h"

#include <stdlib.h>
#include <string.h>

#include "string_metadata.h"
#include "utils/vector/vector_extra.h"

static int is_reserved_keyword_token_type(enum token_type t)
{
    return t == T_FOR || t == T_DO || t == T_DONE || t == T_WHILE
        || t == T_UNTIL || t == T_CASE || t == T_IN || t == T_ESAC || t == T_IF
        || t == T_THEN || t == T_FI || t == T_ELSE || t == T_ELIF
        || t == T_LEFT_BRACK || t == T_RIGHT_BRACK || t == T_EXCLAMATION;
}

int is_reserved_keyword_str(char *v)
{
    return (strcmp(v, "for") == 0 || strcmp(v, "while") == 0
            || strcmp(v, "until") == 0 || strcmp(v, "case") == 0
            || strcmp(v, "if") == 0 || strcmp(v, "in") == 0
            || strcmp(v, "do") == 0 || strcmp(v, "done") == 0
            || strcmp(v, "esac") == 0 || strcmp(v, "fi") == 0
            || strcmp(v, "then") == 0 || strcmp(v, "else") == 0
            || strcmp(v, "elif") == 0 || strcmp(v, "{") == 0
            || strcmp(v, "}") == 0 || strcmp(v, "!") == 0);
}
int is_reserved_keyword(struct token *tok)
{
    if (tok->type != T_WORD)
        return 0;
    if (tok->value.string->size != 1)
        return 0;
    struct string_segment *seg = vector_get_pointer(tok->value.string, 0);
    if (seg->escaped || seg->quote != Q_NOT_QUOTED)
        return 0;
    return is_reserved_keyword_str(seg->str);
}

int token_matches_type(struct token *tok, enum token_type t)
{
    if (is_reserved_keyword_token_type(t))
    {
        if (is_reserved_keyword(tok))
        {
            struct string_segment *seg =
                vector_get_pointer(tok->value.string, 0);
            return strcmp(seg->str, token_type_to_string(t)) == 0;
        }
        return 0;
    }
    else
        return tok->type == t;
}

int token_type_has_string_value(enum token_type t)
{
    return t == T_WORD || is_reserved_keyword_token_type(t);
}
int token_type_has_int_value(enum token_type t)
{
    return t == T_IONUMBER;
}

static int token_strtoint(char *v, int *out, size_t n)
{
    int res = 0;
    char c;
    for (size_t i = 0; i < n; i++)
    {
        c = *v;
        res *= 10;
        if (c < '0' || c > '9')
            return -1;
        res += c - '0';
        v++;
    }
    *out = res;
    return 0;
}

struct token *create_token_string(enum token_type type, char *val, size_t n)
{
    struct token *tok = malloc(sizeof(struct token));
    if (tok == NULL)
        return NULL;

    tok->type = type;
    if (token_type_has_int_value(type))
    {
        if (token_strtoint(val, &tok->value.int_value, n) == -1)
            return NULL;
    }
    else
    {
        struct string_segment *segment =
            create_segment(val, n, Q_NOT_QUOTED, 0);
        m_string segments = vector_init(sizeof(struct string_segment *));
        if (segment == NULL || segments == NULL)
            return NULL;
        if (vector_append(segments, &segment) == -1)
            return NULL;
        tok->value.string = segments;
    }

    return tok;
}

struct token *create_token(enum token_type type, union token_value val)
{
    struct token *tok = malloc(sizeof(struct token));
    if (tok == NULL)
        return NULL;

    tok->type = type;
    tok->value = val;

    return tok;
}

void free_token(struct token *token)
{
    if (!token_type_has_int_value(token->type))
    {
        free_m_string(token->value.string);
    }
    free(token);
}

static char *token_type_to_string_a(enum token_type type)
{
    if (type == T_EOF)
        return "eof";
    if (type == T_WORD)
        return "word";
    if (type == T_NL)
        return "\\n";
    if (type == T_IONUMBER)
        return "ionumber";
    if (type == T_LEFT_PAREN)
        return "(";
    if (type == T_RIGHT_PAREN)
        return ")";
    if (type == T_LEFT_BRACK)
        return "{";
    if (type == T_RIGHT_BRACK)
        return "}";
    if (type == T_PIPE)
        return "|";
    if (type == T_DOUBLE_PIPE)
        return "||";
    if (type == T_SEMICOL)
        return ";";
    if (type == T_DOUBLE_SEMICOL)
        return ";;";
    if (type == T_AND)
        return "&";
    if (type == T_DOUBLE_AND)
        return "&&";
    if (type == T_EXCLAMATION)
        return "!";
    if (type == T_GT)
        return ">";
    if (type == T_LT)
        return "<";
    if (type == T_DOUBLE_GT)
        return ">>";
    return NULL;
}
static char *token_type_to_string_b(enum token_type type)
{
    if (type == T_DOUBLE_LT)
        return "<<";
    if (type == T_GT_AND)
        return ">&";
    if (type == T_LT_AND)
        return "<&";
    if (type == T_GT_PIPE)
        return ">|";
    if (type == T_LT_GT)
        return "<>";
    if (type == T_DOUBLE_LT_MINUS)
        return "<<-";
    if (type == T_FOR)
        return "for";
    if (type == T_DO)
        return "do";
    if (type == T_DONE)
        return "done";
    if (type == T_WHILE)
        return "while";
    if (type == T_UNTIL)
        return "until";
    if (type == T_CASE)
        return "case";
    if (type == T_IN)
        return "in";
    if (type == T_ESAC)
        return "esac";
    if (type == T_IF)
        return "if";
    if (type == T_THEN)
        return "then";
    if (type == T_FI)
        return "fi";
    if (type == T_ELSE)
        return "else";
    if (type == T_ELIF)
        return "elif";
    return NULL;
}

char *token_type_to_string(enum token_type type)
{
    char *res;
    res = token_type_to_string_a(type);
    if (res != NULL)
        return res;
    res = token_type_to_string_b(type);
    return res;
}

struct token *token_copy(struct token *token)
{
    struct token *cpy = malloc(sizeof(struct token));
    if (token == NULL)
        return NULL;
    cpy->type = token->type;
    if (token_type_has_int_value(token->type))
    {
        cpy->value.int_value = token->value.int_value;
        return cpy;
    }
    cpy->value.string = copy_m_string(token->value.string);
    if (cpy->value.string == NULL)
    {
        free_token(cpy);
        return NULL;
    }
    return cpy;
}
