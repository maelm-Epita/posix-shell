lib_LIBRARIES = libshell.a

libshell_a_SOURCES = \
				  shell.c \
				  shell.h
libshell_a_CPPFLAGS = -I$(top_srcdir)/src
libshell_a_CFLAGS = -std=c99 -pedantic $(WERROR_FLAG) -Wall -Wextra -Wvla

noinst_LIBRARIES = libshell.a
