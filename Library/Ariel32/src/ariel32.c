// ARIEL - Arbitrary Resolution Integer Execution Library - 32 bit version
// Version 0.1a - first iteration - very ALPHA code!
//
// Note that the code is not currently optimized.
// Copyright 2025, Bryan K Reinhart

#include "ariel32.h"

#include <assert.h>


// remainder register
extern arz_t rres = { NULL, 0, 0, 0 };
extern arz_t* res = &rres;


// print message and exit
void invalid_parameter(const char* c)
{
    xprintf("Invalid parameter in %s call\n", c);
    terminate();
}

// Display an integer register in decimal format in output to file
// If fp=NULL then print to stdout.
void arz_fdisplay(FILE* fp, arz_t* a)
{
    if (a->limbs == NULL)
        invalid_parameter("arz_display");

    if (a->used_limbs < 1)
        invalid_parameter("arz_display");

    // Decimal occupies at least log(2^32)/log(10^9) times as much space.
    int32_t ddigits = ((9 * a->used_limbs) >> 3) + 4;
    if (res->limbs == NULL)
        _ualloc(res, ddigits, 1);

    if (res->alloc_limbs < ddigits)
        _uextend(res, ddigits);

    assert(res->limbs != NULL);

    // Repeatedly divide by 10^9, saving remainder starting at ddigits - 1
    _umov(res, a);
    res->used_limbs = a->used_limbs;
    int32_t decimal = ddigits;

    while (_xsig(res->limbs, res->used_limbs))
    {
        decimal--;
        res->limbs[decimal] = _xdivk(res->limbs, 1000000000, res->used_limbs);
        _utrim(res);
    }

    // Print the remainders
    if (arz_sgn(a) < 0)
    {
        if (fp)
            fprintf(fp, "-");
        else
            printf("-");
    }

    int32_t significant = 0;
    for (int32_t i = decimal; i < ddigits; i++)
    {
        if (res->limbs[i] && !significant)
        {
            if (fp)
                fprintf(fp, "%d", res->limbs[i]);
            else
                printf("%d", res->limbs[i]);

            significant = 1;
        }
        else
        {
            if (fp)
                fprintf(fp, "%09d", res->limbs[i]);
            else
                printf("%09d", res->limbs[i]);
        }
    }

    if (!significant)
    {
        if (fp)
            fprintf(fp, "0");
        else
            printf("0");
    }
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

// a = a + b
void arz_add(arz_t* a, arz_t* b)
{
    int32_t f;

    if (a->limbs == NULL)
        invalid_parameter("arz_add");

    if (b->limbs == NULL)
        invalid_parameter("arz_add");

    if (a->used_limbs < 1)
        invalid_parameter("arz_add");

    if (b->used_limbs < 1)
        invalid_parameter("arz_add");

    assert(a->limbs != NULL);

    // Extend a if shorter than b
    if (a->used_limbs < b->used_limbs)
    {
        if (a->alloc_limbs < b->used_limbs)
            _uextend(a, b->used_limbs);

        _xmovz(&a->limbs[a->used_limbs], b->used_limbs - a->used_limbs);
        a->used_limbs = b->used_limbs;
    }

    if ((a->flags & NegativeReg) == (b->flags & NegativeReg))
    {
        // Same signs
        f = _xadd(a->limbs, b->limbs, a->used_limbs, b->used_limbs);
        if (f & CarryFlag)
        {
            if (a->alloc_limbs <= a->used_limbs)
                _uextend(a, a->used_limbs + 1);

            a->limbs[a->used_limbs] = 1;
            a->used_limbs++;
        }
    }
    else
    {
        // Different signs
        f = _xsub(a->limbs, b->limbs, a->used_limbs, b->used_limbs);
        if (f & CarryFlag)
        {
            _xneg(a->limbs, a->used_limbs);
            a->flags ^= NegativeReg;
        }
    }
}

// a = a + k, -2^31 < k < 2^31
void arz_addk(arz_t* a, int32_t k)
{
    int32_t f;

    if (a->limbs == NULL)
        invalid_parameter("arz_addk");

    if (a->used_limbs < 1)
        invalid_parameter("arz_addk");

    assert(a->limbs != NULL);

    if (k >= 0)
    {
        if (!(a->flags & NegativeReg))
        {
            f = _xaddk(a->limbs, k, a->used_limbs);
            if (f & CarryFlag)
            {
                if (a->alloc_limbs <= a->used_limbs)
                    _uextend(a, a->used_limbs + 1);

                a->limbs[a->used_limbs] = 1;
                a->used_limbs++;
            }
        }
        else
        {
            f = _xsubk(a->limbs, k, a->used_limbs);
            if (f & CarryFlag)
            {
                _xneg(a->limbs, a->used_limbs);
                a->flags ^= NegativeReg;
            }
        }
    }
    else // k < 0
    {
        if (!(a->flags & NegativeReg))
        {
            f = _xsubk(a->limbs, -k, a->used_limbs);
            if (f & CarryFlag)
            {
                _xneg(a->limbs, a->used_limbs);
                a->flags ^= NegativeReg;
            }
        }
        else
        {
            f = _xaddk(a->limbs, -k, a->used_limbs);
            if (f & CarryFlag)
            {
                if (a->alloc_limbs <= a->used_limbs)
                    _uextend(a, a->used_limbs + 1);

                a->limbs[a->used_limbs] = 1;
                a->used_limbs++;
            }
        }
    }
}

// a = a - b
void arz_sub(arz_t* a, arz_t* b)
{
    int32_t f;

    if (a->limbs == NULL)
        invalid_parameter("arz_sub");

    if (b->limbs == NULL)
        invalid_parameter("arz_sub");

    if (a->used_limbs < 1)
        invalid_parameter("arz_sub");

    if (b->used_limbs < 1)
        invalid_parameter("arz_sub");

    assert(a->limbs != NULL);

    // Extend a if shorter than b
    if (a->used_limbs < b->used_limbs)
    {
        if (a->alloc_limbs < b->used_limbs)
            _uextend(a, b->used_limbs);

        _xmovz(&a->limbs[a->used_limbs], b->used_limbs - a->used_limbs);
        a->used_limbs = b->used_limbs;
    }

    if ((a->flags & NegativeReg) != (b->flags & NegativeReg))
    {
        // Different signs
        f = _xadd(a->limbs, b->limbs, a->used_limbs, b->used_limbs);
        if (f & CarryFlag)
        {
            if (a->alloc_limbs <= a->used_limbs)
                _uextend(a, a->used_limbs + 1);

            a->limbs[a->used_limbs] = 1;
            a->used_limbs++;
        }
    }
    else
    {
        // Same signs
        f = _xsub(a->limbs, b->limbs, a->used_limbs, b->used_limbs);
        if (f & CarryFlag)
        {
            _xneg(a->limbs, a->used_limbs);
            a->flags ^= NegativeReg;
        }
    }
}

// arz_sgn(a) = -1 if a < 0, 0 if a = 0, 1 if a > 0
int32_t arz_sgn(arz_t* a)
{
    if (a->limbs == NULL)
        invalid_parameter("arz_sgn");

    if (a->used_limbs < 1)
        invalid_parameter("arz_sgn");

    if (!_xsig(a->limbs, a->used_limbs))
        return 0;

    if (a->flags & NegativeReg)
        return -1;

    return 1;
}

// a = a * b
void arz_mul(arz_t* a, arz_t* b)
{
    if (a->limbs == NULL)
        invalid_parameter("arz_mul");

    if (b->limbs == NULL)
        invalid_parameter("arz_mul");

    if (a->used_limbs < 1)
        invalid_parameter("arz_mul");

    if (b->used_limbs < 1)
        invalid_parameter("arz_mul");

    _utrim(a);
    _utrim(b);

    int32_t da2 = (a->used_limbs + a->used_limbs);
    int32_t db2 = (b->used_limbs + b->used_limbs);
    int32_t dab = a->used_limbs + b->used_limbs;

    if (a->alloc_limbs < dab)
            _uextend(a, dab);
    
    _xmul(a->limbs, b->limbs, a->used_limbs, b->used_limbs);
    
}

// a = a * k
void arz_mulk(arz_t* a, int32_t k)
{
    if (a->limbs == NULL)
        invalid_parameter("arz_mulk");

    if (a->used_limbs < 1)
        invalid_parameter("arz_mulk");

    _utrim(a);

    if (a->alloc_limbs <= a->used_limbs)
        _uextend(a, a->used_limbs + 1);

    if (k < 0)
    {
        _xmulk(a->limbs, -k, a->used_limbs);
        a->flags ^= NegativeReg;
    }
    else
        _xmulk(a->limbs, k, a->used_limbs);

    a->used_limbs++;
}
