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


%endif
