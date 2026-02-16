lib_LIBRARIES = libio.a

libio_a_SOURCES = \
				   io.c \
				   io.h
libio_a_CPPFLAGS = -I$(top_srcdir)/src
libio_a_CFLAGS = -std=c99 -pedantic $(WERROR_FLAG) -Wall -Wextra -Wvla

noinst_LIBRARIES = libio.a
