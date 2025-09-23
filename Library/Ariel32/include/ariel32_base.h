#ifndef __ARIEL_BASE_X86_H__
#define __ARIEL_BASE_X86_H__

// ARIEL - Arbitrary Resolution Integer Execution Library - 32 bit version
// Version 0.1a - first iteration - very ALPHA code!
//
// Note that the code is not currently optimized.
// Copyright 2025, Bryan K Reinhart

#include <conio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


// limb type: 32-bit word used in multi-precision arithmetic
typedef int32_t limb_t;

// ariel integer register type
typedef struct ariel_integer_s
{
    limb_t* limbs;          // Pointer to least-significant limb (LSL at [0])
    int32_t alloc_limbs;    // Number of limbs allocated (memory size/capacity)
    int32_t used_limbs;     // Number of limbs currently used (active length/digits)
    uint32_t flags;         // Bitmask: sign/zero/special states
} arz_t;

// flags
#define NegativeReg 0x80000000

// Hardware flags as stored by LAHF
#define SignFlag    0x00008000
#define ZeroFlag    0x00004000
#define AuxFlag     0x00001000
#define ParityFlag  0x00000200
#define CarryFlag   0x00000100

// Hardware register sign bit
#define SignBit     0x80000000


// memory allocation/control
void _ualloc(arz_t* a, int32_t c, int32_t d);
void _ufree(arz_t* a);
void _uextend(arz_t* a, int32_t d);
void _usep2k(arz_t* a, arz_t* b, int32_t k);
void _udump(arz_t* a);

// formatted print
int xfprintf(FILE* fp, const char* s, ...);
int xprintf(const char* s, ...);


// terminate program
inline void terminate(void)
{
    printf("Sorry, cannot continue... Press 'X' to exit.\n");

    while (true)
    {
        if (_getch() == tolower('X'))
            exit(EXIT_SUCCESS);
    }
}


// FFT variables
extern double* FFTsincos;
extern double* FFTv1;
extern double* FFTv2;
extern int32_t FFTdim;
extern int32_t FFTdimq1;
extern int32_t FFTdimq2;
extern int32_t FFTdimq3;
extern int32_t FFTdimv1;
extern int32_t FFTdimv2;
extern int32_t FFTsqu;
extern int32_t FFTmul;
extern int32_t checkFFTerror;
extern double maxFFTerror;

// FFT and FFT support functions
double _xsin(int32_t n);
double _xcos(int32_t n);

void _xfft_init(int32_t n, int32_t b);
void _xfft_free(void);
void _xfft_pack(double* z, uint16_t* x, int32_t n, int32_t w);
void _xfft_unpack(double* z, uint16_t* x, int32_t n);

void _xfft_real_to_hermitian(double* z, int32_t n);
void _xfft_inv_hermitian_to_real(double* z, int32_t n);
void _xfft_square_hermitian(double* b, int32_t n);
void _xfft_mul_hermitian(double* a, double* b, int32_t n);


void _xdump(int32_t* a, int32_t d);
void _xdumps(int16_t* a, int32_t d);
void _xdumpd(double* a, int32_t d);


// assembly-level function prototypes

extern void _xmov(limb_t* a, limb_t* b, int32_t da, int32_t db);
extern void _xmovf(limb_t* a, limb_t* b, int32_t db);
extern void _xmovk(limb_t* a, int32_t k, int32_t da);
extern void _xmovz(limb_t* a, int32_t da);

extern int32_t _xadd(limb_t* a, limb_t* b, int32_t da, int32_t db);
extern void _xaddf(limb_t* a, limb_t* b, int32_t db);
extern int32_t _xaddk(limb_t* a, int32_t k, int32_t da);

extern int32_t _xsub(limb_t* a, limb_t* b, int32_t da, int32_t db);
extern void _xsubf(limb_t* a, limb_t* b, int32_t da);
extern int32_t _xsubk(limb_t* a, int32_t k, int32_t da);

extern void _xneg(limb_t* a, int32_t da);
extern int32_t _xcmp(limb_t* a, limb_t* b, int32_t da, int32_t db);

extern int32_t _xdigits(int32_t da, limb_t* a);
extern int32_t _xsig(limb_t* a, int32_t da);

extern void _xsqu(limb_t* a, int32_t da);
extern void _xmul(limb_t* a, limb_t* b, int32_t da, int32_t db);
extern void _xmulk(limb_t* a, int32_t k, int32_t da);
extern void _xmul2k(limb_t* a, int32_t k, int32_t da);
extern void _xmul2d(limb_t* a, int32_t d, int32_t da);

extern void _xdiv(limb_t* a, limb_t* b, int32_t da, int32_t db, limb_t* q);
extern int32_t _xdivk(limb_t* a, int32_t k, int32_t da);
extern void _xdiv2k(limb_t* a, int32_t k, int32_t da);
extern void _xdiv2d(limb_t* a, int32_t d, int32_t da);

extern void _xmod(limb_t* a, limb_t* b, int32_t da, int32_t db);
extern void _xmodk(limb_t* a, int32_t k, int32_t da);
extern void _xsep2k(limb_t* a, limb_t* q, int32_t k, int32_t da);

extern int32_t _xnext_prime(int32_t k);


// The macro-level '_u*' functions are completely unchecked and have no error-checking as they
// are intended for internal use. Thet are similar to the main 'arz_*' functions minus the error
// checking and memory allocation that the main functions handle automatically. If you use any
// of these functions, you will have to do your own error checking and input validation.
// You have been warned...

// a = b  [d(a) >= d(b) > 0)
#define _umov(areg,breg) _xmov(areg->limbs, breg->limbs, areg->used_limbs, breg->used_limbs)

// a = b  [d(a) = d(b) > 0]
#define _umovf(areg,breg) _xmovf(areg->limbs, breg->limbs, breg->used_limbs); areg->used_limbs = breg->used_limbs

// a = k  [d(a) > 0]
#define _umovk(areg,knum) _xmovk(areg->limbs, knum, areg->used_limbs)

// a = 0  [d(a) > 0]
#define _umovz(areg) _xmovz(areg->limbs, areg->used_limbs)

// a = a + b (mod 2^(32*d(a))),  returns CarryFlag and SignBit  [d(a) >= d(b) > 0]
#define _uadd(areg,breg) _xadd(areg->limbs, breg->limbs, areg->used_limbs, breg->used_limbs)

// a = a + b (mod 2^(32*d(a))) [d(a) = d(b) > 0]
#define _uaddf(areg,breg) _xaddf(areg->limbs, breg->limbs, breg->used_limbs)

// a = a + k (mod 2^(32*d(a))),  returns CarryFlag and SignBit  [k >= 0, d(a) >= d(b) > 0]
#define _uaddk(areg,knum) _xaddk(areg->limbs, knum, areg->used_limbs)

// a = a - b (mod 2^(64*d(a))),  returns CarryFlag and SignBit  [d(a) >= d(b) > 0]
#define _usub(areg,breg) _xsub(areg->limbs, breg->limbs, areg->used_limbs, breg->used_limbs)

// a = a - b (mod 2^(32*d(a)))  [d(a) = d(b) > 0]
#define _usubf(areg,breg) _xsubf(areg->limbs, breg->limbs, breg->used_limbs)

// a = a - k (mod 2^(32*d(a))),  returns CarryFlag and SignBit  [k >= 0, d(a) >= d(b) > 0]
#define _usubk(areg,knum) _xsubk(areg->limbs, knum, areg->used_limbs)

// a = 2^(64*d(a)) - a  [d(a) > 0]
#define _uneg(areg) _xneg(areg->limbs, areg->used_limbs)

// flags(a-b)  [d(a), d(b) > 0]
#define _ucmp(areg,breg) _xcmp(areg->limbs, breg->limbs, areg->used_limbs, breg->used_limbs)

// a = a^2  [d(a) > 0, c(a) >= 2*d(a)]
#define _usqu(areg) _xsqu(areg->limbs, areg->used_limbs); areg->used_limbs = 2*areg->used_limbs

// a = a * b  [d(a), d(b) > 0, c(a) >= d(a) + d(b)]
#define _umul(areg,breg) _xmul(areg->limbs, breg->limbs, areg->used_limbs, breg->used_limbs); areg->used_limbs = areg->used_limbs + breg->used_limbs

// a = a * k  [d(a) > 0, c(a) > d(a)]
#define _umulk(areg,knum) _xmulk(areg->limbs, knum, areg->used_limbs); areg->used_limbs++

// a = a * 2^k  [d(a) > 0, c(a) >= d(a) + [(k + 63)/64]]
#define _umul2k(areg,knum) _xmul2k(areg->limbs, knum, areg->used_limbs); areg->used_limbs += ((k + 63) >> 6)

// a = a * 2^(64*d)  [d(a) > 0, c(a) >= d(a) + d]
#define _umul2d(areg,dnum) _xmul2d(areg->limbs, dnum, areg->used_limbs); areg->used_limbs += dnum

// a = (a mod b), q = [a / b]  [c(a) > d(a) >= d(b) > 0, c(q) >= d(a) - d(b) + 1]
#define _udiv(areg,breg,qreg) _xdiv(areg->limbs, breg->limbs, areg->used_limbs, breg->used_limbs, qreg->limbs); qreg->used_limbs = areg->used_limbs - breg->used_limbs + 1  

// a = [a / k], returns (a mod k)  [k > 0, d(a) > 0]
#define _udivk(areg,knum) _xdivk(areg->limbs, knum, areg->used_limbs)

// a = [a / 2^k]  [0 <= k < 64*d(a), d(a) > 0]
#define _udiv2k(areg,knum) _xdiv2k(areg->limbs, knum, areg->used_limbs); areg->used_limbs -= (knum >> 6)

// a = [a / 2^(64*d)]  [0 <= d < d(a), d(a) > 0]
#define _udiv2d(areg,dnum) _xdiv2d(areg->limbs, dnum, areg->used_limbs); areg->used_limbs -= dnum

// a = (a mod b)  [b > 0, c(a) > d(a) >= d(b) > 0]
#define _umod(areg,breg) _xmod(areg->limbs, breg->limbs, areg->used_limbs, breg->used_limbs)

// a = (a mod k)  [k > 0, d(a) > 0]
#define _umodk(areg,knum) _xmodk(areg->limbs, knum, areg->used_limbs); areg->used_limbs = 1

// Remove non-significant digits from an integer register
#define _utrim(areg)  areg->used_limbs = _xdigits(areg->used_limbs, areg->limbs)

// high order digit of a  [d(a) > 0]
#define _usig(areg) _xsig(areg->limbs, areg->used_limbs)


#endif
