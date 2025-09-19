// ARIEL - Arbitrary Resolution Integer Execution Library - 32 bit version
// Version 0.1a - first iteration - very ALPHA code!
//
// Note that the code is not currently optimized.
// Copyright 2025, Bryan K Reinhart

#include "ariel32_base.h"

#include <assert.h>
#include <math.h>


//FFT related

#define TWOPI (2 * 3.14159265358979323846)
#define SQRTHALF 0.70710678118654752440
#define TWO16 65536.0
#define TWOM16 0.0000152587890625

// Increment bit-reversed index x
// MUST define 'rb' and 'half' before use!
#define inc_bit_rev(x) rb=half; while(x&rb) {x-=rb; rb>>=1;} x+=rb

double* FFTsincos = NULL;
double* FFTv1 = NULL;
double* FFTv2 = NULL;
int32_t FFTdim = 0;
int32_t FFTdimq1 = 0;
int32_t FFTdimq2 = 0;
int32_t FFTdimq3 = 0;
int32_t FFTdimv1 = 0;
int32_t FFTdimv2 = 0;
int32_t FFTsqu = 730;
int32_t FFTmul = 1200;
int32_t checkFFTerror = 1;
double maxFFTerror = 0;


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

// Split a into two parts, a and b, at 2^k
// |a| =  (|a| mod 2^k), |b| = [|a| / 2^k]
void _usep2k(arz_t* a, arz_t* b, int32_t k)
{
    int32_t db = a->used_limbs - (k >> 6); // quotient length

    if (db > 0)
    {
        if (b->limbs == NULL)
            _ualloc(b, db, db);

        if (b->used_limbs < db)
            _uextend(b, db);

        _xsep2k(a->limbs, b->limbs, k, a->used_limbs);
        b->used_limbs = db;
    }
    else
    {
        if (b->limbs == NULL)
            _ualloc(b, 8, 1);

        assert(b->limbs != NULL);

        b->limbs[0] = 0;
        b->used_limbs = 1;
    }

    b->flags = a->flags;
}

// Unformatted print of an ARIEL integer register
void _udump(arz_t* a)
{
    xprintf("c=%d d=%d f=%d &v=%p v=", a->alloc_limbs, a->used_limbs, a->flags, a->limbs);

    for (int32_t i = 0; i < a->used_limbs; i++)
        xprintf("%d ", a->limbs[i]);

    xprintf("\n");
}

int xfprintf(FILE* fp, const char* s, ...)
{
    int retval;
    va_list arglist;

    va_start(arglist, s);
#ifdef _MSC_VER
    retval = vfprintf_s(fp, s, arglist);
#else
    retval = vfprintf(fp, s, arglist);
#endif
    va_end(arglist);

    return retval;
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

double _xsin(int32_t n)
{
    if (n < FFTdimq1)
        return FFTsincos[n];

    if (n < FFTdimq2)
        return FFTsincos[FFTdimq2 - n];

    if (n < FFTdimq3)
        return -FFTsincos[n - FFTdimq2];

    return -FFTsincos[FFTdim - n];
}

double _xcos(int32_t n)
{
    if (n < FFTdimq1)
        return FFTsincos[FFTdimq1 - n];

    if (n < FFTdimq2)
        return -FFTsincos[n - FFTdimq1];

    if (n < FFTdimq3)
        return -FFTsincos[FFTdimq3 - n];

    return FFTsincos[n - FFTdimq3];
}

// Allocate FFT vectors and initialise sin/cos table
// b = 1, 2, 3: allocate FFTv1, FFTv2, both
void _xfft_init(int32_t n, int32_t b)
{
    double* v;
    double e;

    if (b & 1)
    {
        if (n > FFTdimv1)
        {
            v = realloc(FFTv1, sizeof(double) * n);
            if (v != NULL)
                FFTv1 = v;
            else
            {
                printf("Unable to allocate FFT %d-vector in xfft_init\n", n);
                terminate();
            }

            FFTdimv1 = n;
        }
    }

    if (b & 2)
    {
        if (n > FFTdimv2)
        {
            v = realloc(FFTv2, sizeof(double) * n);
            if (v != NULL)
                FFTv2 = v;
            else
            {
                printf("Unable to allocate FFT %d-vector in xfft_init\n", n);
                terminate();
            }

            FFTdimv2 = n;
        }
    }

    if (n > FFTdim)
    {
        int32_t vec_size = 1 + (n >> 2);
        e = TWOPI / n;
        v = realloc(FFTsincos, sizeof(double) * vec_size);

        if (v != NULL)
            FFTsincos = v;
        else
        {
            printf("Unable to allocate %d-vector for sin/cos table in xfft_init\n", vec_size);
            terminate();
        }

        for (int32_t j = 0; j <= (n >> 2); j++)
            FFTsincos[j] = sin(e * j);

        FFTdim = n;
        FFTdimq2 = FFTdim >> 1;
        FFTdimq1 = FFTdimq2 >> 1;
        FFTdimq3 = FFTdimq1 + FFTdimq2;
    }
}

// Free memory occupied by FFT vectors and sin/cos table
void _xfft_free(void)
{
    if (FFTdimv1)
    {
        free(FFTv1);
        FFTdimv1 = 0;
        FFTv1 = NULL;
    }

    if (FFTdimv2)
    {
        free(FFTv2);
        FFTdimv2 = 0;
        FFTv1 = NULL;
    }

    if (FFTdim)
    {
        free(FFTsincos);
        FFTdim = 0;
        FFTsincos = NULL;
    }
}

void _xfft_pack(double* z, uint16_t* x, int32_t n, int32_t w)
{
    int32_t j, r;
    int32_t rb, half = n >> 1; // variables 'rb' and 'half' are required for 'inc_bit_rev'

    for (j = 0, r = 0; j < w; j++)
    {
        z[r] = x[j];
        inc_bit_rev(r);
    }

    while (r)
    {
        z[r] = 0.0;
        inc_bit_rev(r);
    }
}

void _xfft_unpack(double* z, uint16_t* x, int32_t n)
{
    double d = 1 / (double)n;
    double g = 0.0;

    int32_t j, r;
    int32_t rb, half = n >> 1; // variables 'rb' and 'half' are required for 'inc_bit_rev'

    maxFFTerror = 0;

    for (j = 0, r = 0; j < n; j++)
    {
        double h = z[r] * d;
        double f = floor(h + 0.5);

        if (checkFFTerror)
        {
            double e = fabs(f - h);
            if (e > maxFFTerror)
                maxFFTerror = e;
        }

        f += g;
        g = floor(f * TWOM16);
        f -= (g * TWO16);

        x[j] = (uint16_t)f;
        inc_bit_rev(r);
    }

    if (g < TWO16)
        x[n] = (uint16_t)g;
    else
        maxFFTerror += 100;

    x[n + 1] = 0;

    if (checkFFTerror)
    {
        if (maxFFTerror > 0.25)
        {
            if (maxFFTerror > 0.5)
                printf("FFT error: maxFFTerror=%g \n", maxFFTerror);
            else
                printf("FFT warning: maxFFTerror=%g \n", maxFFTerror);
        }
    }
}

// Input  is {z[0], z[n/2], z[n/4], ..., z[n-1]} in bit-reversed order
// Output is {Re(z^[0]), ..., Re(z^[n/2]), Im(z^[n/2-1]), ..., Im(z^[1]).
// This is a decimation-in-time, split-radix algorithm.
void _xfft_real_to_hermitian(double* z, int32_t n)
{
    double cc1, ss1, cc3, ss3, e;
    double t0, t1, t2, t3, t4, t5;

    int32_t i, j;
    int32_t i0, i1, i2, i3, i4, i5, i6, i7;
    int32_t nn = n >> 1;
    int32_t is = 1;
    int32_t id = 4;

    do
    {
        for (i1 = is; i1 <= n; i1 += id)
        {
            i0 = i1 - 1;
            e = z[i0];

            z[i0] = e + z[i1];
            z[i1] = e - z[i1];
        }

        is = (id << 1) - 1;
        id <<= 2;
    } while (is < n);

    int32_t n2 = 2;

    while (nn >>= 1)
    {
        n2 <<= 1;
        int32_t n4 = n2 >> 2;
        int32_t n8 = n2 >> 3;

        is = 0;
        id = n2 << 1;

        do
        {
            for (i = is; i < n; i += id)
            {
                i0 = i;
                i1 = i0 + n4;
                i2 = i1 + n4;
                i3 = i2 + n4;

                t0 = z[i3] + z[i2];

                z[i3] -= z[i2];
                z[i2] = z[i0] - t0;
                z[i0] += t0;

                if (n4 == 1)
                    continue;

                i0 += n8;
                i1 += n8;
                i2 += n8;
                i3 += n8;

                t0 = (z[i2] + z[i3]) * SQRTHALF;
                t1 = (z[i2] - z[i3]) * SQRTHALF;

                z[i3] = z[i1] - t0;
                z[i2] = -z[i1] - t0;
                z[i1] = z[i0] - t1;
                z[i0] += t1;
            }

            is = (id << 1) - n2;
            id <<= 2;
        } while (is < n);

        int32_t dil = FFTdim / n2;
        int32_t a = dil;

        for (j = 1; j < n8; j++)
        {
            int32_t scmask = FFTdim - 1;
            int32_t a3 = (a + (a << 1)) & scmask;

            // cc1 = xcos(a);
            if (a < FFTdimq1)
                cc1 = FFTsincos[FFTdimq1 - a];
            else if (a < FFTdimq2)
                cc1 = -FFTsincos[a - FFTdimq1];
            else if (a < FFTdimq3)
                cc1 = -FFTsincos[FFTdimq3 - a];
            else
                cc1 = FFTsincos[a - FFTdimq3];

            // cc3 = xcos(a3);
            if (a3 < FFTdimq1)
                cc3 = FFTsincos[FFTdimq1 - a3];
            else if (a3 < FFTdimq2)
                cc3 = -FFTsincos[a3 - FFTdimq1];
            else if (a3 < FFTdimq3)
                cc3 = -FFTsincos[FFTdimq3 - a3];
            else
                cc3 = FFTsincos[a3 - FFTdimq3];

            // ss1 = xsin(a);
            if (a < FFTdimq1)
                ss1 = FFTsincos[a];
            else if (a < FFTdimq2)
                ss1 = FFTsincos[FFTdimq2 - a];
            else if (a < FFTdimq3)
                ss1 = -FFTsincos[a - FFTdimq2];
            else
                ss1 = -FFTsincos[FFTdim - a];

            // ss3 = xsin(a3);
            if (a3 < FFTdimq1)
                ss3 = FFTsincos[a3];
            else if (a3 < FFTdimq2)
                ss3 = FFTsincos[FFTdimq2 - a3];
            else if (a3 < FFTdimq3)
                ss3 = -FFTsincos[a3 - FFTdimq2];
            else
                ss3 = -FFTsincos[FFTdim - a3];

            a = (a + dil) & scmask;

            is = 0;
            id = n2 << 1;

            do
            {
                for (i = is; i < n; i += id)
                {
                    i0 = i + j;
                    i1 = i0 + n4;
                    i2 = i1 + n4;
                    i3 = i2 + n4;

                    i4 = i + n4 - j;
                    i5 = i4 + n4;
                    i6 = i5 + n4;
                    i7 = i6 + n4;

                    t0 = z[i2] * cc1 + z[i6] * ss1;
                    t1 = z[i6] * cc1 - z[i2] * ss1;
                    t2 = z[i3] * cc3 + z[i7] * ss3;
                    t3 = z[i7] * cc3 - z[i3] * ss3;

                    t4 = t0 + t2;
                    t5 = t1 + t3;
                    t2 = t0 - t2;
                    t3 = t1 - t3;

                    t1 = z[i5] + t5;
                    z[i2] = t5 - z[i5];
                    z[i7] = t1;

                    t1 = z[i1] - t2;
                    z[i6] = -z[i1] - t2;
                    z[i3] = t1;

                    t0 = z[i0] + t4;
                    z[i5] = z[i0] - t4;
                    z[i0] = t0;

                    t0 = z[i4] + t3;
                    z[i4] -= t3;
                    z[i1] = t0;
                }

                is = (id << 1) - n2;
                id <<= 2;
            } while (is < n);
        }
    }
}

// Input  is   {Re(z^[0]), ..., Re(z^[n/2]), Im(z^[n/2-1]), ..., Im(z^[1]).
// Output is n*{z[0], z[n/2], z[n/4], ..., z[n-1]} in bit-reversed order
// This is a decimation-in-frequency, split-radix algorithm.
void _xfft_inv_hermitian_to_real(double* z, int32_t n)
{
    double cc1, ss1, cc3, ss3;
    double t0, t1, t2, t3, t4;

    int32_t i, j;
    int32_t is, id, i0, i1, i2, i3, i4, i5, i6, i7;
    int32_t nn = n >> 1;
    int32_t n2 = n << 1;

    while (nn >>= 1)
    {
        is = 0;
        id = n2;

        n2 >>= 1;
        int32_t n4 = n2 >> 2;
        int32_t n8 = n4 >> 1;

        do
        {
            for (i = is; i < n; i += id)
            {
                i0 = i;
                i1 = i0 + n4;
                i2 = i1 + n4;
                i3 = i2 + n4;

                t0 = z[i0] - z[i2];

                z[i0] += z[i2];
                z[i1] += z[i1];
                z[i2] = t0 - 2.0 * z[i3];
                z[i3] = t0 + 2.0 * z[i3];

                if (n4 == 1)
                    continue;

                i0 += n8;
                i1 += n8;
                i2 += n8;
                i3 += n8;

                t0 = (z[i1] - z[i0]) * SQRTHALF;
                t1 = (z[i3] + z[i2]) * SQRTHALF;

                z[i0] += z[i1];
                z[i1] = z[i3] - z[i2];
                z[i2] = -2.0 * (t1 + t0);
                z[i3] = 2.0 * (t0 - t1);
            }

            is = (id << 1) - n2;
            id <<= 2;
        } while (is < n - 1);

        int32_t dil = FFTdim / n2;
        int32_t a = dil;

        for (j = 1; j < n8; j++)
        {
            int32_t scmask = FFTdim - 1;
            int32_t a3 = (a + (a << 1)) & scmask;

            // cc1 = xcos(a);
            if (a < FFTdimq1)
                cc1 = FFTsincos[FFTdimq1 - a];
            else if (a < FFTdimq2)
                cc1 = -FFTsincos[a - FFTdimq1];
            else if (a < FFTdimq3)
                cc1 = -FFTsincos[FFTdimq3 - a];
            else
                cc1 = FFTsincos[a - FFTdimq3];

            // cc3 = xcos(a3);
            if (a3 < FFTdimq1)
                cc3 = FFTsincos[FFTdimq1 - a3];
            else if (a3 < FFTdimq2)
                cc3 = -FFTsincos[a3 - FFTdimq1];
            else if (a3 < FFTdimq3)
                cc3 = -FFTsincos[FFTdimq3 - a3];
            else
                cc3 = FFTsincos[a3 - FFTdimq3];

            // ss1 = xsin(a);
            if (a < FFTdimq1)
                ss1 = FFTsincos[a];
            else if (a < FFTdimq2)
                ss1 = FFTsincos[FFTdimq2 - a];
            else if (a < FFTdimq3)
                ss1 = -FFTsincos[a - FFTdimq2];
            else
                ss1 = -FFTsincos[FFTdim - a];

            // ss3 = xsin(a3);
            if (a3 < FFTdimq1)
                ss3 = FFTsincos[a3];
            else if (a3 < FFTdimq2)
                ss3 = FFTsincos[FFTdimq2 - a3];
            else if (a3 < FFTdimq3)
                ss3 = -FFTsincos[a3 - FFTdimq2];
            else
                ss3 = -FFTsincos[FFTdim - a3];

            a = (a + dil) & scmask;

            is = 0;
            id = n2 << 1;

            do
            {
                for (i = is; i < n; i += id)
                {
                    i0 = i + j;
                    i1 = i0 + n4;
                    i2 = i1 + n4;
                    i3 = i2 + n4;

                    i4 = i + n4 - j;
                    i5 = i4 + n4;
                    i6 = i5 + n4;
                    i7 = i6 + n4;

                    t0 = z[i0] - z[i5];
                    z[i0] += z[i5];

                    t1 = z[i4] - z[i1];
                    z[i4] += z[i1];

                    t2 = z[i7] + z[i2];
                    z[i5] = z[i7] - z[i2];

                    t3 = z[i3] + z[i6];
                    z[i1] = z[i3] - z[i6];

                    t4 = t0 - t3;
                    t0 += t3;
                    t3 = t1 - t2;
                    t1 += t2;

                    z[i2] = t4 * cc1 + t3 * ss1;
                    z[i6] = -t3 * cc1 + t4 * ss1;
                    z[i3] = t0 * cc3 - t1 * ss3;
                    z[i7] = t1 * cc3 + t0 * ss3;
                }

                is = (id << 1) - n2;
                id <<= 2;
            } while (is < n - 1);
        }
    }

    is = 1;
    id = 4;

    do
    {
        for (i1 = is; i1 <= n; i1 += id)
        {
            i0 = i1 - 1;
            double e = z[i0];

            z[i0] = e + z[i1];
            z[i1] = e - z[i1];
        }

        is = (id << 1) - 1;
        id <<= 2;
    } while (is < n);
}

void _xfft_square_hermitian(double* b, int32_t n)
{
    int32_t k, half = n >> 1;
    double c, d;

    b[0] *= b[0];
    b[half] *= b[half];

    for (k = 1; k < half; k++)
    {
        c = b[k];
        d = b[n - k];

        b[n - k] = 2.0 * c * d;
        b[k] = (c + d) * (c - d);
    }
}

void _xfft_mul_hermitian(double* a, double* b, int32_t n)
{
    int32_t k, half = n >> 1;
    double aa, bb, am, bm;

    a[0] *= b[0];
    a[half] *= b[half];

    for (k = 1; k < half; k++)
    {
        aa = a[k];
        bb = b[k];

        am = a[n - k];
        bm = b[n - k];

        a[k] = aa * bb - am * bm;
        a[n - k] = aa * bm + am * bb;
    }
}

// unformatted print of an array
void _xdump(int32_t* a, int32_t d)
{
    printf("%d ", d);

    for (int32_t i = 0; i < d; i++)
        printf("%d ", (uint32_t)a[i]);

    printf("\n");
}

// unformatted print of an array
void _xdumps(int16_t* a, int32_t d)
{
    printf("%d ", d);

    for (int32_t i = 0; i < d; i++)
        printf("%d ", (uint16_t)a[i]);

    printf("\n");
}

// unformatted print of an array
void _xdumpd(double* a, int32_t d)
{
    printf("%d ", d);

    for (int32_t i = 0; i < d; i++)
        printf("%e ", a[i]);

    printf("\n");
}
