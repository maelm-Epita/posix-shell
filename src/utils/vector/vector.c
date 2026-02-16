#include "vector.h"

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

static inline int decrease_data(vector *vec)
{
    assert(vec != NULL);
    assert(vec->data != NULL);

    vec->_capacity /= 2;

    vec->data = realloc_data(vec);

    if (vec->data == NULL)
        return -1;

    return 0;
}

vector *vector_init(const size_t data_size)
{
    vector *vec = malloc(sizeof(vector));

    if (vec == NULL)
        return NULL;

    vec->_data_size = data_size;
    vec->_capacity = VEC_DEFAULT_CAPACITY;
    vec->size = 0;
    vec->data = malloc(vec->_data_size * vec->_capacity);

    if (vec->data == NULL)
        return NULL;

    return vec;
}

void vector_free(vector *vec)
{
    if (vec == NULL)
        return;

    if (vec->data != NULL)
        free(vec->data);

    free(vec);
}

int vector_append(vector *vec, void *data)
{
    if (vec == NULL || data == NULL)
        return 0;

    if (vec->size == vec->_capacity)
    {
        if (expand_data(vec) == -1)
            return -1;
    }

    assert(vec->size < vec->_capacity);

    void *data_unused = get_ith(vec, vec->size);

    memcpy(data_unused, data, vec->_data_size);

    vec->size++;

    return 0;
}

int vector_pop(vector *vec)
{
    if (vec == NULL || vec->size == 0)
        return 0;

    if (vec->size < vec->_capacity / 2)
    {
        if (decrease_data(vec) == -1)
            return -1;
    }

    vec->size--;

    return 0;
}

void *vector_get(vector *vec, size_t i)
{
    // assert(i < vec->size);

    void *element = get_ith(vec, i);

    return element;
}

void vector_set(vector *vec, size_t i, void *data)
{
    assert(i < vec->size);

    char *arr = vec->data;

    memcpy(arr + (i * vec->_data_size), data, vec->_data_size);
}
