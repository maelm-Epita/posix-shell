SUBDIRS = src tests

check_PROGRAMS = test_expansion

test_expansion_SOURCES = tests/unit_tests/expansion_unit_suite.c

test_expansion_CFLAGS = -std=c99 $(WERROR_FLAG) -Wall -Wextra -g -I$(top_srcdir)/src

test_expansion_LDADD = \
			$(top_builddir)/src/expansion/libexpansion.a \
			$(top_builddir)/src/shell/libshell.a \
			$(top_builddir)/src/execution/libexecution.a \
			$(top_builddir)/src/parser/libparser.a \
			$(top_builddir)/src/lexer/liblexer.a \
			$(top_builddir)/src/ast/libast.a \
			$(top_builddir)/src/builtins/libbuiltins.a \
			$(top_builddir)/src/token/libtoken.a \
			$(top_builddir)/src/io/libio.a \
			$(top_builddir)/src/variables/libvariables.a \
			$(top_builddir)/src/functions/libfunctions.a \
			$(top_builddir)/src/matching/libmatching.a \
			$(top_builddir)/src/utils/libutils.a \
			-lcriterion

TESTS = test_expansion
