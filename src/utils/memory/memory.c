#include "memory.h"

#include <stdlib.h>
#include <string.h>

void *pointer_add(void *p, size_t x)
{
    char *_p = p;
    return _p + x;
}
