#include <criterion/criterion.h>

#include "io/io.h"
#include "lexer/lexer.h"

TestSuite(lexer_tests);

Test(lexer_tests_long_input, lexer_tests)
{
    char *chars = "While tr=ue echo >> $b \"test =string\" e\\\"\\( \'test "
                  "string\'\n;; && || | <& | >& \n     1<<- \n ({!})\n&";
    struct token expected_tokens[] = {
        { T_WORD, { .str_value = "While" } },
        { T_ASSIGNMENT_WORD, { .str_value = "tr=ue" } },
        { T_WORD, { .str_value = "echo" } },
        { T_DOUBLE_GT, { .str_value = ">>" } },
        { T_WORD, { .str_value = "$b" } },
        { T_WORD, { .str_value = "test =string" } },
        { T_WORD, { .str_value = "e\"(" } },
        { T_WORD, { .str_value = "test string" } },
        { T_NL, { .str_value = "\n" } },
        { T_DOUBLE_SEMICOL, { .str_value = ";;" } },
        { T_DOUBLE_AND, { .str_value = "&&" } },
        { T_DOUBLE_PIPE, { .str_value = "||" } },
        { T_PIPE, { .str_value = "|" } },
        { T_LT_AND, { .str_value = "<&" } },
        { T_PIPE, { .str_value = "|" } },
        { T_GT_AND, { .str_value = ">&" } },
        { T_NL, { .str_value = "\n" } },
        { T_IONUMBER, { .int_value = 1 } },
        { T_DOUBLE_LT_MINUS, { .str_value = "<<-" } },
        { T_NL, { .str_value = "\n" } },
        { T_LEFT_PAREN, { .str_value = "(" } },
        { T_LEFT_BRACK, { .str_value = "{" } },
        { T_EXCLAMATION, { .str_value = "!" } },
        { T_RIGHT_BRACK, { .str_value = "}" } },
        { T_RIGHT_PAREN, { .str_value = ")" } },
        { T_NL, { .str_value = "\n" } },
        { T_AND, { .str_value = "&" } },
        { T_EOF, { .str_value = "" } }
    };

    struct char_stream stream = { .stream = chars, .size = strlen(chars) };
    struct io_source source = { .source_stream = { .char_stream = stream },
                                .source_type = IO_CHAR_STREAM };
    io_init(source);

    struct token *tok;
    size_t i = 0;

    cr_assert(lexer_lookahead(&tok, 2) == LEXER_SUCCESS);
    cr_assert(tok->type == T_WORD);
    cr_assert(strcmp(tok->value.str_value, expected_tokens[2].value.str_value)
              == 0);
    cr_assert(lexer_lookahead(&tok, 0) == LEXER_SUCCESS); // equivalent to peek
    cr_assert(tok->type == T_WORD);
    cr_assert(strcmp(tok->value.str_value, expected_tokens[0].value.str_value)
              == 0);
    while (1)
    {
        cr_assert(lexer_peek(&tok) == LEXER_SUCCESS);
        cr_assert(tok->type == expected_tokens[i].type, "expected %s, got %s",
                  token_type_to_string(expected_tokens[i].type),
                  token_type_to_string(tok->type));
        if (token_type_has_int_value(tok->type))
            cr_assert(tok->value.int_value
                      == expected_tokens[i].value.int_value);
        else
            cr_assert(
                strcmp(tok->value.str_value, expected_tokens[i].value.str_value)
                    == 0,
                "expected %s, got %s", expected_tokens[i].value.str_value,
                tok->value.str_value);

        if (tok->type == T_EOF)
            break;

        cr_assert(lexer_eat("main", tok->type) == 0);
        i++;
    }
}
