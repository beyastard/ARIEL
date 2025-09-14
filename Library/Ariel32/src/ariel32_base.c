// ARIEL - Arbitrary Resolution Integer Execution Library - 32 bit version
// Version 0.1a - first iteration - very ALPHA code!
//
// Note that the code is not currently optimized.
// Copyright 2025, Bryan K Reinhart

#include "ariel32_base.h"


// Allocate array memory for an integer register
// a = ARIEL integer register
// c = capacity of ARIEL integer register in digits
// d = initial number of digits
void _ualloc(arz_t* a, int32_t c, int32_t d)
{
    a->alloc_limbs = max(c, 1);
    a->used_limbs = min(max(d, 1), c);
    a->flags = 0;
    a->limbs = malloc(sizeof(limb_t) * c);

    if (a->limbs != NULL)
        _xmovz(a->limbs, a->used_limbs);
    else
    {
        xprintf("Unable to allocate %d digits in function _ualloc!\n", a->alloc_limbs);
        terminate();
    }
}

// Free ARIEL integer array memory
void _ufree(arz_t* a)
{
    free(a->limbs);
    a->limbs = NULL;
}

// Extend an integer register
// a = ARIEL integer register
// d = number of digits to extend ARIEL integer register to
void _uextend(arz_t* a, int32_t d)
{
    if (a->limbs == NULL)
    {
        _ualloc(a, d, d);
        return;
    }

    if (d <= a->alloc_limbs)
        return;

    // Allocate new array, copy old into new, free old
    limb_t* v = realloc(a->limbs, sizeof(limb_t) * d);
    if (v != NULL)
    {
        a->limbs = v;
        a->alloc_limbs = d;
    }
    else
    {
        xprintf("Unable to allocate %d extra digits in _uextend!\n", d - a->alloc_limbs);
        terminate();
    }
}

// formatted print
int xprintf(const char* s, ...)
{
    int retval;
    va_list arglist;

    va_start(arglist, s);
#ifdef _MSC_VER
    retval = vprintf_s(s, arglist);
#else
    retval = vprintf(s, arglist);
#endif
    va_end(arglist);

    return retval;
}
