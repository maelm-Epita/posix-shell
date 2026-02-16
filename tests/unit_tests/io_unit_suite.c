#define _GNU_SOURCE
#include <criterion/criterion.h>

#include "io/io.h"

TestSuite(io_tests);

Test(io_char_stream_test, io_tests)
{
    char *chars = "Test stream, let's see what happens";
    struct char_stream stream = { .stream = chars, .size = strlen(chars) };
    struct io_source source = { .source_stream = { .char_stream = stream },
                                .source_type = IO_CHAR_STREAM };
    io_init(source);

    char *peek;
    size_t bytes;

    io_consume(5);
    bytes = io_peek(&peek, 6);
    cr_assert(bytes == 6 && memcmp(peek, "stream", 6) == 0);
    bytes = io_peek(&peek, 5);
    cr_assert(bytes == 5 && memcmp(peek, "strea", 5) == 0);
    io_consume(8);
    bytes = io_peek(&peek, 5);
    cr_assert(bytes == 5 && memcmp(peek, "let's", 5) == 0);
    io_consume(19);
    bytes = io_peek(&peek, 5);
    cr_assert(bytes == 3 && memcmp(peek, "ens", 3) == 0);
    io_consume(100);
    bytes = io_peek(&peek, 10);
    cr_assert(bytes == 0);
}

Test(io_file_test, io_tests)
{
    char *chars = "Test stream, let's see what happens";
    FILE *file = fmemopen(chars, strlen(chars), "r");
    struct io_source source = { .source_stream = { .file = file },
                                .source_type = IO_FILE };
    io_init(source);

    char *peek;
    size_t bytes;

    io_consume(5);
    bytes = io_peek(&peek, 6);
    cr_assert(bytes == 6 && memcmp(peek, "stream", 6) == 0);
    bytes = io_peek(&peek, 5);
    cr_assert(bytes == 5 && memcmp(peek, "strea", 5) == 0);
    io_consume(8);
    bytes = io_peek(&peek, 5);
    cr_assert(bytes == 5 && memcmp(peek, "let's", 5) == 0);
    io_consume(19);
    bytes = io_peek(&peek, 5);
    cr_assert(bytes == 3 && memcmp(peek, "ens", 3) == 0);
    io_consume(100);
    bytes = io_peek(&peek, 10);
    cr_assert(bytes == 0);

    fclose(file);
    file = fmemopen(chars, strlen(chars), "r");
    source.source_stream.file = file;
    io_init(source);

    io_consume(21);
    bytes = io_peek(&peek, 3);
    cr_assert(bytes == 3 && memcmp(peek, "e w", 3) == 0);
}
