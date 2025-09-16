; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

%include "globals.inc"


section .text align=8


; void _xmovk(limb_t* a, int32_t k, int32_t da);  // a = k (k = 32-bit integer)
; Assumptions: d(a) > 0
;
global __xmovk
align 8
__xmovk:
    push    ebp
    mov     ebp, esp

    mov     dword [Zsg], 0
    mov     eax, [ebp+8]            ; EAX = a->limbs
    mov     edx, [ebp+12]           ; EDX = k
    mov     ecx, [ebp+16]           ; ECX = da

    mov     [eax], edx              ; move k into a->limbs
    dec     ecx
    jz      .LmovkX
    sar     edx, 31
    and     edx, [Zsg]              ; EDX = sign bits

.LmovkD:
    mov     [eax+4], edx            ; extend with sign
    add     eax, 4
    dec     ecx
    jg      .LmovkD

.LmovkX:
    leave
    ret
