#ifndef __ARIEL_TEST_X86_H__
#define __ARIEL_TEST_X86_H__

#include "ariel32.h"


// CheckLog   0 = log failures only, 1 = log everything
// CheckStop  1 = stop if fail, 2 = stop anyway
// CheckDiag  1 = activate tdiag routine
#define CheckLog  1
#define CheckStop 1
#define CheckDiag 1


// function prototypes
void halt(void);
int32_t test_quit(void);
void test_check_register(arz_t* a, arz_t* b, char* id, int32_t ik);
void test_check_digits(arz_t* a, char* s, char* id, int32_t ik);


#endif
