lib_LIBRARIES = libtoken.a

libtoken_a_SOURCES = \
				   token.c \
				   token.h \
				   string_metadata.h \
				   string_metadata.c \
				   aliases.c \
				   aliases.h

libtoken_a_CPPFLAGS = -I$(top_srcdir)/src
libtoken_a_CFLAGS = -std=c99 -pedantic $(WERROR_FLAG) -Wall -Wextra -Wvla

noinst_LIBRARIES = libtoken.a
