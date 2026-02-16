lib_LIBRARIES = libutils.a

libutils_a_SOURCES = \
				  hash_map/hash_map.c \
				  hash_map/hash_map.h \
				  memory/memory.c \
				  memory/memory.h \
				  queue/queue.c \
				  queue.queue.h \
				  vector/vector.c \
				  vector/vector.h \
				  vector/vector_extra.c \
				  vector/vector_extra.h \
				  string/string_helpers.h \
				  string/string_helpers.c

libutils_a_CPPFLAGS = -I$(top_srcdir)/src
libutils_a_CFLAGS = -std=c99 -pedantic $(WERROR_FLAG) -Wall -Wextra -Wvla

noinst_LIBRARIES = libutils.a
