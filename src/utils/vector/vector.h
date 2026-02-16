#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

static const int VEC_DEFAULT_CAPACITY = 8;

/**
 ** @brief              Generic vector type
 ** @param data         Internal data array
 ** @param size         Number of elements
 ** @param data_size    Size of elements
 ** @param _capacity    Capacity of data array
 */
typedef struct
{
    void *data;
    size_t size;
    size_t _data_size;
    size_t _capacity;
} vector;

/**
 ** @brief              Creates an empty vector with default capacity
 ** @param data_size    The size of one element
 ** @return             A pointer to the created vector on success, NULL on
 * failure
 */
vector *vector_init(const size_t data_size);

/**
 ** @brief              Frees a vector
 ** @param vec          A pointer to the vector to be freed
 ** @return             void
 */
void vector_free(vector *vec);

/**
 ** @brief              Appends data to the vector
 ** @param vec          A pointer to the vector to append to
 ** @param data         A pointer to the data to append
 ** @return             Returns 0 on success, -1 on failure (memory allocation
 * failure)
 */
int vector_append(vector *vec, void *data);

/**
 ** @brief              Pops data from the vector
 ** @param vec          A pointer to the vector to pop from
 ** @return             Returns 0 on success, -1 on failure (memory allocation
 * failure)
 */
int vector_pop(vector *vec);

/**
 ** @brief              Gets the element at index i (crashes via assert on
 * overflow)
 ** @param vec          A pointer to the vector to get the element from
 ** @param i            The index of the element to get
 ** @return             Returns a pointer to the element
 */
void *vector_get(vector *vec, size_t i);

/**
 ** @brief              Sets the element at index i
 ** @param vec          A pointer to the vector to set the element in
 ** @param i            The index of the element to set
 ** @param data         A pointer to the data to set the element to
 ** @return             void
 */
void vector_set(vector *vec, size_t i, void *data);

#endif /* ! VECTOR_H */
