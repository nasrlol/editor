#ifndef BASE_MEM_H
#define BASE_MEM_H

#include "base.h"

#define ARENA_ALIGN (2 * sizeof(void *))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

internal inline b8
is_pow(umm x);
internal inline u64
align(u64 ptr, umm alignment);

#endif
