#ifndef __ARIEL_LIBRARY_X86_H__
#define __ARIEL_LIBRARY_X86_H__

// ARIEL - Arbitrary Resolution Integer Execution Library - 32 bit version
// Version 0.1a - first iteration - very ALPHA code!
//
// Note that the code is not currently optimized.
// Copyright 2025, Bryan K Reinhart

#include "ariel32_base.h"


extern arz_t* res;


void invalid_parameter(const char* c);
void arz_fdisplay(FILE* fp, arz_t* a);
#define arz_display(areg) arz_fdisplay(NULL, areg)

void arz_mov(arz_t* a, arz_t* b);
void arz_movk(arz_t* a, int32_t k);

void arz_add(arz_t* a, arz_t* b);
void arz_addk(arz_t* a, int32_t k);

void arz_sub(arz_t* a, arz_t* b);

int32_t arz_sgn(arz_t* a);

void arz_mul(arz_t* a, arz_t* b);
void arz_mulk(arz_t* a, int32_t k);


// maco-level functions
#define arz_subk(areg,klng) arz_addk(areg, -(klng))
#define arz_neg(areg) (areg->flags ^= NegativeReg)
#define arz_abs(areg) (areg->flags &= (0xFFFFFFFF-NegativeReg))
#define arz_odd(areg) (areg->limbs[0] & 1)
#define arz_even(areg) !(areg->limbs[0] & 1)
#define arz_wd0(areg) (areg->limbs[0])

#endif
