; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; int32_t _xsig(limb_t* a, int32_t da); // high-order digit of a
; Assumptions: d(a) > 0
; Returns:     EAX = high order digit of a
;              (a = 0 iff EAX = 0)
;
global __xsig
align 8
__xsig:
    push    ebp
    mov     ebp, esp

    push    ebx
    push    esi
    push    edi
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]

    lea     ebx, [eax+edx*4-4]      ; EBX = &a_{d-1}

.Lsig2:
    mov     eax, [ebx]
    lea     ebx, [ebx-4]
    cmp     eax, 0
    jne     .Lsig8
    dec     edx
    jg      .Lsig2

.Lsig8:
    pop     edi
    pop     esi
    pop     ebx
    leave
    ret
