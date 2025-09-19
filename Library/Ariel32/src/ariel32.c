// ARIEL - Arbitrary Resolution Integer Execution Library - 32 bit version
// Version 0.1a - first iteration - very ALPHA code!
//
// Note that the code is not currently optimized.
// Copyright 2025, Bryan K Reinhart

#include "ariel32.h"

#include <assert.h>
#include <math.h>


// remainder register
extern arz_t rres = { NULL, 0, 0, 0 };
extern arz_t* res = &rres;
extern int32_t kres = 0;

// temporary registers
extern arz_t rtemp1 = { NULL, 0, 0, 0 };
extern arz_t rtemp2 = { NULL, 0, 0, 0 };
extern arz_t rtemp3 = { NULL, 0, 0, 0 };
extern arz_t rtemp4 = { NULL, 0, 0, 0 };
extern arz_t rtemp5 = { NULL, 0, 0, 0 };

extern arz_t* temp1 = &rtemp1;
extern arz_t* temp2 = &rtemp2;
extern arz_t* temp3 = &rtemp3;
extern arz_t* temp4 = &rtemp4;
extern arz_t* temp5 = &rtemp5;

// useful constants
extern double dlog2  = 0.69314718055994530941723212145817656807;
extern double dlog10 = 2.30258509299404568401799145468436420757;


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

// a =  value converted from a string of decimal digits denoted by 'digits'
void arz_movd(arz_t* a, const char* digits)
{
    if (a->limbs == NULL)
        _ualloc(a, 8, 1);

    if (a->used_limbs < 1)
        invalid_parameter("arz_movd");

    int32_t i = 0;
    int32_t s = 0;

    // see if continuation (_) or minus sign (-)
    if (digits[0] == '_')
    {
        s = (a->flags & NegativeReg);
        i++;
    }
    else
    {
        arz_movk(a, 0);
        if (digits[0] == '-')
        {
            s = NegativeReg;
            i++;
        }
    }

    while (true) // Stop at first non-decimal
    {
        int32_t k = digits[i] - '0';

        if (k < 0 || k > 9)
            break;

        if (a->alloc_limbs <= a->used_limbs)
            _uextend(a, a->used_limbs + 1);

        _xmulk(a->limbs, 10, a->used_limbs);
        a->used_limbs++;
        _xaddk(a->limbs, k, a->used_limbs);

        _utrim(a);
        i++;
    }

    a->flags = s;
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

// arz_cmp(a, b) = arz_sgn(a - b) = -1 if a < b, 0 if a = b, 1 if a > b
int32_t arz_cmp(arz_t* a, arz_t* b)
{
    int32_t f;

    if (a->limbs == NULL)
        invalid_parameter("arz_cmp");

    if (b->limbs == NULL)
        invalid_parameter("arz_cmp");

    if (a->used_limbs < 1)
        invalid_parameter("arz_cmp");

    if (b->used_limbs < 1)
        invalid_parameter("arz_cmp");

    if (!(a->flags & NegativeReg))
    {
        if (!(b->flags & NegativeReg))
        {
            // a >= 0, b >= 0
            f = _xcmp(a->limbs, b->limbs, a->used_limbs, b->used_limbs);
            if (f & ZeroFlag)
                return 0;

            if (f & CarryFlag)
                return -1;
            else
                return 1;
        }
        else
        {
            // a >= 0, b <= 0
            if (arz_sgn(a))
                return 1;

            if (arz_sgn(b))
                return 1;

            return 0;
        }
    }
    else
    {
        if (b->flags & NegativeReg)
        {
            // a <= 0, b <= 0
            f = _xcmp(b->limbs, a->limbs, b->used_limbs, a->used_limbs);
            if (f & ZeroFlag)
                return 0;

            if (f & CarryFlag)
                return -1;
            else
                return 1;
        }
        else
        {
            // a <= 0, b >= 0
            if (arz_sgn(a))
                return -1;

            if (arz_sgn(b))
                return -1;

            return 0;
        }
    }
}

// arz_cmpk(a, k) = arz_sgn(a - k) = -1 if a < k, 0 if a = k, 1 if a > k, -2^31 < k < 2^31
int32_t arz_cmpk(arz_t* a, int32_t k)
{
    int32_t b, s;

    if (a->limbs == NULL)
        invalid_parameter("arz_cmpk");

    if (a->used_limbs < 1)
        invalid_parameter("arz_cmpk");

    if (k > 0)
    {
        if (!(a->flags & NegativeReg))
        {
            b = k; // a >= 0, k > 0
            s = _xcmp(a->limbs, &b, a->used_limbs, 1);

            if (s & ZeroFlag)
                return 0;

            if (s & CarryFlag)
                return -1;
            else
                return 1;
        }
        else
            return -1; // a <= 0, k > 0
    }
    else if (k < 0)
    {
        if (a->flags & NegativeReg)
        {
            b = -k; // a <= 0, k < 0
            s = _xcmp(&b, a->limbs, 1, a->used_limbs);

            if (s & ZeroFlag)
                return 0;

            if (s & CarryFlag)
                return -1;
            else
                return 1;
        }
        else
            return 1; // a >= 0, k < 0
    }
    else
        return arz_sgn(a); // k = 0
}

// a = a^2
void arz_squ(arz_t* a)
{
    if (a->limbs == NULL)
        invalid_parameter("arz_squ");

    if (a->used_limbs < 1)
        invalid_parameter("arz_squ");

    _utrim(a);

    int32_t da2 = (a->used_limbs + a->used_limbs);

    if (a->used_limbs < FFTsqu) // School method
    {
        if (a->alloc_limbs < da2)
            _uextend(a, da2);

        _xsqu(a->limbs, a->used_limbs);
    }
    else // Traditional FFT method
    {
        // Allocate FFT vector
        // Set L = least power of two greater than 4 * d(a)
        // FFT elements are 16 bits
        // BUG! This routine does not work well with numbers >= 2^1000000
        int32_t da4 = da2 + da2;
        int32_t L = 1;

        while (L <= da4)
            L += L;

        if (a->alloc_limbs <= (L >> 1))
            _uextend(a, 1 + (L >> 1));

        _xfft_init(L, 1);
        _xfft_pack(FFTv1, (uint16_t*)a->limbs, L, da2);
        _xfft_real_to_hermitian(FFTv1, L);
        _xfft_square_hermitian(FFTv1, L);
        _xfft_inv_hermitian_to_real(FFTv1, L);
        _xfft_unpack(FFTv1, (uint16_t*)a->limbs, L);
    }

    a->used_limbs = da2;
    a->flags = 0;
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

    if (a->used_limbs < FFTmul || b->used_limbs < FFTmul) // School method
    {
        if (a->alloc_limbs < dab)
            _uextend(a, dab);

        _xmul(a->limbs, b->limbs, a->used_limbs, b->used_limbs);
    }
    else // Traditional FFT method
    {
        // Allocate FFT vectors
        // Set L = least power of two greater than 4 * max(d(a), d(b))
        // FFT elements are 16 bits
        // BUG! This routine does not work well with numbers >= 2^1000000
        int32_t dmax2 = a->used_limbs;
        int32_t dmax4 = 0;

        if (dmax2 < b->used_limbs)
            dmax2 = b->used_limbs;

        dmax2 += dmax2;
        dmax4 = dmax2 + dmax2;

        int32_t L = 1;
        while (L <= dmax4)
            L += L;

        if (a->alloc_limbs <= (L >> 1))
            _uextend(a, 1 + (L >> 1));

        _xfft_init(L, 3);
        _xfft_pack(FFTv1, (uint16_t*)a->limbs, L, da2);
        _xfft_pack(FFTv2, (uint16_t*)b->limbs, L, db2);
        _xfft_real_to_hermitian(FFTv1, L);
        _xfft_real_to_hermitian(FFTv2, L);
        _xfft_mul_hermitian(FFTv1, FFTv2, L);
        _xfft_inv_hermitian_to_real(FFTv1, L);
        _xfft_unpack(FFTv1, (uint16_t*)a->limbs, L);
    }

    a->used_limbs = dab;
    a->flags = a->flags ^ (b->flags & NegativeReg);
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

// a = [a * 2^k],  (k = 64-bit positive integer)
void arz_mul2k(arz_t* a, int32_t k)
{
    if (a->limbs == NULL)
        invalid_parameter("arz_mul2k");

    if (a->used_limbs < 1)
        invalid_parameter("arz_mul2k");

    int32_t da = (k + 31) >> 5;
    if (a->alloc_limbs < a->used_limbs + da)
        _uextend(a, a->used_limbs + da);

    _xmul2k(a->limbs, k, a->used_limbs);
    a->used_limbs += da;
}

// a =  [ a / b], res =  ( a mod k), a >= 0,
// a = -[ a / b], res = -(-a mod k), a <  0;  b > 0
// (b is trimmed by this routine)
void arz_div(arz_t* a, arz_t* b)
{
    if (a->limbs == NULL)
        invalid_parameter("idiv");

    if (b->limbs == NULL)
        invalid_parameter("idiv");

    if (a->used_limbs < 1)
        invalid_parameter("idiv");

    if (b->used_limbs < 1)
        invalid_parameter("idiv");

    if (arz_sgn(b) <= 0)
        invalid_parameter("idiv");

    assert(b->limbs != NULL);

    _utrim(a);
    _utrim(b);

    if (b->used_limbs > 1)
    {
        arz_mov(res, a);
        if (res->used_limbs >= b->used_limbs)
        {
            if (res->alloc_limbs <= res->used_limbs)
                _uextend(res, res->used_limbs + 1);

            _xdiv(res->limbs, b->limbs, res->used_limbs, b->used_limbs, a->limbs);
            a->used_limbs = res->used_limbs - b->used_limbs + 1;
            res->used_limbs = b->used_limbs;
        }
        else // d(b) > d(a)
            arz_movk(a, 0);
    }
    else // b->digits = 1
    {
        arz_movk(res, 0);
        res->limbs[0] = _xdivk(a->limbs, b->limbs[0], a->used_limbs);
        res->flags = a->flags;
    }
}

// a =  [ a / k], kres =  ( a mod k), a >= 0,
// a = -[-a / k], kres = -(-a mod k), a <  0;  k = 32-bit positive integer
void arz_divk(arz_t* a, int32_t k)
{
    if (a->limbs == NULL)
        invalid_parameter("arz_divk");

    if (a->used_limbs < 1)
        invalid_parameter("arz_divk");

    if (k == 0)
        invalid_parameter("arz_divk");

    _utrim(a);

    kres = _xdivk(a->limbs, k, a->used_limbs);
    if (a->flags & NegativeReg)
        kres = -kres;
}

// a = [a / 2^k],  (k = 32-bit positive integer)
void arz_div2k(arz_t* a, int32_t k)
{
    static int32_t d;

    if (a->limbs == NULL)
        invalid_parameter("arz_div2k");

    if (a->used_limbs < 1)
        invalid_parameter("arz_div2k");

    d = k >> 5;
    if (d < a->used_limbs)
    {
        _xdiv2k(a->limbs, k, a->used_limbs);
        a->used_limbs -= d;
    }
    else
        arz_movk(a, 0);
}

// a = (a mod b) >= 0, b > 0
// (b is trimmed by this routine)
void arz_mod(arz_t* a, arz_t* b)
{
    if (a->limbs == NULL)
        invalid_parameter("arz_mod");

    if (b->limbs == NULL)
        invalid_parameter("arz_mod");

    if (a->used_limbs < 1)
        invalid_parameter("arz_mod");

    if (b->used_limbs < 1)
        invalid_parameter("arz_mod");

    if (arz_sgn(b) <= 0)
        invalid_parameter("arz_mod");

    assert(b->limbs != NULL);

    _utrim(a);
    _utrim(b);

    if (b->used_limbs > 1)
    {
        if (a->used_limbs >= b->used_limbs)
        {
            if (a->alloc_limbs <= a->used_limbs)
                _uextend(a, a->used_limbs + 1);

            _xmod(a->limbs, b->limbs, a->used_limbs, b->used_limbs);
            a->used_limbs = b->used_limbs;
        }

        // Remainder must be non-negative
        if (arz_sgn(a) < 0)
            arz_add(a, b);
    }
    else // b->digits = 1
        arz_modk(a, b->limbs[0]);
}

// a = (a mod k) >= 0,  k = 32-bit positive integer
void arz_modk(arz_t* a, int32_t k)
{
    if (a->limbs == NULL)
        invalid_parameter("arz_modk");

    if (a->used_limbs < 1)
        invalid_parameter("arz_modk");

    if (k == 0)
        invalid_parameter("arz_modk");

    assert(a->limbs != NULL);

    _utrim(a);

    _xmodk(a->limbs, k, a->used_limbs);
    a->used_limbs = 1;

    // Result must be non-negative
    if ((a->flags & NegativeReg) && a->limbs[0])
    {
        a->limbs[0] = k - a->limbs[0];
        a->flags ^= NegativeReg;
    }
}

// a = (a mod 2^k) > 0,  k = 32-bit positive integer
// 
// d(a)=6, k=72           kr=8 km=255
//                        |
// |a a a a|a a a a|a a a a|a a a a|a a a a|
//                         |<-----kq------>|
//                 |<---------da---------->|
void arz_mod2k(arz_t* a, int32_t k)
{
    if (a->limbs == NULL)
        invalid_parameter("arz_mod2k");

    if (a->used_limbs < 1)
        invalid_parameter("arz_mod2k");

    assert(a->limbs != NULL);

    if (k)
    {
        int32_t kq = k >> 6;
        int32_t kr = k & 63;
        int32_t km = 1ULL << kr;

        km--;

        int32_t da = (k + 31) >> 5;

        if (a->flags & NegativeReg)
        {
            a->flags ^= NegativeReg;
            if (!_usig(a))
                return;

            // a < 0: compute 2^k - |a|
            if (a->alloc_limbs < da)
                _uextend(a, da);

            if (a->used_limbs < da)
                _xmovz(&a->limbs[a->used_limbs], da - a->used_limbs);

            _xneg(a->limbs, da);
            if (kr)
                a->limbs[kq] &= km;
        }
        else
        {
            if (kq >= a->used_limbs)
                return;

            if (kr)
                a->limbs[kq] &= km;
        }

        a->used_limbs = da;
    }
    else
        arz_movk(a, 0);
}

// a = b^c
void arz_exp(arz_t* a, arz_t* b, arz_t* c)
{
    if (b->limbs == NULL)
        invalid_parameter("arz_exp");

    if (c->limbs == NULL)
        invalid_parameter("arz_exp");

    if (b->used_limbs < 1)
        invalid_parameter("arz_exp");

    if (c->used_limbs < 1)
        invalid_parameter("arz_exp");

    assert(c->limbs != NULL);

    arz_movk(a, 1);
    if (arz_sgn(c) <= 0)
        return;

    int32_t f = 0;
    for (int32_t i = c->used_limbs - 1; i >= 0; i--)
    {
        for (uint32_t j = 0x80000000; j >= 1; j >>= 1)
        {
            if (f)
                arz_squ(a);

            if (c->limbs[i] & j)
            {
                arz_mul(a, b);
                f++;
            }
        }
    }
}

// Return max{log(|a|), 0}.
double arz_log(arz_t* a)
{
    if (a->limbs == NULL)
        invalid_parameter("arz_log");

    if (a->used_limbs < 1)
        invalid_parameter("arz_log");

    assert(a->limbs != NULL);

    _utrim(a);

    double u = (double)a->limbs[a->used_limbs - 1];
    double v;

    if (a->used_limbs > 1)
        v = (double)a->limbs[a->used_limbs - 2];
    else
        v = 0;

    if (u < 0)
        u += 4294967296.0;

    if (v < 0)
        v += 4294967296.0;

    double w = u + v / 4294967296.0;

    if (w <= 0)
        return 0.0;

    return log(w) + 32.0 * (a->used_limbs - 1) * dlog2;
}

// Set a = [b^(1/2)], res = b - [b^(1/2)]^2.
void arz_sqrt(arz_t* a, arz_t* b)
{
    if (b->limbs == NULL)
        invalid_parameter("arz_sqrt");

    if (b->used_limbs < 1)
        invalid_parameter("arz_sqrt");

    if (arz_sgn(b) < 0)
        invalid_parameter("arz_sqrt");

    arz_movk(temp1, 1);
    if (arz_cmp(b, temp1) <= 0)
    {
        arz_mov(a, b);
        arz_movk(res, 0);
        return;
    }

    // Compute a = b^(1/2) by Newton's method
    // Compute first approximation to a
    double u = arz_log(b) / 2;
    uint32_t j = (uint32_t)floor(u / dlog2);

    if (j >= 30)
        j -= 30;
    else
        j = 0;

    uint32_t w = (uint32_t)floor(exp(u - dlog2 * j));

    // Our first guess for a is |w|.
    arz_movk(a, w);
    arz_abs(a);
    arz_mul2k(a, j);

    // Iterate a -> (b + a^2) / (2*a)
    for (;;)
    {
        arz_mov(temp2, a);  // save old a
        arz_squ(a);
        arz_add(a, b);      // a' = b + a^2
        arz_div(a, temp2);
        arz_div2k(a, 1);    // a' = (b + a^2)/(2*a)
        arz_sub(temp2, a);
        arz_abs(temp2);

        if (arz_cmp(temp2, temp1) <= 0)
            break;
    }

    // a = [b^(1/2)] +/- 1
    arz_addk(a, 1);
    arz_mov(temp2, a);
    arz_squ(temp2);
    arz_mov(res, b);
    arz_sub(res, temp2);    // res = b - a^2

    while (arz_sgn(res) == 1)
    {
        arz_sub(res, a);
        arz_addk(a, 1);
        arz_sub(res, a);
    }

    while (arz_sgn(res) == -1)
    {
        arz_add(res, a);
        arz_subk(a, 1);
        arz_add(res, a);
    }
}

// Return [log(|a|)/log(2)] + 1, the length of a in bits.
int32_t arz_len(arz_t* a)
{
    if (!arz_sgn(a))
        return 0;

    int32_t k = a->used_limbs << 5;
    for (int32_t i = a->used_limbs - 1; i >= 0; i--)
    {
        uint32_t v = a->limbs[i];
        for (uint32_t w = 0x80000000; w != 0; w >>= 1)
        {
            if (v & w)
                return k;

            k--;
        }
    }

    return 0;
}

// Return [|a|/2^k] mod 2, the value of bit k of a.
// Bit 0 is the units bit.
int32_t arz_bit(arz_t* a, int32_t k)
{
    int32_t i = k >> 5;
    if (i >= a->used_limbs)
        return 0;

    if (a->limbs[i] & (1 << (k & 31)))
        return 1;

    return 0;
}
