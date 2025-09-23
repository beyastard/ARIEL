#ifndef __ARIEL_EVALUATE_X86_H__
#define __ARIEL_EVALUATE_X86_H__

#include "ariel32.h"


// extern variables
extern int32_t evaluate_pointer;
extern int32_t evaluate_level;
extern int32_t evaluate_expression;
extern char* evaluate_error;

// function prototypes
void evaluate_break(void);
void evaluate_factorial(void);
void evaluate_primorial(void);
void evaluate_compute(int32_t key);
void evaluate_push(int32_t key);
arz_t* arz_evaluate(char* expr);

#endif
