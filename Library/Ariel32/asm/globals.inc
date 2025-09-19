%ifndef GLOBALS_INC
%define GLOBALS_INC

; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

section .data   align=16


Zsg         dd      0               ; sign indicator
Zcarry      dd      0               ; carry indicator

Zmula0      dd      0               ; save &a->limbs
Zmulb0      dd      0               ; save &b->limbs
Zmulda      dd      0               ; save d(a)
Zmuldb      dd      0               ; save d(b)

Zdiva0      dd      0               ; Save &a
Zdivb0      dd      0               ; Save &b

Zdivda      dd      0               ; Save d(a)
Zdivdb      dd      0               ; Save d(b)

Zdivq0      dd      0               ; Save &q
Zdivai      dd      0               ; &a_i

Zdivbz      dd      0               ; b_z = number of high order zero bits of b
Zdivv1      dd      0               ; v_1 = high order 32 bits of b
Zdivv2      dd      0               ; v_2 = next 32 bits of b

Zdivii      dd      0               ; i
Zdivqi      dd      0               ; q_i


%endif
