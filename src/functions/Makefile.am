lib_LIBRARIES = libfunctions.a

libfunctions_a_SOURCES = \
				   functions.c \
				   functions.h
libfunctions_a_CPPFLAGS = -I$(top_srcdir)/src
libfunctions_a_CFLAGS = -std=c99 -pedantic $(WERROR_FLAG) -Wall -Wextra -Wvla

noinst_LIBRARIES = libfunctions.a
