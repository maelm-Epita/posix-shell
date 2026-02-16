#ifndef IO_H
#define IO_H

#include <stdio.h>

#define PEEK_CAPACITY 4096

/**
 ** @brief              Represents a stream of characters as a source data
 * stream for IO
 ** @param size         Number of characters in the stream
 */
struct char_stream
{
    char *stream;
    size_t size;
};

enum io_source_type
{
    IO_FILE,
    IO_CHAR_STREAM
};

union io_source_stream
{
    struct char_stream char_stream;
    FILE *file;
};

/**
 ** @brief                  Represents the source of IO
 ** @param source_type      A union representing one of two types: FILE or
 * CHAR_STREAM
 ** @param source_stream    The source stream, either a file or char stream
 */
struct io_source
{
    enum io_source_type source_type;
    union io_source_stream source_stream;
};

/* implementation detail */
struct io_internal
{
    size_t peek_size;
    size_t peek_start;
    char peek_buf[PEEK_CAPACITY];
};

/* implementation detail */
struct io
{
    struct io_source source;
    struct io_internal internal;
};

/**
 ** @brief              Initiates the IO by setting its source and initializing
 * its internals
 ** @param source       The io_source struct to use as a source
 ** @return             void
 */
void io_init(struct io_source source);

/**
 ** @brief              Peeks count chars from the input without consuming them
 ** @param out_buf      The buffer to output the peeked characters into.
 ** @param count        The number of chars to peek, should be <= PEEK_CAPACITY
 ** @return             The number of chars actually peeked (can be less if EOF
 * was reached)
 ** @safety             The returned buffer will only be valid until the next
 * call to this function
 */
size_t io_peek(char **out_buf, size_t count);

/**
 ** @brief              Consumes count chars from the input
 ** @param count        The number of chars to consume
 ** @return             void
 */
void io_consume(size_t count);

struct io_source io_get_source(void);

#endif /* ! IO_H */
