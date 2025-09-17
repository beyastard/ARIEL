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

    mov     eax, [ebp+8]            ; EAX = a->limbs
    mov     edx, [ebp+12]           ; EDX = da

    lea     ebx, [eax+edx*4-4]      ; ECX = &a_{d-1}

.loop:
    mov     eax, [ebx]
    lea     ebx, [ebx-4]
    cmp     eax, 0
    jne     .done
    dec     edx
    jg      .loop

.done:
    pop     ebx
    leave
    ret
