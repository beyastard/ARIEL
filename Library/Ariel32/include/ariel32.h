#ifndef __ARIEL_LIBRARY_X86_H__
#define __ARIEL_LIBRARY_X86_H__

// ARIEL - Arbitrary Resolution Integer Execution Library - 32 bit version
// Version 0.1a - first iteration - very ALPHA code!
//
// Note that the code is not currently optimized.
// Copyright 2025, Bryan K Reinhart

#include "ariel32_base.h"


// remainder register
extern arz_t* res;
extern int32_t kres;

// temporary registers
extern arz_t* temp1;
extern arz_t* temp2;
extern arz_t* temp3;
extern arz_t* temp4;
extern arz_t* temp5;

// useful constants
extern double dlog2;
extern double dlog10;

void invalid_parameter(const char* c);
void arz_fdisplay(FILE* fp, arz_t* a);
#define arz_display(areg) arz_fdisplay(NULL, areg)

void arz_mov(arz_t* a, arz_t* b);
void arz_movd(arz_t* a, const char* digits);
void arz_movk(arz_t* a, int32_t k);

void arz_add(arz_t* a, arz_t* b);
void arz_addk(arz_t* a, int32_t k);

void arz_sub(arz_t* a, arz_t* b);

int32_t arz_sgn(arz_t* a);
int32_t arz_cmp(arz_t* a, arz_t* b);
int32_t arz_cmpk(arz_t* a, int32_t k);

void arz_squ(arz_t* a);
void arz_mul(arz_t* a, arz_t* b);
void arz_mulk(arz_t* a, int32_t k);
void arz_mul2k(arz_t* a, int32_t k);

void arz_div(arz_t* a, arz_t* b);
void arz_divk(arz_t* a, int32_t k);
void arz_div2k(arz_t* a, int32_t k);

void arz_mod(arz_t* a, arz_t* b);
void arz_modk(arz_t* a, int32_t k);
void arz_mod2k(arz_t* a, int32_t k);

void arz_exp(arz_t* a, arz_t* b, arz_t* c);
void arz_expmod(arz_t* a, arz_t* b, arz_t* c, arz_t* q);
void arz_expmodm2ke(arz_t* a, arz_t* b, arz_t* c, arz_t* q, int32_t m, int32_t k, arz_t* e);

void arz_gcd(arz_t* a, arz_t* b);
void arz_gcdext(arz_t* u, arz_t* d, arz_t* a, arz_t* b);

double arz_log(arz_t* a);
void arz_sqrt(arz_t* a, arz_t* b);

int32_t arz_len(arz_t* a);
int32_t arz_bit(arz_t* a, int32_t k);


// maco-level functions
#define arz_subk(areg,klng) arz_addk(areg, -(klng))
#define arz_neg(areg) (areg->flags ^= NegativeReg)
#define arz_abs(areg) (areg->flags &= (0xFFFFFFFF-NegativeReg))
#define arz_odd(areg) (areg->limbs[0] & 1)
#define arz_even(areg) !(areg->limbs[0] & 1)
#define arz_wd0(areg) (areg->limbs[0])

#endif
