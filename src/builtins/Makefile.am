lib_LIBRARIES = libbuiltins.a

libbuiltins_a_SOURCES = \
				   builtins.c \
				   builtins.h \
				   echo.c \
				   exit.c \
				   cd.c \
				   export.c \
				   unset.c \
				   dot.c \
				   break.c \
				   continue.c \
				   alias.c

libbuiltins_a_CPPFLAGS = -I$(top_srcdir)/src
libbuiltins_a_CFLAGS = -std=c99 -pedantic $(WERROR_FLAG) -Wall -Wextra -Wvla

noinst_LIBRARIES = libbuiltins.a
