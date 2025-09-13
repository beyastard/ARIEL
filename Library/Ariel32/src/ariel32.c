// ARIEL - Arbitrary Resolution Integer Execution Library - 32 bit version
// Version 0.1a - first iteration - very ALPHA code!
//
// Note that the code is not currently optimized.
// Copyright 2025, Bryan K Reinhart

#include "ariel32.h"


// a = b
void arz_mov(arz_t* a, arz_t* b)
{
}

// a = k, -2^31 < k < 2^31
void arz_movk(arz_t* a, int32_t k)
{
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
