#ifndef __ARIEL_BASE_X86_H__
#define __ARIEL_BASE_X86_H__

// ARIEL - Arbitrary Resolution Integer Execution Library - 32 bit version
// Version 0.1a - first iteration - very ALPHA code!
//
// Note that the code is not currently optimized.
// Copyright 2025, Bryan K Reinhart

#include <stdint.h>


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



#endif
