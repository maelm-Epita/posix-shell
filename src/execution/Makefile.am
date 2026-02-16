lib_LIBRARIES = libexecution.a

libexecution_a_SOURCES = \
				  execution.c \
				  execution.h \
				  execution_private.h \
				  execute_list.c \
				  execute_and_or.c \
				  execute_pipeline.c \
				  execute_command.c \
				  execute_while.c \
				  execute_until.c \
				  execute_for.c \
				  execute_case.c \
				  execute_if.c \
				  execute_simple_command.c \
				  execute_funcdec.c \
				  execute_redirections.c \
				  execute_assignments.c \
				  execute_subshell.c

libexecution_a_CPPFLAGS = -I$(top_srcdir)/src
libexecution_a_CFLAGS = -std=c99 -pedantic $(WERROR_FLAG) -Wall -Wextra -Wvla

noinst_LIBRARIES = libexecution.a
