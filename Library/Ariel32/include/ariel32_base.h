#ifndef __ARIEL_BASE_X86_H__
#define __ARIEL_BASE_X86_H__

// ARIEL - Arbitrary Resolution Integer Execution Library - 32 bit version
// Version 0.1a - first iteration - very ALPHA code!
//
// Note that the code is not currently optimized.
// Copyright 2025, Bryan K Reinhart

#include <conio.h>
#include <ctype.h>
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


// memory allocation/control
void _ualloc(arz_t* a, int32_t c, int32_t d);
void _ufree(arz_t* a);
void _uextend(arz_t* a, int32_t d);


// terminate program
inline void terminate(void)
{
#ifdef _MSC_VER
    printf_s("Sorry, cannot continue... Press 'X' to exit.\n");
#else
    printf("Sorry, cannot continue... Press 'X' to exit.\n");
#endif
    while (true)
    {
        if (_getch() == tolower('X'))
            exit(EXIT_SUCCESS);
    }
}


// assembly-level function prototypes

extern void _xmovf(limb_t* a, limb_t* b, int32_t db);
extern void _xmovz(limb_t* a, int32_t da);


// The macro-level '_u*' functions are completely unchecked and have no error-checking as they
// are intended for internal use. Thet are similar to the main 'arz_*' functions minus the error
// checking and memory allocation that the main functions handle automatically. If you use any
// of these functions, you will have to do your own error checking and input validation.
// You have been warned...

// a = b  [d(a) = d(b) > 0]
#define _umovf(areg,breg) _xmovf(areg->limbs, breg->limbs, breg->used_limbs); areg->used_limbs = breg->used_limbs

// a = 0  [d(a) > 0]
#define _umovz(areg) _xmovz(areg->limbs, areg->used_limbs)


#endif
