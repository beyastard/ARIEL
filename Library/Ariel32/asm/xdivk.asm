; ARIEL - Arbitrary Resolution Integer Execution Library x32
; Version 0.1a - first iteration - very ALPHA code!
;
; Note that the code is not currently optimized.
; Copyright 2025, Bryan K Reinhart

bits 32
default rel

section .text align=8


; int32_t _xdivk(limb_t* a, int32_t k, int32_t da); // a = [a / k]  (k = 32-bit integer)
; Assumptions: k > 0, d(a) > 0
; Returns:     (a mod k) in EAX
;
global __xdivk
align 8
__xdivk:
    push    ebp
    mov     ebp, esp

    push    ebx
    push    edi
    
    mov     eax, [ebp+8]
    mov     edx, [ebp+12]
    mov     ebx, [ebp+16]

    lea     edi, [eax+ebx*4-4]      ; EDI = &a_{d-1}
    mov     ecx, edx                ; EDX = k
    xor     edx, edx                ; EDX = 0

.Ldivk2:
    mov     eax, [edi]              ; EAX = a_i
    div     ecx                     ; EAX = [(2^32*EDX + a_i)/k]
    mov     [edi], eax              ; EDX = (2^32*EDX + a_i) mod k]
    sub     edi, 4
    dec     ebx
    jg      .Ldivk2
    mov     eax, edx                ; remainder

    pop     edi
    pop     ebx
    leave
    ret
