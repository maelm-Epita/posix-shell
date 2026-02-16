#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

/**
 ** @brief              Adds x bytes to a pointer address
 ** @param p            The pointer to add to
 ** @param x            The amount of bytes to add
 ** @return             A pointer to the address p+x
 */
void *pointer_add(void *p, size_t x);

#endif /* ! MEMORY_H */
