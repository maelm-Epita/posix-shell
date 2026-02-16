#include "string_metadata.h"

#include <stdlib.h>
#include <string.h>

#include "utils/vector/vector_extra.h"

struct string_segment *create_segment(char *str, size_t str_size,
                                      enum quote_type type, int escaped)
{
    struct string_segment *seg = malloc(sizeof(struct string_segment));
    if (seg == NULL)
        return NULL;

    char *_value = calloc(str_size + 1, sizeof(char));
    if (_value == NULL)
        return NULL;
    _value = memcpy(_value, str, str_size);

    seg->str = _value;
    seg->escaped = escaped;
    seg->quote = type;
    return seg;
}

void free_segment(struct string_segment *seg)
{
    free(seg->str);
    free(seg);
}

void free_m_string(m_string str)
{
    for (size_t i = 0; i < str->size; i++)
    {
        struct string_segment *seg = vector_get_pointer(str, i);
        free_segment(seg);
    }
    vector_free(str);
}

m_string copy_m_string(m_string in)
{
    m_string dst = vector_init(sizeof(struct string_segment *));
    if (dst == NULL)
        return NULL;
    for (size_t i = 0; i < in->size; i++)
    {
        struct string_segment *seg = vector_get_pointer(in, i);
        struct string_segment *cpy = create_segment(seg->str, strlen(seg->str),
                                                    seg->quote, seg->escaped);
        if (vector_append(dst, &cpy) != 0)
        {
            free_m_string(dst);
            return NULL;
        }
    }
    return dst;
}

char *m_string_get_raw(m_string str)
{
    if (str == NULL || str->size == 0)
        return NULL;
    static char buf[1024] = { 0 };
    size_t idx = 0;
    for (size_t i = 0; i < str->size; i++)
    {
        struct string_segment *seg = vector_get_pointer(str, i);
        size_t size = strlen(seg->str);
        if (idx + size >= 1024)
            size -= (idx + size) - 1023;
        memcpy(buf + idx, seg->str, size);
        idx += size;
    }
    buf[idx] = 0;
    return buf;
}

m_string m_string_from_raw(char *str)
{
    m_string mstr = vector_init(sizeof(struct string_segment *));
    if (str == NULL)
        return NULL;
    struct string_segment *s =
        create_segment(str, strlen(str), Q_NOT_QUOTED, 0);
    if (s == NULL || vector_append(mstr, &s) == -1)
    {
        vector_free(mstr);
        return NULL;
    }
    return mstr;
}
