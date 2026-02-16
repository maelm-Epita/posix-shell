#ifndef VECTOR_EXTRA_H
#define VECTOR_EXTRA_H

#include "vector.h"

/**
 ** @brief              Appends n elements of nbsize from a data array to the
 * vector
 ** @param vec          A pointer to the vector to append to
 ** @param data         A pointer to the data array to append
 ** @param n            The number of elements to append
 ** @return             Returns 0 on success, -1 on failure (memory allocation
 * failure)
 */
int vector_append_n(vector *vec, void *data, size_t n);

/**
 ** @brief              Gets the element at index i dereferenced (crashes via
 * assert on overflow)
 ** @param vec          A pointer to the vector to get the element from
 ** @param i            The index of the element to get
 ** @return             Returns a pointer to the element dereferenced
 ** @safety             This is only safe to use if every element of the vector
 * is a pointer
 */
void *vector_get_pointer(vector *vec, size_t i);

/**
 ** @brief              Frees a vector of pointer as well as every pointer
 * inside
 ** @param vec          A pointer to the vector free
 ** @return             void
 ** @safety             This is only safe to use if every element of the vector
 * is a pointer
 */
void vector_free_pointer(vector *vec);

/**
 ** @brief              Tries to get the element at index i
 ** @param vec          A pointer to the vector to get the element from
 ** @param i            The index of the element to get
 ** @return             Returns a pointer to the element on success, NULL on
 * failure (overflow)
 */
void *vector_try_get(vector *vec, size_t i);

/**
 ** @brief              Tries to get the element at index i dereferenced
 ** @param vec          A pointer to the vector to get the element from
 ** @param i            The index of the element to get
 ** @return             Returns a pointer to the element on success, NULL on
 * failure (overflow)
 ** @safety             This is only safe to use if every element of the vector
 * is a pointer
 */
void *vector_try_get_pointer(vector *vec, size_t i);

/**
 ** @brief              Clears the vector (resets size to 0)
 ** @param vec          The vector to clear
 ** @return             void
 */
void vector_clear(vector *vec);

#endif /* VECTOR_EXTRA_H */
