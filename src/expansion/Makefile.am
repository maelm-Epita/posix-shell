lib_LIBRARIES = libexpansion.a

libexpansion_a_SOURCES = \
				  expansion.c \
				  expansion.h

libexpansion_a_CPPFLAGS = -I$(top_srcdir)/src
libexpansion_a_CFLAGS = -std=c99 -pedantic $(WERROR_FLAG) -Wall -Wextra -Wvla

noinst_LIBRARIES = libexpansion.a
