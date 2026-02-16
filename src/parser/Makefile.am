lib_LIBRARIES = libparser.a

libparser_a_SOURCES = \
				   parser.c \
				   parser.h \
				   parser_helpers.c \
				   parser_helpers.h \
				   parser_private.c \
				   parser_private.h \
				   parse_list.c \
				   parse_and_or.c \
				   parse_pipeline.c \
				   parse_command.c \
				   parse_simple_command.c \
				   parse_funcdec.c \
				   parse_shell_command.c \
				   parse_keyword_command.c \
				   parse_compound_list.c \
				   parse_rule_for.c \
				   parse_rule_while.c \
				   parse_rule_until.c \
				   parse_rule_case.c \
				   parse_rule_if.c \
				   parse_prefix.c \
				   parse_element.c \
				   parse_redirection.c \
				   parse_heredoc.c \
				   parse_case_clause.c \
				   parse_case_item.c \
				   parse_else_clause.c

libparser_a_CPPFLAGS = -I$(top_srcdir)/src
libparser_a_CFLAGS = -std=c99 -pedantic $(WERROR_FLAG) -Wall -Wextra -Wvla

noinst_LIBRARIES = libparser.a
