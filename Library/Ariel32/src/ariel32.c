// ARIEL - Arbitrary Resolution Integer Execution Library - 32 bit version
// Version 0.1a - first iteration - very ALPHA code!
//
// Note that the code is not currently optimized.
// Copyright 2025, Bryan K Reinhart

#include "ariel32.h"

#include <assert.h>


// print message and exit
void invalid_parameter(const char* c)
{
#ifdef _MSC_VER
    printf_s("Invalid parameter in %s call\n", c);
#else
    printf("Invalid parameter in %s call\n", c);
#endif
    terminate();
}

// a = b
void arz_mov(arz_t* a, arz_t* b)
{
    if (b->limbs == NULL)
        invalid_parameter("arz_mov");

    if (b->used_limbs < 1)
        invalid_parameter("arz_mov");

    if (a->limbs == NULL)
        _ualloc(a, b->used_limbs, b->used_limbs);

    if (a->alloc_limbs < b->used_limbs)
        _uextend(a, b->used_limbs);

    _xmovf(a->limbs, b->limbs, b->used_limbs);
    a->used_limbs = b->used_limbs;
    a->flags = b->flags;
}

// a = k, -2^31 < k < 2^31
void arz_movk(arz_t* a, int32_t k)
{
    if (a->limbs == NULL)
        _ualloc(a, 8, 1);

    if (a->used_limbs < 1)
        invalid_parameter("arz_movk");

    assert(a->limbs != NULL);

    if (k >= 0)
    {
        a->limbs[0] = k;
        a->flags = 0;
    }
    else // k < 0
    {
        a->limbs[0] = -k;
        a->flags = NegativeReg;
    }

    a->used_limbs = 1;
}
