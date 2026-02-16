#include "vector_extra.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils/memory/memory.h"

static inline vector *realloc_data(vector *vec)
{
    assert(vec != NULL);
    assert(vec->data != NULL);

    return realloc(vec->data, vec->_data_size * vec->_capacity);
}

static inline int expand_data(vector *vec)
{
    assert(vec != NULL);
    assert(vec->data != NULL);

    vec->_capacity *= 2;

    vec->data = realloc_data(vec);

    if (vec->data == NULL)
        return -1;

    return 0;
}

static inline void *get_ith(vector *vec, size_t i)
{
    assert(vec != NULL);
    assert(vec->data != NULL);

    return pointer_add(vec->data, i * vec->_data_size);
}

int vector_append_n(vector *vec, void *data, size_t n)
{
    if (vec == NULL || data == NULL)
        return 0;

    while (vec->size + n > vec->_capacity)
    {
        if (expand_data(vec) == -1)
            return -1;
    }

    assert(vec->size + n <= vec->_capacity);

    void *data_unused = get_ith(vec, vec->size);

    memcpy(data_unused, data, n * vec->_data_size);

    vec->size += n;

    return 0;
}

void *vector_get_pointer(vector *vec, size_t i)
{
    assert(i < vec->size);

    void **element = get_ith(vec, i);

    return *element;
}

void vector_free_pointer(vector *vec)
{
    if (vec == NULL)
        return;
    if (vec->data != NULL)
    {
        for (size_t i = 0; i < vec->size; i++)
        {
            void *el = vector_get_pointer(vec, i);
            if (el != NULL)
                free(el);
        }
        free(vec->data);
    }
    free(vec);
}

void *vector_try_get(vector *vec, size_t i)
{
    if (i >= vec->size)
        return NULL;

    return vector_get(vec, i);
}

void *vector_try_get_pointer(vector *vec, size_t i)
{
    if (i >= vec->size)
        return NULL;
    return vector_get_pointer(vec, i);
}

void vector_clear(vector *vec)
{
    vec->size = 0;
}
