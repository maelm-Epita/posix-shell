#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include "io/io.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

enum parser_status parse_with_source(char *chars, struct ast **ast)
{
    struct char_stream stream = { .stream = chars, .size = strlen(chars) };
    struct io_source source = { .source_stream = { .char_stream = stream },
                                .source_type = IO_CHAR_STREAM };
    io_init(source);
    lexer_reset();
    return parse_input(ast);
}

TestSuite(parser_tests);

Test(rejection_tests, parser_tests, .init = cr_redirect_stderr)
{
    struct ast *ast = NULL;

    cr_assert(parse_with_source("done", &ast) == P_SYNTAX_ERR);
    cr_assert(parse_with_source("elif", &ast) == P_SYNTAX_ERR);
    cr_assert(parse_with_source("|", &ast) == P_SYNTAX_ERR);
    cr_assert(parse_with_source("case", &ast) == P_SYNTAX_ERR);
    cr_assert(parse_with_source("A=b if a then; b; fi", &ast) == P_SYNTAX_ERR);
    cr_assert(parse_with_source("<<test command1\ntest", &ast) == P_SYNTAX_ERR);
    cr_assert(parse_with_source("echo | ! echo", &ast) == P_SYNTAX_ERR);
    cr_assert(parse_with_source("( echo test ", &ast) == P_SYNTAX_ERR);
    cr_assert(parse_with_source("for() {echo test}", &ast) == P_SYNTAX_ERR);
    cr_assert(parse_with_source("test() echo test", &ast) == P_SYNTAX_ERR);
    cr_assert(parse_with_source("ls && ; ls", &ast) == P_SYNTAX_ERR);
}

Test(ending_new_line, parser_tests)
{
    struct ast *ast = NULL;

    cr_assert(parse_with_source("\n", &ast) == P_SUCCESS);
    cr_assert(parse_with_source("echo test\n", &ast) == P_SUCCESS);
}

Test(simple_command, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(parse_with_source("", &ast) == P_EOF);
    cr_assert(parse_with_source("echo test", &ast) == P_EOF);
}
Test(prefix, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(parse_with_source("A=b B=c", &ast) == P_EOF);
    cr_assert(parse_with_source("echo test > file.txt", &ast) == P_EOF);
    cr_assert(parse_with_source("A=b echo test 2>&1", &ast) == P_EOF);
}
Test(pipeline, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(parse_with_source("echo | echo", &ast) == P_EOF);
    cr_assert(parse_with_source("! echo |\n\n echo | \n echo | echo", &ast)
              == P_EOF);
}
Test(compound_shell_command, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(parse_with_source("{ echo test }", &ast) == P_EOF);
    cr_assert(parse_with_source("(echo test\n\n\n )", &ast) == P_EOF);
}
Test(funcdec, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(parse_with_source("test() {echo test}", &ast) == P_EOF);
    cr_assert(parse_with_source("test() \n\n\nif a; then b; fi", &ast)
              == P_EOF);
}
Test(and_or, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(parse_with_source("ls | ls && ls || ls | ls || ls", &ast)
              == P_EOF);
}
Test(list, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(
        parse_with_source(
            "ls | ls && ls || ls | ls || ls & ls ; ls || ls ; ls && ls", &ast)
        == P_EOF);
}
Test(heredoc, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(parse_with_source("<<test\ncommand1\ntest", &ast) == P_EOF);
}
Test(for, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(parse_with_source("for a ; \n\n do list; done", &ast) == P_EOF);
    cr_assert(parse_with_source("for a do list; done", &ast) == P_EOF);
    cr_assert(parse_with_source("for a \n\n do list; done", &ast) == P_EOF);
    cr_assert(parse_with_source("for a \nin a b c d\n\n do list\n done", &ast)
              == P_EOF);
}
Test(while, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(parse_with_source("while echo test; do this && that; done", &ast)
              == P_EOF);
}
Test(until, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(parse_with_source("until echo test; do this && that; done", &ast)
              == P_EOF);
}
Test(case, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(parse_with_source("case A in (a|b)echo test;;esac", &ast)
              == P_EOF);
    cr_assert(parse_with_source("case A\n\n\nin\n(a|b)echo test;;esac", &ast)
              == P_EOF);
}
Test(if, parser_tests)
{
    struct ast *ast = NULL;
    cr_assert(parse_with_source("if a; then b; fi", &ast) == P_EOF);
    cr_assert(
        parse_with_source("if a; then b; elif a; then b\n else c; fi", &ast)
        == P_EOF);
}
