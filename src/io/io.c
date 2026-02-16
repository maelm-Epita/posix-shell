#include "io.h"

#include <assert.h>
#include <string.h>

#define io_char_stream (io.source.source_stream.char_stream)
#define io_peek_buff_start (io.internal.peek_buf + io.internal.peek_start)

static struct io io = { 0 };

static void io_ensure(size_t count)
{
    if (io.internal.peek_size >= count)
        return;

    size_t avail =
        PEEK_CAPACITY - (io.internal.peek_start + io.internal.peek_size);
    if (avail < count)
    {
        memmove(io.internal.peek_buf, io_peek_buff_start,
                io.internal.peek_size);
        io.internal.peek_start = 0;
    }

    size_t to_read = count - io.internal.peek_size;
    size_t bytes_read;
    while ((bytes_read =
                fread(io_peek_buff_start + io.internal.peek_size, sizeof(char),
                      to_read, io.source.source_stream.file))
               > 0
           && to_read > 0)
    {
        io.internal.peek_size += bytes_read;
        to_read -= bytes_read;
    }
}

static size_t io_peek_char_stream(char **out_buf, size_t count)
{
    *out_buf = io_char_stream.stream;
    return io_char_stream.size > count ? count : io_char_stream.size;
}
static size_t io_peek_file(char **out_buf, size_t count)
{
    io_ensure(count);

    *out_buf = io_peek_buff_start;
    return io.internal.peek_size > count ? count : io.internal.peek_size;
}

static void io_consume_char_stream(size_t count)
{
    size_t consume = io_char_stream.size > count ? count : io_char_stream.size;
    io_char_stream.stream += consume;
    io_char_stream.size -= consume;
}

static void io_consume_file(size_t count)
{
    size_t consume = count % PEEK_CAPACITY;
    size_t throw_away_bufs = count / PEEK_CAPACITY;
    for (size_t i = 0; i < throw_away_bufs; i++)
    {
        io_ensure(PEEK_CAPACITY);
        io.internal.peek_size = 0;
    }

    io_ensure(consume);

    io.internal.peek_start += consume;
    io.internal.peek_size =
        consume > io.internal.peek_size ? 0 : io.internal.peek_size - consume;
}

void io_init(struct io_source source)
{
    io.source = source;
    memset(io.internal.peek_buf, 0, PEEK_CAPACITY);
    io.internal.peek_size = 0;
    io.internal.peek_start = 0;
}

size_t io_peek(char **out_buf, size_t count)
{
    assert(count <= PEEK_CAPACITY);
    switch (io.source.source_type)
    {
    case IO_CHAR_STREAM:
        return io_peek_char_stream(out_buf, count);
    case IO_FILE:
        return io_peek_file(out_buf, count);
    }
    return 0;
}

void io_consume(size_t count)
{
    switch (io.source.source_type)
    {
    case IO_CHAR_STREAM:
        io_consume_char_stream(count);
        break;
    case IO_FILE:
        io_consume_file(count);
        break;
    }
}

struct io_source io_get_source(void)
{
    return io.source;
}

void io_set_source(struct io_source s)
{
    io.source = s;
}
