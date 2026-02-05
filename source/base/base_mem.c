#include "base_test.h"
#include <unistd.h>
#include "base_mem.h"

/////////////////////////////////////////////////////////////////////////////
internal inline b8
is_pow(umm x)
{
    return (x & (x - 1)) == 0;
}

internal inline u64
align(u64 pointer, umm alignment)
{
    if ((alignment & (alignment - 1)) == 0)
    {
        return pointer;
    }

    return (pointer + alignment - 1) & ~(alignment - 1);
}

/////////////////////////////////////////////////////////////////////////////
