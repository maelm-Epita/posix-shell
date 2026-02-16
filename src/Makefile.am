SUBDIRS = \
		  ast \
		  parser \
		  lexer \
		  io \
		  execution \
		  expansion \
		  matching \
		  shell \
		  token \
		  builtins \
		  variables \
		  functions \
		  utils

bin_PROGRAMS = 42sh

42sh_SOURCES = 42sh.c

42sh_CPPFLAGS = -I%D%
42sh_CFLAGS = -std=c99 -pedantic $(WERROR_FLAG) -Wall -Wextra -Wvla

42sh_LDADD = \
			shell/libshell.a \
			execution/libexecution.a \
			matching/libmatching.a \
			expansion/libexpansion.a \
			parser/libparser.a \
			lexer/liblexer.a \
			ast/libast.a \
			builtins/libbuiltins.a \
			token/libtoken.a \
			io/libio.a \
			variables/libvariables.a \
			functions/libfunctions.a \
			utils/libutils.a
