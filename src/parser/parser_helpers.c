#include "parser_helpers.h"

#include <ctype.h>
#include <stdio.h>

#include "token/token.h"
#include "utils/vector/vector_extra.h"

static int is_valid_name(char *str)
{
    if (isdigit(str[0]))
        return 0;
    int has_alpha_under = 0;
    for (size_t i = 0; str[i] != 0; i++)
    {
        if (!isalnum(str[i]) && str[i] != '_')
            return 0;
        if (isalpha(str[i]) || str[i] == '_')
            has_alpha_under = 1;
    }
    return has_alpha_under;
}

int is_assignment(struct token *tok)
{
    if (tok->type != T_WORD)
        return 0;
    m_string str = tok->value.string;
    char term = 0;
    vector *acc = vector_init(sizeof(char));
    int has_unquoted_equal = 0;
    for (size_t i = 0; i < str->size && !has_unquoted_equal; i++)
    {
        struct string_segment *seg = vector_get_pointer(str, i);
        if (seg->escaped || seg->quote != Q_NOT_QUOTED)
            goto fail_cleanup;
        char *str = seg->str;
        while (*str != 0)
        {
            if (*str == '=')
            {
                has_unquoted_equal = 1;
                break;
            }
            vector_append(acc, str);
            str++;
        }
    }
    vector_append(acc, &term);
    if (!has_unquoted_equal)
        goto fail_cleanup;
    int res = is_valid_name(acc->data);
    vector_free(acc);
    return res;
fail_cleanup:
    vector_free(acc);
    return 0;
}

int is_in_shell_command_first(struct token *tok)
{
    return (
        token_matches_type(tok, T_LEFT_BRACK) || tok->type == T_LEFT_PAREN
        || token_matches_type(tok, T_FOR) || token_matches_type(tok, T_WHILE)
        || token_matches_type(tok, T_UNTIL) || token_matches_type(tok, T_CASE)
        || token_matches_type(tok, T_IF));
}

int is_in_prefix_first(struct token *tok)
{
    return (is_assignment(tok) || is_in_redirection_first(tok));
}

int is_in_element_first(struct token *tok)
{
    return (tok->type == T_WORD || is_in_redirection_first(tok));
}

int is_in_redirection_first(struct token *tok)
{
    enum token_type t = tok->type;
    return (t == T_IONUMBER || t == T_GT || t == T_LT || t == T_DOUBLE_GT
            || t == T_GT_AND || t == T_LT_AND || t == T_GT_PIPE || t == T_LT_GT
            || t == T_DOUBLE_LT || t == T_DOUBLE_LT_MINUS);
}

int is_in_and_or_first(struct token *tok)
{
    return (tok->type == T_EXCLAMATION || is_assignment(tok)
            || is_in_redirection_first(tok) || is_in_shell_command_first(tok))
        || (tok->type == T_WORD && !token_matches_type(tok, T_DO)
            && !token_matches_type(tok, T_DONE)
            && !token_matches_type(tok, T_THEN)
            && !token_matches_type(tok, T_ELSE)
            && !token_matches_type(tok, T_ELIF)
            && !token_matches_type(tok, T_FI)
            && !token_matches_type(tok, T_ESAC)
            && !token_matches_type(tok, T_RIGHT_BRACK));
}

int is_in_case_item_clause_first(struct token *tok)
{
    return (
        (tok->type == T_WORD && !token_matches_type(tok, T_DO)
         && !token_matches_type(tok, T_DONE) && !token_matches_type(tok, T_THEN)
         && !token_matches_type(tok, T_ELSE) && !token_matches_type(tok, T_ELIF)
         && !token_matches_type(tok, T_FI) && !token_matches_type(tok, T_ESAC))
        || tok->type == T_LEFT_PAREN);
}

int is_in_else_clause_first(struct token *tok)
{
    return (token_matches_type(tok, T_ELSE) || token_matches_type(tok, T_ELIF));
}

int is_in_compound_list_first(struct token *tok)
{
    return (tok->type == T_NL || is_in_and_or_first(tok));
}

int is_funcdec(struct token *first, struct token *second)
{
    return ((first->type == T_WORD && !token_matches_type(first, T_DO)
             && !token_matches_type(first, T_DONE)
             && !token_matches_type(first, T_THEN)
             && !token_matches_type(first, T_ELSE)
             && !token_matches_type(first, T_ELIF)
             && !token_matches_type(first, T_FI)
             && !token_matches_type(first, T_ESAC))
            && second->type == T_LEFT_PAREN);
}
