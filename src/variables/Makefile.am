lib_LIBRARIES = libvariables.a

libvariables_a_SOURCES = \
				   variables.c \
				   variables.h

libvariables_a_CPPFLAGS = -I$(top_srcdir)/src
libvariables_a_CFLAGS = -std=c99 -pedantic $(WERROR_FLAG) -Wall -Wextra -Wvla

noinst_LIBRARIES = libvariables.a
